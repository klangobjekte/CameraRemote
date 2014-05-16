#include "timelapse.h"
#include <QTimer>
#include <QTime>
#include <QDebug>
#include "remote.h"

#define LOG_TIMELAPSE
#ifdef LOG_TIMELAPSE
#   define LOG_TIMELAPSE_DEBUG qDebug()
#else
#   define LOG_TIMELAPSE_DEBUG nullDebug()
#endif

Timelapse::Timelapse(Remote *remote,QObject *parent) :
    QObject(parent)
{
    _remote = remote;
    _duration =0;
    timer = new QTimer;
    time = new QTime;
    connect(timer,SIGNAL(timeout()), this,SLOT(timeOut()));
}

Timelapse::~Timelapse(){
    stop();
    delete timer;
    delete time;
}

void Timelapse::setInterval(const QTime &time){

    interval = time.second()*1000+time.msec();
    LOG_TIMELAPSE_DEBUG << "Timelapse setInterval: " << time.second()  << time.msec() << interval;
    timer->setInterval(interval);
}

void Timelapse::setDuration(const QTime &time){
    long duration = time.hour()*60*60*1000 +time.minute()*60*1000 +time.second()*1000;
    LOG_TIMELAPSE_DEBUG << "Timelapse setDuration: " << time.hour()  << time.minute() << time.second() << duration;
    _duration = duration;
}

void Timelapse::start(){
    LOG_TIMELAPSE_DEBUG << "Timelapse::start: ";
    if(_remote->cameraReady()){

        _remote->setTimeLapsMode(true);
        _remote->getEvent("false");
        _remote->actTakePicture();
        //_remote->awaitTakePicture();

        //emit publishTimelapseState(true);
    }
    timer->start();
    time->start();
}

void Timelapse::stop(){
    LOG_TIMELAPSE_DEBUG << "Timelapse::stop: ";
    _remote->setTimeLapsMode(false);
    timer->stop();
    timer->stop();
    //emit publishTimelapseState(false);
}

void Timelapse::timeOut(){
   //LOG_TIMELAPSE_DEBUG << "Timelapse::timeOut()" << time->elapsed() << "interval: " << timer->interval();
    timer->setInterval(interval);
    if(time->elapsed() > _duration){
        stop();
    }
    else{
         if(_remote->cameraReady()){
            _remote->setTimeLapsMode(true);
            _remote->getEvent("false");
            _remote->actTakePicture();
            //_remote->awaitTakePicture();


        }
        else{
            //_remote->getEvent();

            timer->setInterval(100);
            timer->stop();
            timer->start();


        }
    }
}

bool Timelapse::isActive(){
    return timer->isActive();
}
