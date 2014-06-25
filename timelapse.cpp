#include "timelapse.h"
#include <QTimer>
#include <QTime>
#include <QDebug>
#include "remote.h"

//#define LOG_TIMELAPSE
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
    intervalTime = new QTime;
    intervalTime->setHMS(0,0,1,0);
    tmpinterval = 0;
    timerresolution = 50;
    connect(timer,SIGNAL(timeout()), this,SLOT(timeOut()));
}

Timelapse::~Timelapse(){
    stop();
    delete timer;
    delete time;
    delete intervalTime;
}

void Timelapse::setInterval(const QTime &time){
    intervalTime->setHMS(time.hour(),time.minute(),time.second(),time.msec());
    realinterval = interval = time.second()*1000+time.msec();
    LOG_TIMELAPSE_DEBUG << "Timelapse setInterval: " << time.second()  << time.msec() << interval;

    timer->setInterval(timerresolution);
}

void Timelapse::setDuration(const QTime &time){
    long duration = time.hour()*60*60*1000 +time.minute()*60*1000 +time.second()*1000;
    LOG_TIMELAPSE_DEBUG << "Timelapse setDuration: " << time.hour()  << time.minute() << time.second() << duration;
    _duration = duration;
}

void Timelapse::start(){
    LOG_TIMELAPSE_DEBUG << "Timelapse::start: ";
    //timer->setInterval(timerresolution);
    setInterval(*intervalTime);
    _remote->setTimeLapsMode(true);
    timer->start();
    time->start();
}

void Timelapse::stop(){
    LOG_TIMELAPSE_DEBUG << "Timelapse::stop: ";
    _remote->setTimeLapsMode(false);
    timer->stop();
    timer->stop();
}

void Timelapse::timeOut(){
    static int number = 0;
    if(time->elapsed() > _duration){
        stop();
    }
    else{
         if(_remote->cameraReady()){
             if(realinterval >= interval){
                _remote->actTakePicture();
                number++;
                //! alte Zeit zum Zeitpunkt der letzten Auslösung
                tmpinterval = time->elapsed();
                LOG_TIMELAPSE_DEBUG << "Timelapse                             :  elapsed: " << time->elapsed() << "       realinterval: " << realinterval << "       pictureNumber: " << number;
             }
        }
        realinterval = time->elapsed() - tmpinterval;
    }
}

bool Timelapse::isActive(){
    return timer->isActive();
}
