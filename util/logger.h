/*
 * Copyright (c) 2012, Taobao.com
 * All rights reserved.
 *
 * �ļ����ƣ�logger.h
 * ժҪ������log4cpp����������־���
 * ���ߣ�Benkong <benkong@taobao.com>
 * ���ڣ�2012.7.2
 */
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <stdarg.h>

/**
 * ����������͵���־
 * @param logger  Loggerʵ�� 
 * @param fmt     �����־�ĸ�ʽ�������ο�printf
 * @return 0: �ɹ�; <0: ���ִ�����
 */
#define logDebug( logger, fmt, ...) (logger)->log(__FILE__, __LINE__, L_DEBUG,  fmt, ##__VA_ARGS__)
#define logInfo(  logger, fmt, ...) (logger)->log(__FILE__, __LINE__, L_INFO,   fmt, ##__VA_ARGS__)
#define logNotice(logger, fmt, ...) (logger)->log(__FILE__, __LINE__, L_NOTICE, fmt, ##__VA_ARGS__)
#define logWarn(  logger, fmt, ...) (logger)->log(__FILE__, __LINE__, L_WARN,   fmt, ##__VA_ARGS__)
#define logError( logger, fmt, ...) (logger)->log(__FILE__, __LINE__, L_ERROR,  fmt, ##__VA_ARGS__)
#define logCrit(  logger, fmt, ...) (logger)->log(__FILE__, __LINE__, L_CRIT,   fmt, ##__VA_ARGS__)
#define logFatal( logger, fmt, ...) (logger)->log(__FILE__, __LINE__, L_FATAL,  fmt, ##__VA_ARGS__)

#define logDebug_r( logger, fmt, ...) (logger)->log_r(__FILE__, __LINE__, L_DEBUG,  fmt, ##__VA_ARGS__)
#define logInfo_r(  logger, fmt, ...) (logger)->log_r(__FILE__, __LINE__, L_INFO,   fmt, ##__VA_ARGS__)
#define logNotice_r(logger, fmt, ...) (logger)->log_r(__FILE__, __LINE__, L_NOTICE, fmt, ##__VA_ARGS__)
#define logWarn_r(  logger, fmt, ...) (logger)->log_r(__FILE__, __LINE__, L_WARN,   fmt, ##__VA_ARGS__)
#define logError_r( logger, fmt, ...) (logger)->log_r(__FILE__, __LINE__, L_ERROR,  fmt, ##__VA_ARGS__)
#define logCrit_r(  logger, fmt, ...) (logger)->log_r(__FILE__, __LINE__, L_CRIT,   fmt, ##__VA_ARGS__)
#define logFatal_r( logger, fmt, ...) (logger)->log_r(__FILE__, __LINE__, L_FATAL,  fmt, ##__VA_ARGS__)

// ��־���𣬴ӵ͵���
enum LLEVEL_T
{
	L_MIN_LEVEL= 0,
	L_DEBUG    = 1,
	L_INFO     = 2,
	L_NOTICE   = 3,
	L_WARN     = 4,
	L_ERROR    = 5,
	L_CRIT     = 6,
	L_FATAL    = 7,
	TOTAL_LEVEL
};

struct LogHandle;

class Logger
{
public:
	virtual ~Logger();

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
	 * switch_size= 100   # �ֻ��ļ���С����switch_byΪsizeʱ��Ч����λΪM
	 * max_fileage= 1440  # ʱ���ֻ����ڣ���switch_byΪtimeʱ��Ч����λΪ�����ӡ�, ���1440
	 * rolling_num= 7     # ��־���Թ������������255����ȱʡΪ7
	 */
	static Logger* createLoggerFromProfile(const char *profile, const char *section);

	/**
	 * �����־����"_r"Ϊ�̰߳�ȫ�ĺ���
	 * @param fileName  ���øú�����Դ�ļ���
	 * @param lineNo    ���øú�����Դ�ļ��к�
	 * @param level     ��־����
	 * @param format    �����־�ĸ�ʽ�������ο�printf
	 * @return 0: �ɹ�; <0: ���ִ�����
	 */
	int log(  const char *fileName, int lineNo, int level, const char *format, ...);
	int log_r(const char *fileName, int lineNo, int level, const char *format, ...);

	/**
	 * �����־(ת�������ӿڵĵ���)����"_r"Ϊ�̰߳�ȫ�ĺ���
	 * @param level     ��־����
	 * @param format    �����־�ĸ�ʽ�������ο�printf
	 * @param ap        �����ӿڵĿɱ�����б�
	 * @return 0: �ɹ�; <0: ���ִ�����
	 */
	int log(  int level, const char *format, va_list &ap);
	int log_r(int level, const char *format, va_list &ap);

	/**
	 * �������û����������
	 * @param baseLevel �����������
	 * @return 0: ����; >0: �ɵĻ����������
	 */
	int setBaseLevel(int baseLevel);

	/**
	 * ����������־��������
	 * @param rollingNum ��־��������
	 * return <0: ����; >=0: �ɵ���־��������
	 */
	int setRollingNum(int rollingNum);

	/**
	 * ģ��log4cpp������
	 * @param prgName ������
	 * @param modName
	 */
	void setPrgAndModName(const char *prgName, const char *modName);

	/**
	 * �ѱ�ʾ�ַ�������־����ת��Ϊ��Ӧ������
	 * @param szLevel ��־������"INFO"/"DEBUG"��
	 * @return ���szLevel�Ϸ���������Ӧ������; ���szLevel���Ϸ�������0
	 */
	static int strToLevel(const char* level);
protected:
	Logger();
	int init(const char *logPath, const char *filePrefix, int options, const char *timeFmt, int switchSize);

	LogHandle *m_log;
public:
	static Logger *m_logger; // ȫ��logger
};

#endif
