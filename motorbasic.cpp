#include "motorbasic.h"
#include <QUdpSocket>
#include <QString>


MotorBasic::MotorBasic()
{

    QHostAddress hostaddress("192.168.0.1");
    int PORT = 50000;
    udpSocket = new QUdpSocket();
    udpSocket->bind(hostaddress,PORT);
}


void MotorBasic::move(QString port,QString move){
    QHostAddress hostaddress("192.168.0.1");
    int PORT = 50000;
    QByteArray ssdpRequest;
            ssdpRequest.append(port);
            ssdpRequest.append(",");
            ssdpRequest.append(move);
            udpSocket->writeDatagram(ssdpRequest,hostaddress,PORT);
}
