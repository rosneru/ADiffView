TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11 -Wall -Wconversion -Wextra -pedantic #-Weffc++

INCLUDEPATH +=  ../ \
                ../../worker \
                ../../oscottlibs

SOURCES += \
    DiffEngine_test_plain.cpp \
    ../DiffEngine.cpp \
    ../DiffLine.cpp \
    ../../oscottlibs/LinkedList.cpp \
    ../../oscottlibs/LinkedListNode.cpp \
    ../../oscottlibs/std::string.cpp \
    ../DiffFileLinux.cpp \
    ../Pair.cpp \
    ../DiffFileBase.cpp


HEADERS += \
    ../DiffEngine.h \
    ../DiffLine.h \
    ../../oscottlibs/LinkedList.h \
    ../../oscottlibs/LinkedListNode.h \
    ../../oscottlibs/std::string.h \
    ../DiffFileLinux.h \
    ../../oscottlibs/Array.h \
    ../Pair.h \
    ../DiffFileBase.h
