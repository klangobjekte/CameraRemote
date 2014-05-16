#ifndef TIMELAPSE_H
#define TIMELAPSE_H
#include <QObject>
class QTimer;
class QTime;
class Remote;

class Timelapse : public QObject
{
        Q_OBJECT
public:
    explicit Timelapse(Remote *remote, QObject *parent = 0);
    ~Timelapse();
    void setInterval(const QTime &time);
    void setDuration(const QTime &time);
    void start();
    void stop();
    bool isActive();

public slots:

signals:
    //void publishTakePicture();
    void publishTimelapseState(bool);

private:
    Remote *_remote;
    QTimer *timer;
    QTime *time;

    long _duration;
    int interval;
    int realinterval;
    int tmpinterval;
    int timerresolution;
private slots:
    void timeOut();

};

#endif // TIMELAPSE_H
