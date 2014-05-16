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
#include <QNetworkSession>
#include <QMessageBox>
#include <QString>
#include "net/ethernet.h"

//#define LOG_NETWORKCONNECTION
#ifdef LOG_NETWORKCONNECTION
#   define LOG_NETWORKCONNECTION_DEBUG qDebug()
#else
#   define LOG_NETWORKCONNECTION_DEBUG nullDebug()
#endif

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
    LOG_NETWORKCONNECTION_DEBUG << "\ninterface->allAddresses(): "<< interface->allAddresses();
    networkConfigurationManager = new QNetworkConfigurationManager;
    const bool canStartIAP = (networkConfigurationManager->capabilities()
                                 & QNetworkConfigurationManager::CanStartAndStopInterfaces);

    QList<QNetworkConfiguration> activeConfigs = networkConfigurationManager->allConfigurations();
    LOG_NETWORKCONNECTION_DEBUG << "\nactiveConfigs.count()"  << activeConfigs.count();
    //if(activeConfigs.count() == 0){
        //networkConfigurationManager->updateConfigurations();
    //}

    QNetworkConfigurationManager ncm;
    auto nc = ncm.allConfigurations();

    for (auto &x : nc)
    {
        if (x.bearerType() == QNetworkConfiguration::BearerWLAN)
        {
            //if (x.name() == "YouDesiredNetwork")
                //cfg = x;
        }
        qDebug() << "auto: " << x.name() << x.bearerType();

    }

    //auto session = new QNetworkSession(cfg, this);
    //session->open();


    QNetworkConfiguration cfg = networkConfigurationManager->defaultConfiguration();
    if (!cfg.isValid() || (!canStartIAP && cfg.state() != QNetworkConfiguration::Active)) {
        //QMessageBox::information(this, tr("Network"), tr("No Access Point found."));
        LOG_NETWORKCONNECTION_DEBUG << "No Access Point found";
        networkConfigurationManager->updateConfigurations();

    }
    else{
        activeConfiguration = cfg;
        networkSession = new QNetworkSession(cfg, this);
        networkSession->open();
        networkSession->waitForOpened(-1);
    }



    if (activeConfigs.count() > 0){
        LOG_NETWORKCONNECTION_DEBUG << "Active Configurations: ";
        foreach (QNetworkConfiguration config, activeConfigs) {
            LOG_NETWORKCONNECTION_DEBUG<< config.bearerTypeName() << config.name() << config.bearerType();
            _availableNetworks.append(config.name());
            //if(config.name() == "en0"){
            //   activeConfiguration =  config;
            //}
        }

    }

    connect(networkConfigurationManager,SIGNAL(updateCompleted()),
            this,SLOT(onUpdateCompleted()));
    connect(networkConfigurationManager,SIGNAL(configurationChanged(QNetworkConfiguration)),
            this,SLOT(onConfigurationChanged(QNetworkConfiguration)));
    connect(networkConfigurationManager,SIGNAL(configurationAdded(QNetworkConfiguration)),
            this,SLOT(onConfigurationAdded(QNetworkConfiguration)));
    connect(networkConfigurationManager,SIGNAL(configurationRemoved(QNetworkConfiguration)),
            this,SLOT(onconfigurationRemoved(QNetworkConfiguration)));

    //networkSession = new QNetworkSession(activeConfiguration);
    LOG_NETWORKCONNECTION_DEBUG << "Network Session State:" << networkSession->state();


    downloadManager = new QNetworkAccessManager;
    connect(downloadManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

}



NetworkConnection::~NetworkConnection(){
    delete networkConfigurationManager;
}


QNetworkConfigurationManager *NetworkConnection::getNetworkConfigurationManager(){
    return networkConfigurationManager;
}

void NetworkConnection::init(){
    networkConfigurationManager->updateConfigurations();
    emit publishConnectionStatus(_CONNECTIONSTATE_WATING);
}


void NetworkConnection::onUpdateCompleted(){

    QList<QNetworkConfiguration> activeConfigs = networkConfigurationManager->allConfigurations();
    _availableNetworks.clear();
    foreach (QNetworkConfiguration config, activeConfigs) {
         _availableNetworks.append(config.name());
         LOG_NETWORKCONNECTION_DEBUG<< "onUpdateCompleted activeConfigs: " << config.bearerTypeName() << config.name() << config.type();
         //if(config.name().contains("DIRECT-IDE")){
         //  activeConfiguration =  config;

        //}
    }

    LOG_NETWORKCONNECTION_DEBUG << "networkConfigurationManager->defaultConfiguration():\n"
             << "name            : "<< networkConfigurationManager->defaultConfiguration().name() << "\n"
             << "type            : "<< networkConfigurationManager->defaultConfiguration().type() << "\n"
             << "bearerTypeName  : "<< networkConfigurationManager->defaultConfiguration().bearerTypeName() << "\n"
             << "identifier      : "<< networkConfigurationManager->defaultConfiguration().identifier() << "\n"
             << "bearerTypeFamily: "<< networkConfigurationManager->defaultConfiguration().bearerTypeFamily() << "\n"
             << "bearerType      : "<< networkConfigurationManager->defaultConfiguration().bearerType() << "\n"
             << "children        : " "\n";

    foreach (QNetworkConfiguration configuration, networkConfigurationManager->defaultConfiguration().children()) {
     LOG_NETWORKCONNECTION_DEBUG   << "child           : " << "\n"
        << "name            : "<< configuration.name() << "\n"
        << "type            : "<< configuration.type() << "\n"
        << "bearerTypeName  : "<< configuration.bearerTypeName() << "\n"
        << "identifier      : "<< configuration.identifier() << "\n"
        << "bearerTypeFamily: "<< configuration.bearerTypeFamily() << "\n"
        << "bearerType      : "<< configuration.bearerType() << "\n";
    }

    const bool CanStartAndStopInterfaces = (networkConfigurationManager->capabilities()
                                 & QNetworkConfigurationManager::CanStartAndStopInterfaces);
    const bool DirectConnectionRouting = (networkConfigurationManager->capabilities()
                                 & QNetworkConfigurationManager::DirectConnectionRouting);
    const bool SystemSessionSupport = (networkConfigurationManager->capabilities()
                                 & QNetworkConfigurationManager::SystemSessionSupport);
    const bool ApplicationLevelRoaming = (networkConfigurationManager->capabilities()
                                 & QNetworkConfigurationManager::ApplicationLevelRoaming);
    const bool ForcedRoaming = (networkConfigurationManager->capabilities()
                                 & QNetworkConfigurationManager::ForcedRoaming);

    const bool DataStatistics = (networkConfigurationManager->capabilities()
                                 & QNetworkConfigurationManager::DataStatistics);

    const bool NetworkSessionRequired = (networkConfigurationManager->capabilities()
                                 & QNetworkConfigurationManager::NetworkSessionRequired);

    LOG_NETWORKCONNECTION_DEBUG << "cababilties: \n"
                << "CanStartAndStopInterfaces: " << CanStartAndStopInterfaces << "\n"
                << "DirectConnectionRouting:   " << DirectConnectionRouting << "\n"
                << "SystemSessionSupport:      " << SystemSessionSupport << "\n"
                << "ApplicationLevelRoaming:   " << ApplicationLevelRoaming << "\n"
                << "ForcedRoaming:             " << ForcedRoaming << "\n"
                << "DataStatistics:            " << DataStatistics << "\n"
                << "NetworkSessionRequired:    " << NetworkSessionRequired << "\n";



    if(!networkSession->state() == QNetworkSession::Connected){
        delete networkSession;
        const bool canStartIAP = (networkConfigurationManager->capabilities()
                                     & QNetworkConfigurationManager::CanStartAndStopInterfaces);
        QNetworkConfiguration cfg = networkConfigurationManager->defaultConfiguration();
        activeConfiguration =  cfg;
        if (!cfg.isValid() || (!canStartIAP && cfg.state() != QNetworkConfiguration::Active)) {
            //QMessageBox::information(this, tr("Network"), tr("No Access Point found."));
            LOG_NETWORKCONNECTION_DEBUG << "No Access Point found";
            //networkConfigurationManager->updateConfigurations();

        }
        else{
            activeConfiguration = cfg;
            networkSession = new QNetworkSession(cfg, this);
            networkSession->open();
            networkSession->waitForOpened(-1);
            LOG_NETWORKCONNECTION_DEBUG << "Network Session State:" << networkSession->state();
        }

    }
    LOG_NETWORKCONNECTION_DEBUG << "networkSession->interface(): " << networkSession->interface() << "\n\n";
    publishDeviceFound(_availableNetworks);

}

void NetworkConnection::onConfigurationAdded(QNetworkConfiguration configration){
    LOG_NETWORKCONNECTION_DEBUG << "onConfigurationAdded: " << configration.name();


}

void NetworkConnection::onConfigurationChanged(QNetworkConfiguration configration){
    LOG_NETWORKCONNECTION_DEBUG << "onConfigurationChanged: " << configration.name();
}

void NetworkConnection::onconfigurationRemoved(QNetworkConfiguration configration){
      LOG_NETWORKCONNECTION_DEBUG << "onconfigurationRemoved: " << configration.name();
}

void NetworkConnection::setActiveNetwork(QString networkName){
    _networkName = networkName;
    QList<QNetworkConfiguration> activeConfigs = networkConfigurationManager->allConfigurations();
    foreach (QNetworkConfiguration config, activeConfigs) {
        if(config.name() == networkName){
           activeConfiguration =  config;
           LOG_NETWORKCONNECTION_DEBUG<< config.bearerTypeName() << config.name() << config.type();

        }
    }
}

void NetworkConnection::notifyConnectionStatus(int status,QString message){
    LOG_NETWORKCONNECTION_DEBUG << "NetworkConnection::notifyConnectionStatus" << status << message;
    emit publishConnectionStatus(status,message);
}

void NetworkConnection::readPendingDatagrams()
{
    //onUpdateCompleted();
    LOG_NETWORKCONNECTION_DEBUG << "readPendingDatagrams: udpSocket->localAddress()" << udpSocket->localAddress();
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);
        LOG_NETWORKCONNECTION_DEBUG << "sender: " << sender;
        LOG_NETWORKCONNECTION_DEBUG << "senderPort: " << senderPort;
        decodeDatagramm(datagram);
        if(httpresponse.value("NTS") == "ssdp:alive" &&
            httpresponse.value("NT") == "urn:schemas-sony-com:service:ScalarWebAPI:1"){
            downloadManager->get(QNetworkRequest(QUrl(httpresponse.value("LOCATION"))));
            LOG_NETWORKCONNECTION_DEBUG << "SSDP Client: Received alive from " << httpresponse.value("USN");
            emit publishConnectionStatus(_CONNECTIONSTATE_SSDP_ALIVE_RECEIVED);
        }
        else if(httpresponse.value("NTS") == "ssdp:byebye"){
            LOG_NETWORKCONNECTION_DEBUG << "SSDP Client: Received byebye from " << httpresponse.value("USN");
            emit publishConnectionStatus(_CONNECTIONSTATE_DISCONNECTED);
        }
        else {
            LOG_NETWORKCONNECTION_DEBUG << "SSDP Client: Received unknown subtype: " << httpresponse.value("NTS");
        }
    }
}

void NetworkConnection::decodeDatagramm(QByteArray datagramm){
    QString sdatagramm(datagramm);
    QString value;
    QString key;
    QStringList splitted = sdatagramm.split("\n");
    httpresponse.clear();
    //LOG_NETWORKCONNECTION_DEBUG << "splitted: " << splitted;
    foreach(QString splitter,splitted){
        splitter = splitter.trimmed();
        int keysize = splitter.indexOf(":");
        int size = splitter.size();
        if(keysize > 0){
            value = splitter.right(size-keysize-1);
            key = splitter.left(keysize);
            value = value.trimmed();
            //LOG_NETWORKCONNECTION_DEBUG << "KEY  : " << key;
            //LOG_NETWORKCONNECTION_DEBUG << "VALUE: " << value;
            httpresponse[key] = value;
        }
    }
}



void NetworkConnection::replyFinished(QNetworkReply *reply){
     QByteArray bts = reply->readAll();
     QString str(bts);
    //LOG_NETWORKCONNECTION_DEBUG << "reply: " << str;
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
            LOG_NETWORKCONNECTION_DEBUG << "isStartElement";
            LOG_NETWORKCONNECTION_DEBUG << "xml.name() " <<xml.name().toString();
            LOG_NETWORKCONNECTION_DEBUG << "xml.attributes() " <<xml.attributes().value(xml.name().toString()).toString();
            LOG_NETWORKCONNECTION_DEBUG << "xml.text() " <<xml.text().toString();
            LOG_NETWORKCONNECTION_DEBUG << "\n ";
            */
            if(xml.name().toString() == "friendlyName"){
                friendlyName = xml.readElementText();
                LOG_NETWORKCONNECTION_DEBUG << xml.name().toString() << "     " << friendlyName;
            }
            if(xml.name().toString() == "manufacturer"){
                manufacturer = xml.readElementText();
                LOG_NETWORKCONNECTION_DEBUG << xml.name().toString() << "     " << manufacturer;
            }
            if(xml.name().toString() == "manufacturerURL"){
                manufacturerURL = xml.readElementText();
                LOG_NETWORKCONNECTION_DEBUG << xml.name().toString() << "  " << manufacturerURL;
            }
            if(xml.name().toString() == "modelDescription"){
                modelDescription = xml.readElementText();
                LOG_NETWORKCONNECTION_DEBUG << xml.name().toString() << " " << modelDescription;
            }
            if(xml.name().toString() == "modelName"){
                modelName = xml.readElementText();
                LOG_NETWORKCONNECTION_DEBUG << xml.name().toString() << "        " << modelName;
            }
            if(xml.name().toString() == "UDN"){
                UDN = xml.readElementText();
                LOG_NETWORKCONNECTION_DEBUG << xml.name().toString() << "              " << UDN;
            }
            /*
            if(xml.name().toString() == "X_ScalarWebAPI_DeviceInfo"){
                LOG_NETWORKCONNECTION_DEBUG << "xml.name() " <<xml.name().toString();
                LOG_NETWORKCONNECTION_DEBUG << "xml.readElementText() " <<xml.readElementText();
            }

            if(xml.name().toString() == "X_ScalarWebAPI_Version"){
                LOG_NETWORKCONNECTION_DEBUG << "xml.name() " <<xml.name().toString();
                LOG_NETWORKCONNECTION_DEBUG << "xml.readElementText() " <<xml.readElementText();
            }

            if(xml.name().toString() == "X_ScalarWebAPI_ServiceList"){
                LOG_NETWORKCONNECTION_DEBUG << "xml.name() " <<xml.name().toString();
                LOG_NETWORKCONNECTION_DEBUG << "xml.readElementText() " <<xml.readElementText();
            }

            if(xml.name().toString() == "X_ScalarWebAPI_Service"){
                LOG_NETWORKCONNECTION_DEBUG << "xml.name() " <<xml.name().toString();
                LOG_NETWORKCONNECTION_DEBUG << "xml.readElementText() " <<xml.readElementText();
            }
            if(xml.name().toString() == "X_ScalarWebAPI_ServiceType"){
                LOG_NETWORKCONNECTION_DEBUG << "xml.name() " <<xml.name().toString();
                LOG_NETWORKCONNECTION_DEBUG << "xml.readElementText() " <<xml.readElementText();
            }
            */
            if(xml.name().toString() == "X_ScalarWebAPI_ActionList_URL"){
                linkString = xml.readElementText();
                LOG_NETWORKCONNECTION_DEBUG << xml.name().toString() << " " << linkString;
            }
            /*
            if(xml.name().toString() == "X_ScalarWebAPI_AccessType"){
                LOG_NETWORKCONNECTION_DEBUG << "xml.name() " <<xml.name().toString();
                LOG_NETWORKCONNECTION_DEBUG << "xml.readElementText() " <<xml.readElementText();
            }
            */
            //LOG_NETWORKCONNECTION_DEBUG << "\n ";
        }
    }
    if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
    }

}


void NetworkConnection::setUrl(QString urlstring){
    //LOG_NETWORKCONNECTION_DEBUG << "NetworkConnection::setUrl: " << urlstring;
    QUrl localUrl;
    if(!urlstring.isEmpty()){
        localUrl.setUrl(urlstring);
        url = localUrl;
        if(url.port() != -1){
            QString sport;
            sport.setNum(url.port());
        }
        //setPort(sport);
        //LOG_NETWORKCONNECTION_DEBUG << "NetworkConnection::setUrl url: " << url;
        emit publishUrl(urlstring);
    }
}

void NetworkConnection::setPort(QString portstring){
    LOG_NETWORKCONNECTION_DEBUG << "NetworkConnection::setPort: " << portstring;
    QUrl localUrl;
    if(!portstring.isEmpty()){
        url.setPort(portstring.toInt());
        emit publishPort(portstring);
    }
    //LOG_NETWORKCONNECTION_DEBUG << "port: " << portstring;
}


QStringList NetworkConnection::getAvailableNetWorks(){
    return _availableNetworks;
}

QNetworkConfiguration NetworkConnection::getActiveConfiguration(){
    LOG_NETWORKCONNECTION_DEBUG << "QNetworkConfiguration getActiveConfiguration" << activeConfiguration.name();
    return activeConfiguration;
}

QUrl NetworkConnection::getUrl(){
    //LOG_NETWORKCONNECTION_DEBUG << "NetworkConnection::getUrl() path: " << url.toString();
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
        LOG_NETWORKCONNECTION_DEBUG << "Brisa SSDP Client: Received alive from " <<
                datagram->value("usn") << "";

    } else if (datagram->value("nts") == "ssdp:byebye") {
        emit removedDeviceEvent(datagram->value("usn"));
        LOG_NETWORKCONNECTION_DEBUG << "Brisa SSDP Client: Received byebye from " <<
                datagram->value("usn") << "";

    } else {
        LOG_NETWORKCONNECTION_DEBUG << "Brisa SSDP Client: Received unknown subtype: " <<
                datagram->value("nts") << "";
    }
}
*/
