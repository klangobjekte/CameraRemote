#-------------------------------------------------
#
# Project created by QtCreator 2014-02-17T18:35:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets \
                                    network

QMAKE_MAC_SDK = macosx10.9
#QMAKE_MAC_SDK = iphoneos8.0
#QMAKE_MAC_SDK = iphoneos8.1

TARGET = CameraRemote
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    remote.cpp \
    ssdpclient.cpp \
    timelapse.cpp \
    networkconnection.cpp \
    controllstation.cpp \
    QsLog.cpp \
    QsLogDest.cpp \
    QsLogDestConsole.cpp \
    QsLogDestFile.cpp \
    motorbasic.cpp \
    liveviewthread.cpp \
    ringbuffer.cpp

HEADERS  += mainwindow.h \
    remote.h \
    ssdpclient.h \
    timelapse.h \
    networkconnection.h \
    cameraremotedefinitions.h \
    controllstation.h \
    QsLog.h \
    QsLogDest.h \
    QsLogLevel.h \
    QsLogDestConsole.h \
    QsLogDestFile.h \
    motorbasic.h \
    liveviewthread.h \
    ringbuffer.h \
    qringbuffer.h \
    common.h \
    myconstants.h

FORMS    += mainwindow.ui

RESOURCES += \
    cameraremoteres.qrc
