#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T19:37:04
#
#-------------------------------------------------

QT       += core gui concurrent

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
    collectionscanner.cpp \
    songframe.cpp

HEADERS  += mainwindow.h \
    songinfo.h \
    selectsongdirs.h \
    song.h \
    collectionscanner.h \
    songframe.h

FORMS    += mainwindow.ui \
    songinfo.ui \
    selectsongdirs.ui \
    songframe.ui

RESOURCES += \
    ultraedit.qrc
