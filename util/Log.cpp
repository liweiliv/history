/**
 * 兼容以前使用log4cpp的Log.h，建议直接使用logger.h
 */
#include "Log.h"
#include "logger.h"

using namespace std;

static Logger* g_logger = NULL;

int Log::Init(const char* configFile, const char *prgName, const char *modName)
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

void Log::Shutdown()
{
	if (g_logger != NULL)
	{
		delete g_logger;
		g_logger = NULL;
	}
}

void Log::Debug(const string& msg)
{
	logDebug(g_logger, msg.c_str());
}

void Log::Info(const string& msg)
{
	logInfo(g_logger, msg.c_str());
}

void Log::Notice(const string& msg)
{
	logNotice(g_logger, msg.c_str());
}

void Log::Warn(const string& msg)
{
	logWarn(g_logger, msg.c_str());
}

void Log::Error(const string& msg)
{
	logError(g_logger, msg.c_str());
}

void Log::Crit(const string& msg)
{
	logCrit(g_logger, msg.c_str());
}

void Log::Fatal(const string& msg)
{
	logFatal(g_logger, msg.c_str());
}

void Log::Debug(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log(L_DEBUG, format, ap);
	va_end(ap);
}

void Log::Info(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log(L_INFO, format, ap);
	va_end(ap);
}

void Log::Notice(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log(L_NOTICE, format, ap);
	va_end(ap);
}

void Log::Warn(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log(L_WARN, format, ap);
	va_end(ap);
}

void Log::Error(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log(L_ERROR, format, ap);
	va_end(ap);
}

void Log::Crit(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log(L_CRIT, format, ap);
	va_end(ap);
}

void Log::Fatal(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	g_logger->log(L_FATAL, format, ap);
	va_end(ap);
}

void Log::SetLogLevel(const char *level)
{
	int l= Logger::strToLevel(level);
	if (l > 0)
		g_logger->setBaseLevel(l);
}
