TEMPLATE = app


CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


LIBS += -L/usr/local/lib -lrote -lncurses
INCLUDEPATH += /usr/local/include/


SOURCES += \
    ../../src/curutil.cpp \
    ../../src/help.cpp \
    ../../src/machine.cpp \
    ../../src/main.cpp \
    ../../src/menu.cpp \
    ../../src/machine_manager.cpp \
    ../../src/window_manager.cpp

HEADERS += \
    ../../src/curutil.h \
    ../../src/help.h \
    ../../src/machine.h \
    ../../src/menu.h \
    ../../src/machine_manager.h \
    ../../src/window_manager.h


