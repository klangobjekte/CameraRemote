#-------------------------------------------------
#
# Project created by QtCreator 2014-02-17T18:35:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets \
                                    network

TARGET = CameraRemote
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    remote.cpp \
    ssdpclient.cpp \
    timelapse.cpp \
    networkconnection.cpp \
    controllstation.cpp

HEADERS  += mainwindow.h \
    remote.h \
    ssdpclient.h \
    timelapse.h \
    networkconnection.h \
    cameraremotedefinitions.h \
    controllstation.h

FORMS    += mainwindow.ui

RESOURCES += \
    cameraremoteres.qrc
