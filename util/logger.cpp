/*
 * Copyright (c) 2012, Taobao.com
 * All rights reserved.
 *
 * 文件名称：logger.cpp
 * 摘要：相对于log4cpp的轻量级日志输出
 * 作者：Benkong <benkong@taobao.com>
 * 日期：2012.7.2
 */
#include "logger.h"
#include "INIParser.h"
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_TIMEFMT "%Y-%m-%d %H:%M:%S" /* 缺省时间格式 */
#define STD_PREFIX   "std"

#define _1K   1024
#define _1M   (_1K*_1K)
#define SECS_IN_1MIN  60
#define MINS_IN_1HOUR 60
#define DEFAULT_SWITCH_SIZE   _1K // 缺省轮换大小1K M == 1G 
#define MAX_SWITCH_SIZE   (2*_1K) // 最大的轮换大小 2G 

#define DEFAULT_ROLLING_NUM 7
#define MAX_ROLLING_NUM 255
#define DEFAULT_FILEAGE 60
#define MAX_FILEAGE  1440

#define MAX_PATH_LEN 512
#define MAX_LOG_LEN  512
#define MAX_FMT_TIME_LEN 48

// 日志输出选项定义，多个值可以通过"或"一起传递
enum LLO_T
{
	LLO_LEVEL_MASK      = 0x0000000F,  // 日志级别掩码

	LLO_SWITCH_BY_TIME  = 0x00000010,  // 日志以时间进行轮换输出
	LLO_SWITCH_BY_SIZE  = 0x00000020,  // 日志以大小进行轮换输出，同时需指明轮换大小

	LLO_OUTPUT_LEVEL    = 0x00000100,  // 输出日志的级别
	LLO_OUTPUT_PID      = 0x00000200,  // 输出pid标志 
	LLO_OUTPUT_FILELINE = 0x00000400,  // 输出文件名和行号
	LLO_OUTPUT_PRGNAME  = 0x00000800,  // 输出程序名和模块名

	// 除了LLO_OUTPUT_LEVEL的输出掩码
	LLO_OUTPUT_MASK     = LLO_OUTPUT_PID|LLO_OUTPUT_FILELINE|LLO_OUTPUT_PRGNAME, 
	
	LLO_ROLLING_NUM_MASK= 0x000FF000,  // 滚动计数掩码，最多255个
	LLO_ROLLING_SHIFT   = 12,          // 滚动计数左移位数
};

// 日志级别字符串
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
	bool   m_usingStd; // 是否输出到标准设备? 
	FILE*  m_fLog;     // 日志文件
	time_t m_logTime;  // 输出日志的时间
	pthread_mutex_t m_mutexLog;
	
	// 和大小轮换相关的参数
	unsigned m_currNo;  // 当前日志序号
	size_t m_size;      // 当前日志大小 
	
	// 和时间轮换相关
	time_t m_rotateTime; // 轮换时间
	char m_time[MAX_FMT_TIME_LEN];     // 时间字符串

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

/* 日志句柄结构定义 */
struct LogHandle
{
	int  m_options;
	std::string m_path;
	std::string m_prefix;
	std::string m_timeFmt;
	size_t   m_switchSize; // 轮换大小 或 轮换时间
	size_t   m_switchMins; // 轮换时间间隔，分钟
	int      m_baseLevel;
	unsigned m_rollingNum; // 日志滚动数目，0表示不滚动

	std::string m_prgName;
	std::string m_modName;
	
	LogData m_logData;

	LogHandle():m_options(0) {}
};

/* 创建日志文件 */
static int createLogFile(char *file, LogHandle *log)
{
	LogData *logData = &(log->m_logData);
	if (logData->m_fLog != NULL)
		return 0;

	if (logData->m_usingStd)
	{
		// 使用标准输出设备
		logData->m_fLog = stdout;
		return 0;
	}

	const char *path = log->m_path.c_str();
	const char *prefix = log->m_prefix.c_str();
	if (log->m_options & LLO_SWITCH_BY_SIZE)
	{
		// 文件以大小轮换
		struct stat sb;
		int len; // file中已有的字符长度
	
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
			// 有日志数目滚动
			char oldFile[MAX_PATH_LEN];
			memcpy(oldFile, file, len);

			// 删除0号日志
			sprintf(file+len, "%d", 0);
			remove(file);

			// #1 ~ #(log->m_rollingNum-1)号日志改名
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
		// 文件以时间轮换
		struct tm t;
		localtime_r(&(logData->m_logTime), &t);
		int mins = (t.tm_hour * MINS_IN_1HOUR + t.tm_min) / log->m_switchMins * log->m_switchMins; // 轮换文件基准时间
		int hour = mins / MINS_IN_1HOUR;
		int min  = mins % MINS_IN_1HOUR;
		sprintf(file, "%s/%s_%04d%02d%02d-%02d:%02d:00", path, prefix, t.tm_year+1900, t.tm_mon+1, t.tm_mday, hour, min);

		mins += log->m_switchMins;
		hour = mins / MINS_IN_1HOUR;
		min  = mins % MINS_IN_1HOUR;
		t.tm_hour = hour;
		t.tm_min  = min;
		t.tm_sec  = 0;
		logData->m_rotateTime = mktime(&t); // 下一个轮换时间
	}
	else
	{
		// 日志不轮换
		sprintf(file, "%s/%s", path, prefix);
	}
	
	logData->m_fLog = fopen(file, "a");
	if (logData->m_fLog == NULL)
	{
		return 1; // 创建文件失败 
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
		// 在未指明轮换方式时使用大小轮换
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
	
	// 复制初始化信息
	m_log->m_path.assign(logPath);
	m_log->m_prefix.assign(filePrefix);
	m_log->m_logData.m_usingStd = !strcasecmp(STD_PREFIX, filePrefix);
	if (timeFmt != NULL)
		m_log->m_timeFmt.assign(timeFmt);
	else
		m_log->m_timeFmt.assign(DEFAULT_TIMEFMT);

	if (0 != createLogFile(file, m_log))
	{
		return 1; // 创建日志文件失败 
	}
	
	// 取基本日志级别
	m_log->m_baseLevel = options & LLO_LEVEL_MASK;
	if (m_log->m_baseLevel <= L_MIN_LEVEL || m_log->m_baseLevel >= TOTAL_LEVEL)
	{
		m_log->m_baseLevel = L_INFO;
	}
	
	// 成功返回
	return 0; 
}

/**
 * 把表示字符串的日志基本转换为对应的数字
 * @param szLevel 日志级别，如"INFO"/"DEBUG"等
 * @return 如果szLevel合法，返回相应的数字; 如果szLevel不合法，返回0
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
 * 通过配置文件初始化日志
 * @param profile   日志配置文件名
 * @param section   配置文件区域名称
 * @return 非NULL：成功; NULL: 初始化失败
 *
 * 配置文件必须有如下格式
 * [log]
 * root_path  = .     # 日志根路径
 * prefix     = std   # 日志文件名前缀，如果为"std"，则输出到标准设备
 * show_pid   = YES   # 是否输出pid，可取值YES/NO
 * show_line  = YES   # 是否输出文件名/行号，可取之YES/NO
 * base_level = TRACE # 基本输出日志级别，可取值从低到高有DEBUG/INFO/NOTICE/WARN/ERROR/CRIT/FATAL
 * show_level = YES   # 是否输出级别信息，可取值有YES/NO
 * switch_by  = SIZE  # 轮换方式，可取值SIZE/TIME
 * switch_size= 100   # 轮换文件大小，当switch_by为size时有效，单位为M，最小为10
 * max_fileage= 1440  # 时间轮换周期，当switch_by为time时有效，单位为“分钟”, 最大1440
 * rolling_num= 7     # 日志数以滚动计数，最多255个，缺省为7
 */
Logger* Logger::createLoggerFromProfile(const char *profile, const char *section)
{
	const char *rootPath;
	const char *prefix;
	int showPid;
	int baseLevel;
	int showLevel;
	int showLine;
	int switchBy;
	int switchSize;
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
		return NULL;
	prefix = parser.get_string(LOG_SECTION, "prefix");
	if (prefix == NULL)
		return NULL;
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
	
	switchBy = 0;
	tmpStr = parser.get_string(LOG_SECTION, "switch_by");
	if (tmpStr != NULL) {
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

// 调用logImpl的参数
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

// 输出到标准设备
static void stdLog(LogParam *logParam)
{
	FILE* out = logParam->m_logData->m_fLog;
	LogHandle *log = logParam->m_log;
	LogData *logData = logParam->m_logData;
	
	// 输出时间
	logData->getCurrTime(log->m_timeFmt.c_str());
	fprintf(out, "%s ", logData->m_time);
	
	// 输出日志级别
	if (log->m_options & LLO_OUTPUT_LEVEL)
	{
		fprintf(out, "[%s]: ", LOG_LEVEL[logParam->m_level]);
	}

	if (log->m_options & LLO_OUTPUT_MASK)
	{
		fprintf(out, "(");
		if ((log->m_options & LLO_OUTPUT_PRGNAME))
		{
			// 输出程序名、模块名，就不再输出文件名、行号了
			fprintf(out, "%s,%s ", log->m_prgName.c_str(), log->m_modName.c_str());
		}
		else if ((log->m_options & LLO_OUTPUT_FILELINE) && (logParam->m_fileName != NULL))
		{
			// 输出文件名和行号
			fprintf(out, "%s:%d ", logParam->m_fileName, logParam->m_line);
		}
		if (log->m_options & LLO_OUTPUT_PID)
		{
			// 需要输出pid 
			fprintf(out, "%u,%u", getpid(), (unsigned)pthread_self());
		}
		fprintf(out, ") ");
	}

	// 输出用户自定义信息 
	fprintf(out, "%s\n", logParam->m_msg);
}

// 输出日志
static int logImpl(LogParam* logParam)
{
	char file[MAX_PATH_LEN];
	char logStr[MAX_LOG_LEN]; // 保存当前日志
	char *tmpLog = NULL;      // 当前日志大于MAX_LOG_LEN时，动态申请空间保存当前日志
	int logLen = 0;
	LogHandle *log = logParam->m_log;
	LogData* logData = logParam->m_logData;
	
	if (logData->m_usingStd)
	{
		stdLog(logParam);
		return 0;
	}
	
	// 合成日志
	logLen = 0;

	// 输出时间
	logData->getCurrTime(log->m_timeFmt.c_str());
	logLen += sprintf(logStr + logLen, "%s ", logData->m_time);
	
	// 输出日志级别
	if (log->m_options & LLO_OUTPUT_LEVEL)
	{
		logLen += sprintf(logStr + logLen, "[%s]: ", LOG_LEVEL[logParam->m_level]);
	}

	if (log->m_options & LLO_OUTPUT_MASK)
	{
		logLen += sprintf(logStr + logLen, "(");

		if ((log->m_options & LLO_OUTPUT_PRGNAME))
		{
			// 输出程序名、模块名，就再输出文件名、行号了
			logLen += sprintf(logStr + logLen, "%s,%s ", log->m_prgName.c_str(), log->m_modName.c_str());
		}
		else if ((log->m_options & LLO_OUTPUT_FILELINE) && (logParam->m_fileName != NULL))
		{
			// 输出文件名和行号
			logLen += sprintf(logStr + logLen, "%s:%d ", logParam->m_fileName, logParam->m_line);
		}
		if (log->m_options & LLO_OUTPUT_PID)
		{
			// 需要输出pid
			logLen += sprintf(logStr + logLen, "%u,%u", getpid(), (unsigned)pthread_self());
		}
		logLen += sprintf(logStr + logLen, ") ");
	}
	
	// 把日志合成完整的字符串
	if (logLen + logParam->m_msgLen + 1 < MAX_LOG_LEN)
	{
		memcpy(logStr+logLen, logParam->m_msg, logParam->m_msgLen);
		logLen += logParam->m_msgLen;;
		logStr[logLen++] = '\n';
	}
	else
	{
		// 当前日志大于MAX_LOG_LEN 
		tmpLog = (char*)malloc(logLen + logParam->m_msgLen+1);
		if (tmpLog == NULL)
			return 1; // 申请内存失败
		memcpy(tmpLog, logStr, logLen);
		memcpy(tmpLog+logLen, logParam->m_msg, logParam->m_msgLen);
		logLen += logParam->m_msgLen;;
		tmpLog[logLen++] = '\n';
	}

	// 判断是否需要时间轮换
	if (log->m_options & LLO_SWITCH_BY_TIME)
	{
		if (logData->m_logTime >= logData->m_rotateTime)
		{
			fclose(logData->m_fLog);
			logData->m_fLog = NULL;
			logData->m_size = 0;
			
			// 生成新的日志文件
			if (!createLogFile(file, log))
			{
				if (tmpLog != NULL)
					free(tmpLog);
				return 2; // 创建日志文件失败
			}
		}
	}
	
	// 输出到日志文件
	fwrite(tmpLog?tmpLog:logStr, 1, logLen, logData->m_fLog);
	if (tmpLog != NULL)
		free(tmpLog);
	logData->m_size += logLen;
	
	// 是否需要大小轮换
	if ((log->m_options & LLO_SWITCH_BY_SIZE) && (logData->m_size >= log->m_switchSize))
	{
		fclose(logData->m_fLog);
		logData->m_fLog = NULL;
		logData->m_size = 0;
		logData->m_currNo++;
		
		if (!createLogFile(file, log))
			return 3; // 创建文件失败
	}
	
	return 0;
}

/**
 * 输出日志，带"_r"为线程安全的函数
 * @param fileName  调用该函数的源文件名
 * @param lineNo    调用该函数的源文件行号
 * @param level     日志级别
 * @param format    输出日志的格式化串，参考printf
 * @return 0: 成功; <0: 各种错误码
 */
int Logger::log(const char *fileName, int lineNo, int level, const char *format, ...)
{
	va_list ap;
	char *msg = NULL;
	int msgLen;
	int ret;
	
	if (format == NULL)
		return 1; // 参数错误
	
	// 检查日志级别
	if (level <= L_MIN_LEVEL || level >= TOTAL_LEVEL || level < m_log->m_baseLevel)
		return 0;
	
	// 合成自定义日志
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
		return 1; // 参数错误
	
	// 检查日志级别
	if (level <= L_MIN_LEVEL || level >= TOTAL_LEVEL || level < m_log->m_baseLevel)
		return 0;
	
	// 合成自定义日志
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
 * 输出日志(转发其它接口的调用)，带"_r"为线程安全的函数
 * @param level     日志级别
 * @param format    输出日志的格式化串，参考printf
 * @param ap        其它接口的可变参数列表
 * @return 0: 成功; <0: 各种错误码
 */
int Logger::log(  int level, const char *format, va_list &ap)
{
	char *msg = NULL;
	int msgLen;;
	int ret;
	
	if (format == NULL)
		return 1; // 参数错误
	
	// 检查日志级别
	if (level <= L_MIN_LEVEL || level >= TOTAL_LEVEL || level < m_log->m_baseLevel)
		return 0;
	
	// 合成自定义日志
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
		return 1; // 参数错误
	
	// 检查日志级别
	if (level <= L_MIN_LEVEL || level >= TOTAL_LEVEL || level < m_log->m_baseLevel)
		return 0;
	
	// 合成自定义日志
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
 * 重新设置基本输出级别
 * @param baseLevel 基本输出级别
 * @return 0: 错误; >0: 旧的基本输出级别
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
 * 重新设置日志滚动个数
 * @param rollingNum 日志滚动个数
 * return <0: 错误; >=0: 旧的日志滚动个数
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
 * 模拟log4cpp而引入
 * @param prgName 程序名
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
