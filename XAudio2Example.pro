QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR = $$PWD/bin
CONFIG += c++17
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS = $$QMAKE_LFLAGS_RELEASE_DEBUGINFO

SOURCES += \
    XAudioSound.cpp \
    main.cpp \
    Widget.cpp

HEADERS += \
    Widget.h \
    XAudioSound.h


msvc:{
QMAKE_CXXFLAGS += /wd"4819"
#QMAKE_CXXFLAGS += -execution-charset:utf-8
#QMAKE_CXXFLAGS += -source-charset:utf-8
#QMAKE_CXXFLAGS += /utf-8
}
win32:{
LIBS += -lxaudio2 -lole32
}
