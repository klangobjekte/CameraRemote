#include "networkconnection.h"
#include <QNetworkConfigurationManager>
#include <QUrl>
#include <QDebug>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTreeWidget>
#include "cameraremotedefinitions.h"


NetworkConnection::NetworkConnection()
{

    QHostAddress hostaddress("239.255.255.250");
    QString  SSDP_ADDR("239.255.255.250");
    int SSDP_PORT = 1900;
    int SSDP_MX = 1;
    QString SSDP_ST = "urn:schemas-sony-com:service:ScalarWebAPI:1";

    //QString SSDP_ST = "ssdp:all";

    //treeWidget = new QTreeWidget;

    QByteArray ssdpRequest = "M-SEARCH * HTTP/1.1\r\nHOST: ";
            ssdpRequest.append(SSDP_ADDR);
            ssdpRequest.append(":");
            ssdpRequest.append(SSDP_PORT);
            ssdpRequest.append("\r\n");
            ssdpRequest.append("MAN: \"ssdp:discover\"\r\n");
            ssdpRequest.append("MX: ");
            ssdpRequest.append(SSDP_MX);
            ssdpRequest.append("\r\n");
            ssdpRequest.append("ST: ");
            ssdpRequest.append(SSDP_ST);
            ssdpRequest.append("\r\n");
            ssdpRequest.append("\r\n");


    udpSocket = new QUdpSocket(this);
    udpSocket->bind(hostaddress,SSDP_PORT);
    //udpSocket->bind(hostaddress, QUdpSocket::ShareAddress);
    //udpSocket->bind(45454, QUdpSocket::ShareAddress);
    udpSocket->joinMulticastGroup(hostaddress);
    connect(udpSocket, SIGNAL(readyRead()),
                this, SLOT(readPendingDatagrams()));
    udpSocket->writeDatagram(ssdpRequest,hostaddress,SSDP_PORT);


    QNetworkInterface *interface = new QNetworkInterface;
    qDebug() << "interface->allAddresses(): "<< interface->allAddresses();

    //QNetworkConfigurationManager networkConfigurationManager;
    networkConfigurationManager = new QNetworkConfigurationManager;
    QList<QNetworkConfiguration> activeConfigs = networkConfigurationManager->allConfigurations();
    qDebug() << "activeConfigs.count()"  << activeConfigs.count();
    if(activeConfigs.count() == 0){
        networkConfigurationManager->updateConfigurations();
    }

    if (activeConfigs.count() > 0){
        Q_ASSERT(networkConfigurationManager->isOnline());
        foreach (QNetworkConfiguration config, activeConfigs) {
            qDebug()<< config.bearerTypeName() << config.name();
            _availableNetworks.append(config.name());
            if(config.name() == "en1")
               activeConfiguration =  config;
        }

    }
    else
    {
        Q_ASSERT(!networkConfigurationManager->isOnline());
    }
    downloadManager = new QNetworkAccessManager;
    connect(downloadManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

}



NetworkConnection::~NetworkConnection(){
    delete networkConfigurationManager;
}

void NetworkConnection::init(){
    emit publishConnectionStatus(_CONNECTIONSTATE_WATING);
}

void NetworkConnection::setActiveNetwork(QString networkName){
    _networkName = networkName;
}

void NetworkConnection::notifyConnectionStatus(int status,QString message){
    //qDebug() << "NetworkConnection::notifyConnectionStatus" << status;
    emit publishConnectionStatus(status,message);
}

void NetworkConnection::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);
        //qDebug() << "datagram: " << datagram;
        decodeDatagramm(datagram);
        if(httpresponse.value("NTS") == "ssdp:alive" &&
            httpresponse.value("NT") == "urn:schemas-sony-com:service:ScalarWebAPI:1"){
            downloadManager->get(QNetworkRequest(QUrl(httpresponse.value("LOCATION"))));
            qDebug() << "SSDP Client: Received alive from " << httpresponse.value("USN");
            emit publishConnectionStatus(_CONNECTIONSTATE_SSDP_ALIVE_RECEIVED);
        }
        else if(httpresponse.value("NTS") == "ssdp:byebye"){
            qDebug() << "SSDP Client: Received byebye from " << httpresponse.value("USN");
            emit publishConnectionStatus(_CONNECTIONSTATE_DISCONNECTED);
        }
        else {
            qDebug() << "SSDP Client: Received unknown subtype: " << httpresponse.value("NTS");
        }
    }
}

void NetworkConnection::decodeDatagramm(QByteArray datagramm){
    QString sdatagramm(datagramm);
    QString value;
    QString key;
    QStringList splitted = sdatagramm.split("\n");
    httpresponse.clear();
    //qDebug() << "splitted: " << splitted;
    foreach(QString splitter,splitted){
        splitter = splitter.trimmed();
        int keysize = splitter.indexOf(":");
        int size = splitter.size();
        if(keysize > 0){
            value = splitter.right(size-keysize-1);
            key = splitter.left(keysize);
            value = value.trimmed();
            //qDebug() << "KEY  : " << key;
            //qDebug() << "VALUE: " << value;
            httpresponse[key] = value;
        }
    }
}



void NetworkConnection::replyFinished(QNetworkReply *reply){
     QByteArray bts = reply->readAll();
     QString str(bts);
    //qDebug() << "reply: " << str;
    xml.addData(bts);
    parseXml();
    QStringList data;
    linkString.remove("/sony");
    setUrl(linkString);
    data.append(linkString);
    emit publishConnectionStatus(_CONNECTIONSTATE_CAMERA_DETECTED,friendlyName);
}


void NetworkConnection::parseXml()
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            /*
            qDebug() << "isStartElement";
            qDebug() << "xml.name() " <<xml.name().toString();
            qDebug() << "xml.attributes() " <<xml.attributes().value(xml.name().toString()).toString();
            qDebug() << "xml.text() " <<xml.text().toString();
            qDebug() << "\n ";
            */
            if(xml.name().toString() == "friendlyName"){
                friendlyName = xml.readElementText();
                qDebug() << xml.name().toString() << "     " << friendlyName;
            }
            if(xml.name().toString() == "manufacturer"){
                manufacturer = xml.readElementText();
                qDebug() << xml.name().toString() << "     " << manufacturer;
            }
            if(xml.name().toString() == "manufacturerURL"){
                manufacturerURL = xml.readElementText();
                qDebug() << xml.name().toString() << "  " << manufacturerURL;
            }
            if(xml.name().toString() == "modelDescription"){
                modelDescription = xml.readElementText();
                qDebug() << xml.name().toString() << " " << modelDescription;
            }
            if(xml.name().toString() == "modelName"){
                modelName = xml.readElementText();
                qDebug() << xml.name().toString() << "        " << modelName;
            }
            if(xml.name().toString() == "UDN"){
                UDN = xml.readElementText();
                qDebug() << xml.name().toString() << "              " << UDN;
            }
            /*
            if(xml.name().toString() == "X_ScalarWebAPI_DeviceInfo"){
                qDebug() << "xml.name() " <<xml.name().toString();
                qDebug() << "xml.readElementText() " <<xml.readElementText();
            }

            if(xml.name().toString() == "X_ScalarWebAPI_Version"){
                qDebug() << "xml.name() " <<xml.name().toString();
                qDebug() << "xml.readElementText() " <<xml.readElementText();
            }

            if(xml.name().toString() == "X_ScalarWebAPI_ServiceList"){
                qDebug() << "xml.name() " <<xml.name().toString();
                qDebug() << "xml.readElementText() " <<xml.readElementText();
            }

            if(xml.name().toString() == "X_ScalarWebAPI_Service"){
                qDebug() << "xml.name() " <<xml.name().toString();
                qDebug() << "xml.readElementText() " <<xml.readElementText();
            }
            if(xml.name().toString() == "X_ScalarWebAPI_ServiceType"){
                qDebug() << "xml.name() " <<xml.name().toString();
                qDebug() << "xml.readElementText() " <<xml.readElementText();
            }
            */
            if(xml.name().toString() == "X_ScalarWebAPI_ActionList_URL"){
                linkString = xml.readElementText();
                qDebug() << xml.name().toString() << " " << linkString;
            }
            /*
            if(xml.name().toString() == "X_ScalarWebAPI_AccessType"){
                qDebug() << "xml.name() " <<xml.name().toString();
                qDebug() << "xml.readElementText() " <<xml.readElementText();
            }
            */
            //qDebug() << "\n ";
        }
    }
    if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
    }

}


void NetworkConnection::setUrl(QString urlstring){
    qDebug() << "NetworkConnection::setUrl: " << urlstring;
    QUrl localUrl;
    if(!urlstring.isEmpty()){
        localUrl.setUrl(urlstring);
        url = localUrl;
        if(url.port() != -1){
            QString sport;
            sport.setNum(url.port());
        }
        //setPort(sport);
        //qDebug() << "NetworkConnection::setUrl url: " << url;
        emit publishUrl(urlstring);
    }
}

void NetworkConnection::setPort(QString portstring){
    qDebug() << "NetworkConnection::setPort: " << portstring;
    QUrl localUrl;
    if(!portstring.isEmpty()){
        url.setPort(portstring.toInt());
        emit publishPort(portstring);
    }
    //qDebug() << "port: " << portstring;
}


QStringList NetworkConnection::getAvailableNetWorks(){
    return _availableNetworks;
}

QNetworkConfiguration NetworkConnection::getActiveConfiguration(){
    return activeConfiguration;
}

QUrl NetworkConnection::getUrl(){
    //qDebug() << "NetworkConnection::getUrl() path: " << url.toString();
    return url;

}

/*
void NetworkConnection::notifyReceived(QHttpRequestHeader *datagram) {
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
