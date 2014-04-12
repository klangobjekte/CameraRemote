#ifndef SSDPCLIENT_H
#define SSDPCLIENT_H




#include <QObject>
//#include <QHttpRequestHeader>

//#include "brisaglobal.h"
//#include "brisaudplistener.h"



/*!
 *  \class SSDPClient SSDPClient.h BrisaUpnp/SSDPClient
 *  \brief SSDP stack implementantion for UPnP control points.
 *
 *  Create a new BrisaSSCPClient and call "start()" to connect to the multicast
 *  group and start listening to ssdp notification messages.
 *
 *  When SSDPClient receives a notification message it emits \a "newDeviceEvent()"
 *  in case of "ssdp:alive" and \a "removedDeviceEvent" in case of "ssdp:byebye".
 *  Other ssdp messages will be ignored.
 */
class  SSDPClient: public QObject {
Q_OBJECT

public:
    /*!
     *  Constructs a BrisaSSCPClient with the given parent.
     */
    SSDPClient(QObject *parent = 0);

    /*!
     *  Destroys the client.
     *
     *  Stops the client if it's running.
     */
    //virtual ~SSDPClient();

public slots:
    /*!
     *
     *  Connects to the MultiCast group and starts the client.
     *
     *  \sa isRunning(), stop()
     */
    //void start();

    /*!
     *  Stops the client.
     *
     *  \sa isRunning(), start()
     */
    //void stop();

    /*!
     *  Checks if the client is running
     *
     *  \return true if is running
     */
    //bool isRunning() const;

signals:
    /*!
     *  \fn void SSDPClient::newDeviceEvent(const QString &usn, const QString &location,
     *                                           const QString &st, const QString &ext,
     *                                           const QString &server, const QString &cacheControl)
     *
     *  This signal is emitted when the client receives a "ssdp:alive" message from a device joining
     *  the network
     *
     *  \sa removedDeviceEvent()
     */
    //void newDeviceEvent(const QString &usn, const QString &location,
    //        const QString &st, const QString &ext, const QString &server,
    //        const QString &cacheControl);

    /*!
     *  \fn void SSDPClient::removedDeviceEvent(const QString &usn)
     *
     *  This signal is emitted when the client receives a "ssdp:byebye" message from a device leaving the
     *  network
     *
     *  \sa newDeviceEvent()
     */
    //void removedDeviceEvent(const QString &usn);

private slots:
    /*!
     *  \internal
     *  Receives UDP datagrams from a QUdpSocket.
     */
    //void datagramReceived();

    /*!
     *  \internal
     *  Parses the UDP datagram received from "datagramReceived()".
     *
     *  \param datagram datagram
     */
    //void notifyReceived(QHttpRequestHeader *datagram);

private:
    bool running;

    //BrisaUdpListener *udpListener;
};





#endif // SSDPCLIENT_H
