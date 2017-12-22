/*
 * Copyright (c) 2012, Taobao.com
 * All rights reserved.
 *
 * �ļ����ƣ�logger.cpp
 * ժҪ�������log4cpp����������־���
 * ���ߣ�Benkong <benkong@taobao.com>
 * ���ڣ�2012.7.2
 */
#include "logger.h"
#include "INIParser.h"
#include <stdarg.h>

#include <string.h>
#include <stdlib.h>
#if ((defined  __LINUX__) || ( defined  __MAC__))
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#endif
#ifdef __WINDOWS__
#include<windows.h>
#endif
#define DEFAULT_TIMEFMT "%Y-%m-%d %H:%M:%S" /* ȱʡʱ���ʽ */
#define STD_PREFIX   "std"

#define _1K   1024
#define _1M   (_1K*_1K)
#define SECS_IN_1MIN  60
#define MINS_IN_1HOUR 60
#define DEFAULT_SWITCH_SIZE   _1K // ȱʡ�ֻ���С1K M == 1G 
#define MAX_SWITCH_SIZE   (2*_1K) // �����ֻ���С 2G 

#define DEFAULT_ROLLING_NUM 7
#define MAX_ROLLING_NUM 255
#define DEFAULT_FILEAGE 60
#define MAX_FILEAGE  1440

#define MAX_PATH_LEN 512
#define MAX_LOG_LEN  512
#define MAX_FMT_TIME_LEN 48

// ��־���ѡ��壬���ֵ����ͨ��"��"һ�𴫵�
enum LLO_T
{
	LLO_LEVEL_MASK      = 0x0000000F,  // ��־��������

	LLO_SWITCH_BY_TIME  = 0x00000010,  // ��־��ʱ������ֻ����
	LLO_SWITCH_BY_SIZE  = 0x00000020,  // ��־�Դ�С�����ֻ������ͬʱ��ָ���ֻ���С

	LLO_OUTPUT_LEVEL    = 0x00000100,  // �����־�ļ���
	LLO_OUTPUT_PID      = 0x00000200,  // ���pid��־ 
	LLO_OUTPUT_FILELINE = 0x00000400,  // ����ļ������к�
	LLO_OUTPUT_PRGNAME  = 0x00000800,  // �����������ģ����

	// ����LLO_OUTPUT_LEVEL���������
	LLO_OUTPUT_MASK     = LLO_OUTPUT_PID|LLO_OUTPUT_FILELINE|LLO_OUTPUT_PRGNAME, 
	
	LLO_ROLLING_NUM_MASK= 0x000FF000,  // �����������룬���255��
	LLO_ROLLING_SHIFT   = 12,          // ������������λ��
};

// ��־�����ַ���
static const char* LOG_LEVEL[] = {
	NULL,
	"DEBUG",
	"INFO",
	"NOTICE",
	"WARN",
	"ERROR",
	"CRIT",
	"FATAL",
	NULL
};

struct LogData
{
	bool   m_usingStd; // �Ƿ��������׼�豸? 
	FILE*  m_fLog;     // ��־�ļ�
	time_t m_logTime;  // �����־��ʱ��
	pthread_mutex_t m_mutexLog;
	
	// �ʹ�С�ֻ���صĲ���
	unsigned m_currNo;  // ��ǰ��־���
	size_t m_size;      // ��ǰ��־��С 
	
	// ��ʱ���ֻ����
	time_t m_rotateTime; // �ֻ�ʱ��
	char m_time[MAX_FMT_TIME_LEN];     // ʱ���ַ���

	LogData(): m_usingStd(false), m_fLog(NULL), m_currNo(0), m_size(0)
	{
		time(&m_logTime);
		pthread_mutex_init(&m_mutexLog, NULL);
	}

	~LogData()
	{
		pthread_mutex_destroy(&m_mutexLog);
		if (!m_usingStd)
		{
			if (m_fLog != NULL)
			{
				fclose(m_fLog);
			}
		}
	}

	void getCurrTime(const char *timeFmt)
	{
		time(&m_logTime);
		struct tm t;
		localtime_r(&m_logTime, &t);
		strftime(m_time, MAX_FMT_TIME_LEN, timeFmt, &t);
	}

	void lock()
	{
		pthread_mutex_lock(&m_mutexLog);
	}

	void unlock()
	{
		pthread_mutex_unlock(&m_mutexLog);
	}
};

/* ��־����ṹ���� */
struct LogHandle
{
	int  m_options;
	std::string m_path;
	std::string m_prefix;
	std::string m_timeFmt;
	size_t   m_switchSize; // �ֻ���С �� �ֻ�ʱ��
	size_t   m_switchMins; // �ֻ�ʱ����������
	int      m_baseLevel;
	unsigned m_rollingNum; // ��־������Ŀ��0��ʾ������

	std::string m_prgName;
	std::string m_modName;
	
	LogData m_logData;

	LogHandle():m_options(0) {}
};

/* ������־�ļ� */
static int createLogFile(char *file, LogHandle *log)
{
	LogData *logData = &(log->m_logData);
	if (logData->m_fLog != NULL)
		return 0;

	if (logData->m_usingStd)
	{
		// ʹ�ñ�׼����豸
		logData->m_fLog = stdout;
		return 0;
	}

	const char *path = log->m_path.c_str();
	const char *prefix = log->m_prefix.c_str();
	if (log->m_options & LLO_SWITCH_BY_SIZE)
	{
		// �ļ��Դ�С�ֻ�
		struct stat sb;
		int len; // file�����е��ַ�����
	
		len = sprintf(file, "%s/%s_", path, prefix);
		do
		{
			sprintf(file+len, "%d", logData->m_currNo);
			
			if (stat(file, &sb) == 0)
			{
				if ((size_t)sb.st_size >= log->m_switchSize)
				{
					logData->m_currNo++;
				}
				else
				{
					logData->m_size = sb.st_size;
					break;
				}
			}
			else
			{
				logData->m_size = 0;
				break;
			}
		} while (true);
		
		if (logData->m_currNo >= log->m_rollingNum)
		{
			// ����־��Ŀ����
			char oldFile[MAX_PATH_LEN];
			memcpy(oldFile, file, len);

			// ɾ��0����־
			sprintf(file+len, "%d", 0);
			remove(file);

			// #1 ~ #(log->m_rollingNum-1)����־����
			for (unsigned i=1; i<log->m_rollingNum; i++)
			{
				sprintf(oldFile+len, "%u", i);
				rename(oldFile, file);
				sprintf(file+len, "%u", i);
			}
			
			logData->m_currNo = log->m_rollingNum-1;
			sprintf(file+len, "%u", logData->m_currNo);
			if (stat(file, &sb) == 0)
				logData->m_size = sb.st_size;
			else
				logData->m_size = 0;
		}
	}
	else if (log->m_options & LLO_SWITCH_BY_TIME)
	{
		// �ļ���ʱ���ֻ�
		struct tm t;
		localtime_r(&(logData->m_logTime), &t);
		int mins = (t.tm_hour * MINS_IN_1HOUR + t.tm_min) / log->m_switchMins * log->m_switchMins; // �ֻ��ļ���׼ʱ��
		int hour = mins / MINS_IN_1HOUR;
		int min  = mins % MINS_IN_1HOUR;
		sprintf(file, "%s/%s_%04d%02d%02d-%02d:%02d:00", path, prefix, t.tm_year+1900, t.tm_mon+1, t.tm_mday, hour, min);

		mins += log->m_switchMins;
		hour = mins / MINS_IN_1HOUR;
		min  = mins % MINS_IN_1HOUR;
		t.tm_hour = hour;
		t.tm_min  = min;
		t.tm_sec  = 0;
		logData->m_rotateTime = mktime(&t); // ��һ���ֻ�ʱ��
	}
	else
	{
		// ��־���ֻ�
		sprintf(file, "%s/%s", path, prefix);
	}
	
	logData->m_fLog = fopen(file, "a");
	if (logData->m_fLog == NULL)
	{
		fprintf(stderr,"open log file %s failed\n",file);
		return 1; // �����ļ�ʧ�� 
	}
	
	setvbuf(logData->m_fLog, NULL, _IONBF, 0);
	return 0;
}

Logger* Logger::m_logger = NULL;

Logger::Logger()
{
	m_log = new LogHandle();
}

Logger::~Logger()
{
	delete m_log;
}

int Logger::init(const char *logPath, const char *filePrefix, int options, const char *timeFmt, int switchSize)
{
	char file[MAX_PATH_LEN];
	
	if ((options & (LLO_SWITCH_BY_SIZE|LLO_SWITCH_BY_TIME)) == 0)
	{
		// ��δָ���ֻ���ʽʱʹ�ô�С�ֻ�
		options |= LLO_SWITCH_BY_SIZE;
		switchSize = DEFAULT_SWITCH_SIZE;
	}

	m_log->m_options = options;
	if (options & LLO_SWITCH_BY_SIZE)
	{
		if (switchSize >= MAX_SWITCH_SIZE)
			switchSize = DEFAULT_SWITCH_SIZE;
		m_log->m_rollingNum = (options & LLO_ROLLING_NUM_MASK) >> LLO_ROLLING_SHIFT;
		m_log->m_switchSize = switchSize * _1M;
	}
	else
	{
		m_log->m_switchMins = switchSize;
		// m_log->m_switchSize = switchSize * SECS_IN_1MIN;
	}
	
	// ���Ƴ�ʼ����Ϣ
	m_log->m_path.assign(logPath);
	m_log->m_prefix.assign(filePrefix);
	m_log->m_logData.m_usingStd = !strcasecmp(STD_PREFIX, filePrefix);
	if (timeFmt != NULL)
		m_log->m_timeFmt.assign(timeFmt);
	else
		m_log->m_timeFmt.assign(DEFAULT_TIMEFMT);

	if (0 != createLogFile(file, m_log))
	{
		return 1; // ������־�ļ�ʧ�� 
	}
	
	// ȡ������־����
	m_log->m_baseLevel = options & LLO_LEVEL_MASK;
	if (m_log->m_baseLevel <= L_MIN_LEVEL || m_log->m_baseLevel >= TOTAL_LEVEL)
	{
		m_log->m_baseLevel = L_INFO;
	}
	
	// �ɹ�����
	return 0; 
}

/**
 * �ѱ�ʾ�ַ�������־����ת��Ϊ��Ӧ������
 * @param szLevel ��־������"INFO"/"DEBUG"��
 * @return ���szLevel�Ϸ���������Ӧ������; ���szLevel���Ϸ�������0
 */
int Logger::strToLevel(const char* level)
{
	if (level == NULL)
		return 0;
	for (int i=1; LOG_LEVEL[i]; i++) {
		if (strcasecmp(LOG_LEVEL[i], level) == 0)
			return i;
	}
	return 0;
}

/**
 * ͨ�������ļ���ʼ����־
 * @param profile   ��־�����ļ���
 * @param section   �����ļ���������
 * @return ��NULL���ɹ�; NULL: ��ʼ��ʧ��
 *
 * �����ļ����������¸�ʽ
 * [log]
 * root_path  = .     # ��־��·��
 * prefix     = std   # ��־�ļ���ǰ׺�����Ϊ"std"�����������׼�豸
 * show_pid   = YES   # �Ƿ����pid����ȡֵYES/NO
 * show_line  = YES   # �Ƿ�����ļ���/�кţ���ȡ֮YES/NO
 * base_level = TRACE # ���������־���𣬿�ȡֵ�ӵ͵�����DEBUG/INFO/NOTICE/WARN/ERROR/CRIT/FATAL
 * show_level = YES   # �Ƿ����������Ϣ����ȡֵ��YES/NO
 * switch_by  = SIZE  # �ֻ���ʽ����ȡֵSIZE/TIME
 * switch_size= 100   # �ֻ��ļ���С����switch_byΪsizeʱ��Ч����λΪM����СΪ10
 * max_fileage= 1440  # ʱ���ֻ����ڣ���switch_byΪtimeʱ��Ч����λΪ�����ӡ�, ���1440
 * rolling_num= 7     # ��־���Թ������������255����ȱʡΪ7
 */
Logger* Logger::createLoggerFromProfile(const char *profile, const char *section)
{
	const char *rootPath;
	const char *prefix;
	int showPid;
	int baseLevel;
	int showLevel;
	int showLine;
	int switchBy = 0;
	int switchSize = 0;
	int rollingNum;
	
	int option;

	if (profile == NULL)
		return NULL;
	utility::INIParser parser;
	parser.load(profile);

	const char *tmpStr;

#define LOG_SECTION section
	rootPath = parser.get_string(LOG_SECTION, "root_path");
	if (rootPath == NULL)
	{
		fprintf(stderr,"can not find %s.%s in log config\n",section,"root_path");
		return NULL;
	}
	prefix = parser.get_string(LOG_SECTION, "prefix");
	if (prefix == NULL)
	{
		fprintf(stderr,"can not find %s.%s in log config\n",section,"prefix");
		return NULL;
	}
	tmpStr = parser.get_string(LOG_SECTION, "base_level");
	if (tmpStr == NULL) {
		baseLevel = L_INFO;
	} else {
		baseLevel = strToLevel(tmpStr);
	}
	
	tmpStr = parser.get_string(LOG_SECTION, "show_pid");
	if (tmpStr == NULL) {
		showPid = 0;
	} else {
		showPid = !strcasecmp("YES", tmpStr);
	}
	
	tmpStr = parser.get_string(LOG_SECTION, "show_level");
	if (tmpStr == NULL) {
		showLevel = 0;
	} else {
		showLevel = !strcasecmp("YES", tmpStr);
	}
	
	tmpStr = parser.get_string(LOG_SECTION, "show_line");
	if (tmpStr == NULL) {
		showLine = 0;
	} else {
		showLine = !strcasecmp("YES", tmpStr);
	}
	

	tmpStr = parser.get_string(LOG_SECTION, "switch_by");
	if (tmpStr != NULL) {
		switchBy = 0;
		if (!strcasecmp("SIZE", tmpStr)) {
			switchBy = LLO_SWITCH_BY_SIZE;
			switchSize = parser.get_int(LOG_SECTION, "switch_size");
		}
		else if (!strcasecmp("TIME",  tmpStr))
		{
			switchBy = LLO_SWITCH_BY_TIME;
			switchSize = parser.get_int(LOG_SECTION, "max_fileage");
			if (switchSize == 0)
				switchSize = DEFAULT_FILEAGE;
			else if (switchSize > MAX_FILEAGE)
				switchSize = MAX_FILEAGE;
		}
	}
	
	rollingNum = parser.get_int(LOG_SECTION, "rolling_num", DEFAULT_ROLLING_NUM);
	if (rollingNum == 0)
		rollingNum = DEFAULT_ROLLING_NUM;
	else if (rollingNum > MAX_ROLLING_NUM)
		rollingNum = MAX_ROLLING_NUM;
	
	/* init log */
	option = 0;
	if (showPid)
		option |= LLO_OUTPUT_PID;
	if (showLevel) {
		option |= LLO_OUTPUT_LEVEL;
		option |= baseLevel;
	}
	if (showLine)
		option |= LLO_OUTPUT_FILELINE;

	option |= switchBy;
	if (switchBy == LLO_SWITCH_BY_SIZE)
	{
		option |= ((rollingNum<<LLO_ROLLING_SHIFT) & LLO_ROLLING_NUM_MASK);
	}

	Logger *log = new Logger();
	if (log->init(rootPath, prefix, option, NULL, switchSize) != 0)
	{
		delete log;
		return NULL;
	}
	return log;
}

// ����logImpl�Ĳ���
struct LogParam
{
	const char* m_fileName;
	int         m_line;
	LogHandle*  m_log;
	LogData*    m_logData;
	const char* m_msg;
	int         m_msgLen;
	int         m_level;

	LogParam(const char *fileName, int line, LogHandle *log, LogData *logData, const char *msg, int msgLen, int level) :
		m_fileName(fileName), m_line(line), m_log(log), m_logData(logData), m_msg(msg), m_msgLen(msgLen), m_level(level)
	{}
};

// �������׼�豸
static void stdLog(LogParam *logParam)
{
	FILE* out = logParam->m_logData->m_fLog;
	LogHandle *log = logParam->m_log;
	LogData *logData = logParam->m_logData;
	
	// ���ʱ��
	logData->getCurrTime(log->m_timeFmt.c_str());
	fprintf(out, "%s ", logData->m_time);
	
	// �����־����
	if (log->m_options & LLO_OUTPUT_LEVEL)
	{
		fprintf(out, "[%s]: ", LOG_LEVEL[logParam->m_level]);
	}

	if (log->m_options & LLO_OUTPUT_MASK)
	{
		fprintf(out, "(");
		if ((log->m_options & LLO_OUTPUT_PRGNAME))
		{
			// �����������ģ�������Ͳ�������ļ������к���
			fprintf(out, "%s,%s ", log->m_prgName.c_str(), log->m_modName.c_str());
		}
		else if ((log->m_options & LLO_OUTPUT_FILELINE) && (logParam->m_fileName != NULL))
		{
			// ����ļ������к�
			fprintf(out, "%s:%d ", logParam->m_fileName, logParam->m_line);
		}
		if (log->m_options & LLO_OUTPUT_PID)
		{
			// ��Ҫ���pid 
#ifdef __LINUX__
			fprintf(out, "%u,%u", getpid(), (unsigned)pthread_self());
#endif
#ifdef __MAC__
			fprintf(out, "%u,%lu", getpid(), (unsigned long)pthread_self());
#endif
#ifdef __WINDOWS__
			fprintf(out, "%lu,%lu", GetCurrentProcessId(), GetCurrentThreadId());
#endif
		}
		fprintf(out, ") ");
	}

	// ����û��Զ�����Ϣ 
	fprintf(out, "%s\n", logParam->m_msg);
}

// �����־
static int logImpl(LogParam* logParam)
{
	char file[MAX_PATH_LEN];
	char logStr[MAX_LOG_LEN]; // ���浱ǰ��־
	char *tmpLog = NULL;      // ��ǰ��־����MAX_LOG_LENʱ����̬����ռ䱣�浱ǰ��־
	int logLen = 0;
	LogHandle *log = logParam->m_log;
	LogData* logData = logParam->m_logData;
	
	if (logData->m_usingStd)
	{
		stdLog(logParam);
		return 0;
	}
	
	// �ϳ���־
	logLen = 0;

	// ���ʱ��
	logData->getCurrTime(log->m_timeFmt.c_str());
	logLen += sprintf(logStr + logLen, "%s ", logData->m_time);
	
	// �����־����
	if (log->m_options & LLO_OUTPUT_LEVEL)
	{
		logLen += sprintf(logStr + logLen, "[%s]: ", LOG_LEVEL[logParam->m_level]);
	}

	if (log->m_options & LLO_OUTPUT_MASK)
	{
		logLen += sprintf(logStr + logLen, "(");

		if ((log->m_options & LLO_OUTPUT_PRGNAME))
		{
			// �����������ģ��������������ļ������к���
			logLen += sprintf(logStr + logLen, "%s,%s ", log->m_prgName.c_str(), log->m_modName.c_str());
		}
		else if ((log->m_options & LLO_OUTPUT_FILELINE) && (logParam->m_fileName != NULL))
		{
			// ����ļ������к�
			logLen += sprintf(logStr + logLen, "%s:%d ", logParam->m_fileName, logParam->m_line);
		}
		if (log->m_options & LLO_OUTPUT_PID)
		{
			// ��Ҫ���pid
#ifdef __LINUX__
			logLen += sprintf(logStr + logLen, "%u,%u", getpid(), (unsigned)pthread_self());
#endif
#ifdef __MAC__
			logLen += sprintf(logStr + logLen, "%u,%lu", getpid(), (unsigned long)pthread_self());
#endif
#ifdef __WINDOWS__
			logLen += sprintf(logStr + logLen, "%lu,%lu", GetCurrentProcessId(), GetCurrentThreadId());
#endif
		}
		logLen += sprintf(logStr + logLen, ") ");
	}
	
	// ����־�ϳ��������ַ���
	if (logLen + logParam->m_msgLen + 1 < MAX_LOG_LEN)
	{
		memcpy(logStr+logLen, logParam->m_msg, logParam->m_msgLen);
		logLen += logParam->m_msgLen;;
		logStr[logLen++] = '\n';
	}
	else
	{
		// ��ǰ��־����MAX_LOG_LEN 
		tmpLog = (char*)malloc(logLen + logParam->m_msgLen+1);
		if (tmpLog == NULL)
			return 1; // �����ڴ�ʧ��
		memcpy(tmpLog, logStr, logLen);
		memcpy(tmpLog+logLen, logParam->m_msg, logParam->m_msgLen);
		logLen += logParam->m_msgLen;;
		tmpLog[logLen++] = '\n';
	}

	// �ж��Ƿ���Ҫʱ���ֻ�
	if (log->m_options & LLO_SWITCH_BY_TIME)
	{
		if (logData->m_logTime >= logData->m_rotateTime)
		{
			fclose(logData->m_fLog);
			logData->m_fLog = NULL;
			logData->m_size = 0;
			
			// �����µ���־�ļ�
			if (!createLogFile(file, log))
			{
				if (tmpLog != NULL)
					free(tmpLog);
				return 2; // ������־�ļ�ʧ��
			}
		}
	}
	
	// �������־�ļ�
	fwrite(tmpLog?tmpLog:logStr, 1, logLen, logData->m_fLog);
	if (tmpLog != NULL)
		free(tmpLog);
	logData->m_size += logLen;
	
	// �Ƿ���Ҫ��С�ֻ�
	if ((log->m_options & LLO_SWITCH_BY_SIZE) && (logData->m_size >= log->m_switchSize))
	{
		fclose(logData->m_fLog);
		logData->m_fLog = NULL;
		logData->m_size = 0;
		logData->m_currNo++;
		
		if (!createLogFile(file, log))
			return 3; // �����ļ�ʧ��
	}
	
	return 0;
}

/**
 * �����־����"_r"Ϊ�̰߳�ȫ�ĺ���
 * @param fileName  ���øú�����Դ�ļ���
 * @param lineNo    ���øú�����Դ�ļ��к�
 * @param level     ��־����
 * @param format    �����־�ĸ�ʽ�������ο�printf
 * @return 0: �ɹ�; <0: ���ִ�����
 */
int Logger::log(const char *fileName, int lineNo, int level, const char *format, ...)
{
	va_list ap;
	char *msg = NULL;
	int msgLen;
	int ret;
	
	if (format == NULL)
		return 1; // ��������
	
	// �����־����
	if (level <= L_MIN_LEVEL || level >= TOTAL_LEVEL || level < m_log->m_baseLevel)
		return 0;
	
	// �ϳ��Զ�����־
	va_start(ap, format);
	msgLen = vasprintf(&msg, format, ap);
	va_end(ap);
	if (msgLen == -1)
		return 2;

	LogParam logParam(fileName, lineNo, m_log, &(m_log->m_logData), msg, msgLen, level);
	ret = logImpl(&logParam);
	free(msg);
	return ret;
}

int Logger::log_r(const char *fileName, int lineNo, int level, const char *format, ...)
{
	va_list ap;
	char *msg = NULL;
	int msgLen;;
	int ret;
	
	if (format == NULL)
		return 1; // ��������
	
	// �����־����
	if (level <= L_MIN_LEVEL || level >= TOTAL_LEVEL || level < m_log->m_baseLevel)
		return 0;
	
	// �ϳ��Զ�����־
	va_start(ap, format);
	msgLen = vasprintf(&msg, format, ap);
	va_end(ap);
	if (msgLen == -1)
		return 2;

	LogParam logParam(fileName, lineNo, m_log, &(m_log->m_logData), msg, msgLen, level);
	m_log->m_logData.lock();
	ret = logImpl(&logParam);
	m_log->m_logData.unlock();
	
	free(msg);
	return ret;
}

/**
 * �����־(ת�������ӿڵĵ���)����"_r"Ϊ�̰߳�ȫ�ĺ���
 * @param level     ��־����
 * @param format    �����־�ĸ�ʽ�������ο�printf
 * @param ap        �����ӿڵĿɱ�����б�
 * @return 0: �ɹ�; <0: ���ִ�����
 */
int Logger::log(  int level, const char *format, va_list &ap)
{
	char *msg = NULL;
	int msgLen;;
	int ret;
	
	if (format == NULL)
		return 1; // ��������
	
	// �����־����
	if (level <= L_MIN_LEVEL || level >= TOTAL_LEVEL || level < m_log->m_baseLevel)
		return 0;
	
	// �ϳ��Զ�����־
	msgLen = vasprintf(&msg, format, ap);
	if (msgLen == -1)
		return 2;

	LogParam logParam(NULL, -1, m_log, &(m_log->m_logData), msg, msgLen, level);
	ret = logImpl(&logParam);
	
	free(msg);
	return ret;
}

int Logger::log_r(int level, const char *format, va_list &ap)
{
	char *msg = NULL;
	int msgLen;;
	int ret;
	
	if (format == NULL)
		return 1; // ��������
	
	// �����־����
	if (level <= L_MIN_LEVEL || level >= TOTAL_LEVEL || level < m_log->m_baseLevel)
		return 0;
	
	// �ϳ��Զ�����־
	msgLen = vasprintf(&msg, format, ap);
	if (msgLen == -1)
		return 2;

	LogParam logParam(NULL, -1, m_log, &(m_log->m_logData), msg, msgLen, level);
	m_log->m_logData.lock();
	ret = logImpl(&logParam);
	m_log->m_logData.unlock();
	
	free(msg);
	return ret;
}

/**
 * �������û����������
 * @param baseLevel �����������
 * @return 0: ����; >0: �ɵĻ����������
 */
int Logger::setBaseLevel(int baseLevel)
{
	if (baseLevel > L_MIN_LEVEL && baseLevel < TOTAL_LEVEL) {
		int oldLevel = m_log->m_baseLevel;
		m_log->m_baseLevel = baseLevel;
		return oldLevel;
	}
	return 0;
}

/**
 * ����������־��������
 * @param rollingNum ��־��������
 * return <0: ����; >=0: �ɵ���־��������
 */
int Logger::setRollingNum(int rollingNum)
{
	if (rollingNum >= 0 && rollingNum <= MAX_ROLLING_NUM) {
		int oldRollingNum = m_log->m_rollingNum;
		m_log->m_rollingNum = rollingNum;
		return oldRollingNum;
	}
	return -1;
}

/**
 * ģ��log4cpp������
 * @param prgName ������
 * @param modName
 */
void Logger::setPrgAndModName(const char *prgName, const char *modName)
{
	if (prgName == NULL || modName == NULL)
		return;
	m_log->m_prgName.assign(prgName);
	m_log->m_modName.assign(modName);
	m_log->m_options |= LLO_OUTPUT_PRGNAME;
}
