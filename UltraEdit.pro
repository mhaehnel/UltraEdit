#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T19:37:04
#
#-------------------------------------------------

QT       += core gui concurrent multimedia svg

LIBS += -ldrumstick-alsa

CONFIG += c++11

QMAKE_CXXFLAGS_DEBUG += -pg -Wall -pedantic -Werror -Wextra
QMAKE_LFLAGS_DEBUG += -pg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UltraEdit
TEMPLATE = app

VPATH=ui/ src/ include/
INCLUDEPATH=include/

SOURCES += main.cpp\
        mainwindow.cpp \
    songinfo.cpp \
    selectsongdirs.cpp \
    song.cpp \
    songframe.cpp \
    songgroup.cpp \
    audioplayer.cpp \
    validatorsettings.cpp \
    validator.cpp \
    pathinstanceselector.cpp \
    sylabel.cpp \
    notewidget.cpp \
    midiplayer.cpp \
    midithread.cpp

HEADERS  += mainwindow.h \
    songinfo.h \
    selectsongdirs.h \
    song.h \
    songframe.h \
    songgroup.h \
    audioplayer.h \
    validatorsettings.h \
    validator.h \
    pathinstanceselector.h \
    notewidget.h \
    midiplayer.h \
    midithread.h \
    sylabel.h

FORMS    += mainwindow.ui \
    songinfo.ui \
    selectsongdirs.ui \
    songframe.ui \
    songgroup.ui \
    audioplayer.ui \
    validatorsettings.ui \
    pathinstanceselector.ui \
    noteview.ui \
    notewidget.ui

RESOURCES += \
    ultraedit.qrc
