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

#Solve the linker Problem in Debug Modus:
CONFIG += c++11

#mac: CONFIG += MAC_CONFIG

#MAC_CONFIG {
#    QMAKE_CXXFLAGS = -std=c++11 -stdlib=libstdc++ -mmacosx-version-min=10.7
#    QMAKE_LFLAGS = -std=c++11 -stdlib=libstdc++ -mmacosx-version-min=10.7
#}


mac{
    #QT_CONFIG -= no-pkg-config
    #CMAKE_CXX_FLAGS += -std=c++11
    #CMAKE_CXX_FLAGS += -stdlib=stdlibc++
    #CMAKE_CXX_FLAGS += $(pkg-config --cflags --libs opencv)

    #QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    #QMAKE_MAC_SDK = macosx10.9

    #QMAKE_MAC_SDK = iphoneos8.0
    #QMAKE_MAC_SDK = iphoneos8.1
    LIBS =  -L/usr/local/Cellar/opencv/2.4.9/lib
    #LIBS =  -L/usr/local/Cellar/pkg-config/0.28/bin

    LIBS += -lopencv_core
    LIBS += -lopencv_imgproc
    LIBS += -lopencv_highgui
    #-lopencv_calib3d \
    #-lopencv_contrib \
    #-lopencv_features2d \
    #-lopencv_flann \
    #-lopencv_gpu \
    #-lopencv_highgui \
    #-lopencv_legacy \
    #-lopencv_ml \
    #-lopencv_objdetect \
    #-lopencv_video

    INCLUDEPATH += /usr/local/Cellar/opencv/2.4.9/include

#INCLUDEPATH += `pkg-config --cflags opencv`
#LIBS += `pkg-config --libs opencv`

#LIBS+=/opt/local/lib/libcxcore.dylib
#LIBS+=/opt/local/lib/libcvaux.dylib
#LIBS+=/opt/local/lib/libcv.dylib
#LIBS+=/opt/local/lib/libhighgui.dylib
#LIBS+=/opt/local/lib/libml.dylib
#PKG_CONFIG_LIBDIR = /usr/local/Cellar/pkg-config/0.28/bin/

#CONFIG += link_pkgconfig
#PKGCONFIG = gtk+-2.0
#PKGCONFIG = /usr/local/Cellar/pkg-config/0.28/bin/pkg-config
#PKGCONFIG = /usr/local/Cellar/opencv/2.4.9/lib/pkgconfig/opencv
#PKGCONFIG += '/usr/local/Cellar/opencv/2.4.9/lib/pkgconfig/opencv.pc'
#PKGCONFIG += opencv
#mac: CONFIG += MAC_CONFIG


 #   QMAKE_CXXFLAGS = -std=c++11 -stdlib=libstdc++ -mmacosx-version-min=10.7
 #   QMAKE_LFLAGS = -std=c++11 -stdlib=libstdc++ -mmacosx-version-min=10.7



}


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
