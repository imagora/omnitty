TEMPLATE = app


CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


LIBS += -L/usr/local/lib -lrote -lncurses -L/usr/local/Cellar/log4cplus/1.2.0/lib -llog4cplus
INCLUDEPATH += /usr/local/include/ /usr/local/Cellar/log4cplus/1.2.0/include


SOURCES += \
    ../../src/curutil.cpp \
    ../../src/machine.cpp \
    ../../src/main.cpp \
    ../../src/menu.cpp \
    ../../src/machine_manager.cpp \
    ../../src/window_manager.cpp

HEADERS += \
    ../../src/curutil.h \
    ../../src/machine.h \
    ../../src/menu.h \
    ../../src/machine_manager.h \
    ../../src/window_manager.h \
    ../../src/log.h


