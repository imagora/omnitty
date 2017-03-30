TEMPLATE = app


CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


QMAKE_CXXFLAGS += -std=c++0x -g


macx {
LIBS += -L/usr/local/lib -lrote -lncurses -llog4cplus -ljsoncpp
INCLUDEPATH += /usr/local/include/
}

unix:!macx{
LIBS += -L/usr/local/lib -lrote -llog4cplus
LIBS += -L/usr/lib/x86_64-linux-gnu -lncurses -ljsoncpp
INCLUDEPATH += /usr/include
INCLUDEPATH += /usr/local/include
}


SOURCES += \
    ../../src/curutil.cpp \
    ../../src/machine.cpp \
    ../../src/main.cpp \
    ../../src/menu.cpp \
    ../../src/machine_manager.cpp \
    ../../src/window_manager.cpp \
    ../../src/config.cpp \
    ../../src/opt_parser.cpp

HEADERS += \
    ../../src/curutil.h \
    ../../src/machine.h \
    ../../src/menu.h \
    ../../src/machine_manager.h \
    ../../src/window_manager.h \
    ../../src/log.h \
    ../../src/config.h \
    ../../src/opt_parser.h \
    ../../src/utils.h


