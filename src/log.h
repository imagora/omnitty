#pragma once
#include <log4cplus/logger.h>
#include <log4cplus/layout.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loggingmacros.h>
#include "config.h"


namespace omnitty {


static const std::string LOGGER_NAME("omnitty");


inline void InitLogger()
{
    // Load configration from ConfigFile
    const std::string &logConfigFile = OmniConfig::GetInstance()->GetLogConfigFilePath();
    if (!logConfigFile.empty()) {
        log4cplus::initialize();
        log4cplus::PropertyConfigurator::doConfigure(logConfigFile);
        return;
    }

    // User file appender and load log format from ConfigFile.
    const std::string &logFile = OmniConfig::GetInstance()->GetLogFilePath();
    log4cplus::SharedAppenderPtr appenderPtr(new log4cplus::FileAppender(logFile));
    const std::string &logFormat = OmniConfig::GetInstance()->GetLogFormat();
    if (!logFormat.empty()) {
        std::auto_ptr<log4cplus::Layout> layoutPtr(new log4cplus::PatternLayout(logFormat));
        appenderPtr->setLayout(layoutPtr);
    }
    log4cplus::Logger::getInstance(LOGGER_NAME).addAppender(appenderPtr);
}


}

