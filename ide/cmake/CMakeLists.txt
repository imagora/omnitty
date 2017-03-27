cmake_minimum_required(VERSION 3.6)
project(omnitty)


set(CMAKE_CXX_STANDARD 11)


include_directories(
        /usr/local/include
        /usr/local/Cellar/log4cplus/1.2.0/include
        /usr/local/Cellar/jsoncpp/1.8.0/include
)
link_directories(
        /usr/local/lib
        /usr/local/Cellar/log4cplus/1.2.0/lib
        /usr/local/Cellar/jsoncpp/1.8.0/lib
)
link_libraries(ncurses rote log4cplus jsoncpp)


set(SRCPATH ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
set(SOURCE_FILES
        ${SRCPATH}/curutil.cpp
        ${SRCPATH}/config.cpp
        ${SRCPATH}/opt_parser.cpp
        ${SRCPATH}/menu.cpp
        ${SRCPATH}/machine.cpp
        ${SRCPATH}/machine_manager.cpp
        ${SRCPATH}/window_manager.cpp
        ${SRCPATH}/main.cpp
)
set(HEADER_FILES
        ${SRCPATH}/utils.h
        ${SRCPATH}/log.h
        ${SRCPATH}/curutil.h
        ${SRCPATH}/config.h
        ${SRCPATH}/opt_parser.h
        ${SRCPATH}/menu.h
        ${SRCPATH}/machine.h
        ${SRCPATH}/machine_manager.h
        ${SRCPATH}/window_manager.h
)


add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
