/*
 * Copyright (c) 2012, Taobao.com
 * All rights reserved.
 *
 * 文件名称：logger.h
 * 摘要：类似log4cpp的轻量级日志输出
 * 作者：Benkong <benkong@taobao.com>
 * 日期：2012.7.2
 */
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <stdarg.h>

/**
 * 输出各种类型的日志
 * @param logger  Logger实例 
 * @param fmt     输出日志的格式化串，参考printf
 * @return 0: 成功; <0: 各种错误码
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

// 日志级别，从低到高
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
	 * switch_size= 100   # 轮换文件大小，当switch_by为size时有效，单位为M
	 * max_fileage= 1440  # 时间轮换周期，当switch_by为time时有效，单位为“分钟”, 最大1440
	 * rolling_num= 7     # 日志数以滚动计数，最多255个，缺省为7
	 */
	static Logger* createLoggerFromProfile(const char *profile, const char *section);

	/**
	 * 输出日志，带"_r"为线程安全的函数
	 * @param fileName  调用该函数的源文件名
	 * @param lineNo    调用该函数的源文件行号
	 * @param level     日志级别
	 * @param format    输出日志的格式化串，参考printf
	 * @return 0: 成功; <0: 各种错误码
	 */
	int log(  const char *fileName, int lineNo, int level, const char *format, ...);
	int log_r(const char *fileName, int lineNo, int level, const char *format, ...);

	/**
	 * 输出日志(转发其它接口的调用)，带"_r"为线程安全的函数
	 * @param level     日志级别
	 * @param format    输出日志的格式化串，参考printf
	 * @param ap        其它接口的可变参数列表
	 * @return 0: 成功; <0: 各种错误码
	 */
	int log(  int level, const char *format, va_list &ap);
	int log_r(int level, const char *format, va_list &ap);

	/**
	 * 重新设置基本输出级别
	 * @param baseLevel 基本输出级别
	 * @return 0: 错误; >0: 旧的基本输出级别
	 */
	int setBaseLevel(int baseLevel);

	/**
	 * 重新设置日志滚动个数
	 * @param rollingNum 日志滚动个数
	 * return <0: 错误; >=0: 旧的日志滚动个数
	 */
	int setRollingNum(int rollingNum);

	/**
	 * 模拟log4cpp而引入
	 * @param prgName 程序名
	 * @param modName
	 */
	void setPrgAndModName(const char *prgName, const char *modName);

	/**
	 * 把表示字符串的日志基本转换为对应的数字
	 * @param szLevel 日志级别，如"INFO"/"DEBUG"等
	 * @return 如果szLevel合法，返回相应的数字; 如果szLevel不合法，返回0
	 */
	static int strToLevel(const char* level);
protected:
	Logger();
	int init(const char *logPath, const char *filePrefix, int options, const char *timeFmt, int switchSize);

	LogHandle *m_log;
public:
	static Logger *m_logger; // 全局logger
};

#endif
