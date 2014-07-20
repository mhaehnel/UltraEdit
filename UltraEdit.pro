#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T19:37:04
#
#-------------------------------------------------

QT       += core gui concurrent multimedia

CONFIG += c++11

QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UltraEdit
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    songinfo.cpp \
    selectsongdirs.cpp \
    song.cpp \
    songframe.cpp \
    songgroup.cpp \
    audioplayer.cpp \
    validatorsettings.cpp \
    validator.cpp

HEADERS  += mainwindow.h \
    songinfo.h \
    selectsongdirs.h \
    song.h \
    songframe.h \
    songgroup.h \
    audioplayer.h \
    validatorsettings.h \
    validator.h

FORMS    += mainwindow.ui \
    songinfo.ui \
    selectsongdirs.ui \
    songframe.ui \
    songgroup.ui \
    audioplayer.ui \
    validatorsettings.ui

RESOURCES += \
    ultraedit.qrc
