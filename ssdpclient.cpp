#include "ssdpclient.h"




SSDPClient::SSDPClient(QObject *parent) :
    QObject(parent),
    running(false)
{
}
/*
SSDPClient::~SSDPClient() {
    if (isRunning())
        stop();

    delete this->udpListener;
}

void SSDPClient::start() {
    if (!isRunning()) {
        this->udpListener = new BrisaUdpListener("239.255.255.250", 1900,
                                                 "SSDPClient");
        connect(this->udpListener, SIGNAL(readyRead()), this, SLOT(datagramReceived()));
        this->udpListener->start();
        running = true;
    } else {
        qDebug() << "Brisa SSDP Client: Already running!";
    }
}

void SSDPClient::stop() {
    if (isRunning()) {
        this->udpListener->disconnectFromHost();;
        running = false;
    } else {
        qDebug() << "Brisa SSDP Client: Already stopped!";
    }
}

bool SSDPClient::isRunning() const {
    return running;
}

void SSDPClient::datagramReceived() {
    while (this->udpListener->hasPendingDatagrams()) {
        QByteArray *datagram = new QByteArray();

        datagram->resize(udpListener->pendingDatagramSize());
        udpListener->readDatagram(datagram->data(), datagram->size());

        QString temp(datagram->data());
        QHttpRequestHeader *parser = new QHttpRequestHeader(temp);

        notifyReceived(parser);

        delete datagram;
        delete parser;
    }

}

void SSDPClient::notifyReceived(QHttpRequestHeader *datagram) {
    if (!datagram->hasKey("nts"))
        return;

    if (datagram->value("nts") == "ssdp:alive") {
        emit newDeviceEvent(datagram->value("usn"),
                            datagram->value("location"), datagram->value("nt"),
                            datagram->value("ext"), datagram->value("server"),
                            datagram->value("cacheControl"));
        qDebug() << "Brisa SSDP Client: Received alive from " <<
                datagram->value("usn") << "";

    } else if (datagram->value("nts") == "ssdp:byebye") {
        emit removedDeviceEvent(datagram->value("usn"));
        qDebug() << "Brisa SSDP Client: Received byebye from " <<
                datagram->value("usn") << "";

    } else {
        qDebug() << "Brisa SSDP Client: Received unknown subtype: " <<
                datagram->value("nts") << "";
    }
}
*/
