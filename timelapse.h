#ifndef TIMELAPSE_H
#define TIMELAPSE_H
#include <QObject>
class QTimer;
class QTime;

class Timelapse : public QObject
{
        Q_OBJECT
public:
    explicit Timelapse(QObject *parent = 0);
    void setInterval(const QTime &time);
    void setDuration(const QTime &time);
    void start();
    void stop();

public slots:

signals:
    void publishTakePicture();

private:
    QTimer *timer;
    QTime *time;
    long _duration;
private slots:
    void timeOut();

};

#endif // TIMELAPSE_H
