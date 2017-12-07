/**
 * 线程安全的日志，兼容以前使用log4cpp的Log_r.h，建议直接使用logger.h
 */
#include "Log_r.h"
#include "logger.h"

using namespace std;

static Logger* g_logger = NULL;

int Log_r::Init(const char* configFile, const char *prgName, const char *modName)
{
	Logger *logger = Logger::createLoggerFromProfile(configFile, modName);
	if (logger != NULL)
	{
		logger->setPrgAndModName(prgName, modName);
		g_logger = logger;
		return 0;
	}
	return 1;
}

void Log_r::Shutdown()
{
	if (g_logger != NULL)
	{
		delete g_logger;
		g_logger = NULL;
	}
}

void Log_r::Debug(const string& msg)
{
	logDebug_r(g_logger, msg.c_str());
}

void Log_r::Info(const string& msg)
{
	logInfo_r(g_logger, msg.c_str());
}

void Log_r::Notice(const string& msg)
{
	logNotice_r(g_logger, msg.c_str());
}

void Log_r::Warn(const string& msg)
{
	logWarn_r(g_logger, msg.c_str());
}

void Log_r::Error(const string& msg)
{
	logError_r(g_logger, msg.c_str());
}

void Log_r::Crit(const string& msg)
{
	logCrit(g_logger, msg.c_str());
}

void Log_r::Fatal(const string& msg)
{
	logFatal(g_logger, msg.c_str());
}

void Log_r::Debug(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log_r(L_DEBUG, format, ap);
	va_end(ap);
}

void Log_r::Info(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log_r(L_INFO, format, ap);
	va_end(ap);
}

void Log_r::Notice(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log_r(L_NOTICE, format, ap);
	va_end(ap);
}

void Log_r::Warn(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log_r(L_WARN, format, ap);
	va_end(ap);
}

void Log_r::Error(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log_r(L_ERROR, format, ap);
	va_end(ap);
}

void Log_r::Crit(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log_r(L_CRIT, format, ap);
	va_end(ap);
}

void Log_r::Fatal(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log_r(L_FATAL, format, ap);
	va_end(ap);
}

void Log_r::SetLogLevel(const char *level)
{
	int l= Logger::strToLevel(level);
	if (l > 0)
		g_logger->setBaseLevel(l);
}
