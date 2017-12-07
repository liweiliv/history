/**
 * 线程安全的日志，兼容以前使用log4cpp的Log_r.h，建议直接使用logger.h
 */
#ifndef _LOG_R_H_
#define _LOG_R_H_

#include <string>

class Log_r
{
public:
	static int Init(const char* configFile, const char *prgName, const char *modName);
	static void Shutdown();
		
	static void Debug(const std::string& msg);
	static void Info(const std::string& msg);
	static void Notice(const std::string& msg);
	static void Warn(const std::string& msg);
	static void Error(const std::string& msg);
	static void Crit(const std::string& msg);
	static void Fatal(const std::string& msg);

	static void Debug(const char* format, ...) __attribute__((format(printf, 1, 2)));
	static void Info(const char* format, ...)  __attribute__((format(printf, 1, 2)));
	static void Notice(const char* format, ...)__attribute__((format(printf, 1, 2)));
	static void Warn(const char* format, ...)  __attribute__((format(printf, 1, 2)));
	static void Error(const char* format, ...) __attribute__((format(printf, 1, 2)));
	static void Crit(const char* format, ...)  __attribute__((format(printf, 1, 2)));
	static void Fatal(const char* format, ...) __attribute__((format(printf, 1, 2)));

	static void SetLogLevel(const char *level);
};		

#endif
