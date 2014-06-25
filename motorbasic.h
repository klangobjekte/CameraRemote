#ifndef MOTORBASIC_H
#define MOTORBASIC_H

class QUdpSocket;
class QString;

class MotorBasic
{
public:
    MotorBasic();
    void move(QString port, QString move);

private:
    QUdpSocket *udpSocket;
};

#endif // MOTORBASIC_H
