#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T19:37:04
#
#-------------------------------------------------

QT       += core gui concurrent multimedia svg multimediawidgets

LIBS += -ldrumstick-alsa -lfftw3 -lm

CONFIG += c++14

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
    collectioneditor.cpp \
    song.cpp \
    songframe.cpp \
    songgroup.cpp \
    mediaplayer.cpp \
    pathinstanceselector.cpp \
    sylabel.cpp \
    notewidget.cpp \
    midiplayer.cpp \
    midithread.cpp \
    src/videowidget.cpp \
    src/exceptions/songparseexception.cpp \
    src/actions/convertrelative.cpp \
    src/exceptions/sylabelformatexception.cpp \
    src/actions/transposesong.cpp \
    src/actions/modifytag.cpp \
    src/validators/lyricsinorder.cpp \
    src/actions/orderlyrics.cpp \
    src/actions/modifygap.cpp \
    src/audiotrace.cpp \
    src/songimportdialog.cpp \
    src/collection.cpp \
    src/actions/transfertocollection.cpp

HEADERS  += mainwindow.h \
    songinfo.h \
    collectioneditor.h \
    song.h \
    songframe.h \
    songgroup.h \
    mediaplayer.h \
    pathinstanceselector.h \
    notewidget.h \
    midiplayer.h \
    midithread.h \
    sylabel.h \
    include/videowidget.h \
    include/exceptions/songparseexception.h \
    include/actions/action.h \
    include/actions/convertrelative.h \
    include/exceptions/sylabelformatexception.h \
    include/actions/transposesong.h \
    include/actions/modifytag.h \
    include/validators/validator.h \
    include/validators/lyricsinorder.h \
    include/actions/orderlyrics.h \
    include/actions/modifygap.h \
    include/audiotrace.h \
    include/songimportdialog.h \
    include/collection.h \
    include/actions/transfertocollection.h

FORMS    += mainwindow.ui \
    songinfo.ui \
    collectioneditor.ui \
    songframe.ui \
    songgroup.ui \
    mediaplayer.ui \
    validatorsettings.ui \
    pathinstanceselector.ui \
    noteview.ui \
    notewidget.ui \
    ui/actionitem.ui \
    ui/songimportdialog.ui

RESOURCES +=  ultraedit.qrc
