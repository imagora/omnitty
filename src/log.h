#pragma once
#include <log4cplus/logger.h>
#include <log4cplus/layout.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loggingmacros.h>


namespace omnitty {


static const std::string LOGGER_NAME("omnitty");


inline void InitLogger()
{
    log4cplus::SharedAppenderPtr appenderPtr(new log4cplus::FileAppender("/Users/shanhui/Workspaces/OmnittyForMac/ide/qmake/log.log"));
    log4cplus::Logger::getInstance(LOGGER_NAME).addAppender(appenderPtr);
}


}

