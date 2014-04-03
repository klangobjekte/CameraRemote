#include "timelapse.h"
#include <QTimer>
#include <QTime>
#include <QDebug>

Timelapse::Timelapse(QObject *parent) :
    QObject(parent)
{
    _duration =0;
    timer = new QTimer;
    time = new QTime;
    connect(timer,SIGNAL(timeout()), this,SLOT(timeOut()));
}

void Timelapse::setInterval(const QTime &time){
    int interval = time.second()*1000+time.msec();
    qDebug() << "Timelapse setInterval: " << time.second()  << time.msec() << interval;
    timer->setInterval(interval);
}

void Timelapse::setDuration(const QTime &time){
    long duration = time.hour()*60*60*1000 +time.minute()*60*1000 +time.second()*1000;
    qDebug() << "Timelapse setDuration: " << time.hour()  << time.minute() << time.second() << duration;
    _duration = duration;
}

void Timelapse::start(){
    timer->start();
    time->start();
}

void Timelapse::stop(){
    timer->stop();
    timer->stop();
}

void Timelapse::timeOut(){
    if(time->elapsed() > _duration)
        stop();
    else
        emit publishTakePicture();
}
