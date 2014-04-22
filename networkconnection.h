#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H
#include <QObject>
#include <QUrl>
#include <QStringList>
#include <QNetworkConfiguration>
#include <QHttpMultiPart>
#include <QXmlStreamReader>
#include "cameraremotedefinitions.h"

class QNetworkConfigurationManager;
class QUdpSocket;
class QNetworkAccessManager;
class QNetworkReply;
class QTreeWidget;

class NetworkConnection : public QObject
{
     Q_OBJECT
public:
    explicit NetworkConnection();
    ~NetworkConnection();
    void init();
    void notifyConnectionStatus(int status = _CONNECTIONSTATE_DISCONNECTED ,QString message = QString());
    void setUrl(QString urlstring);
    void setPort(QString portstring);
    QUrl getUrl();
    QNetworkConfiguration getActiveConfiguration();
    QStringList getAvailableNetWorks();
    void setActiveNetwork(QString networkName);

signals:
    void publishUrl(QString urlstring);
    void publishPort(QString port);
    void publishAvailableNetworks(QStringList networks);
    void publishDeviceFound(QStringList data);
    //void publishDeviceFound();
    void publishConnectionStatus(int status,QString message = QString());

private slots:
    void readPendingDatagrams();
    void replyFinished(QNetworkReply *reply);

private:
    void decodeDatagramm(QByteArray datagramm);
    void parseXml();
    //notifyReceived(QHttpRequestHeader *datagram);
    QXmlStreamReader xml;
    QNetworkAccessManager *downloadManager;
    QUdpSocket *udpSocket;

    QNetworkConfigurationManager *networkConfigurationManager;
    QNetworkConfiguration activeConfiguration;
    QStringList _availableNetworks;
    QString _networkName;
    QUrl url;

    QMap<QString,QString> httpresponse;
    QString linkString;
    QString friendlyName;
    QString titleString;
    QString manufacturer;
    QString manufacturerURL;
    QString modelDescription;
    QString modelName;
    QString UDN;


    //QTreeWidget *treeWidget;

};

#endif // NETWORKCONNECTION_H
