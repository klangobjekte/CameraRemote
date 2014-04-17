#include "networkconnection.h"
#include <QNetworkConfigurationManager>
#include <QUrl>
#include <QDebug>


NetworkConnection::NetworkConnection()
{

    //QNetworkConfigurationManager networkConfigurationManager;
    networkConfigurationManager = new QNetworkConfigurationManager;
    QList<QNetworkConfiguration> activeConfigs = networkConfigurationManager->allConfigurations(QNetworkConfiguration::Active);


    if (activeConfigs.count() > 0){
        Q_ASSERT(networkConfigurationManager->isOnline());
        foreach (QNetworkConfiguration config, activeConfigs) {
            qDebug()<< config.bearerTypeName() << config.name();
            availableNetworks.append(config.name());
            if(config.name() == "en1")
               activeConfiguration =  config;
        }

    }
    else
    {
        Q_ASSERT(!networkConfigurationManager->isOnline());
    }

}

NetworkConnection::~NetworkConnection(){
    delete networkConfigurationManager;
}

void NetworkConnection::setActiveNetwork(QString networkName){
    _networkName = networkName;
}

void NetworkConnection::setUrl(QString urlstring){
    //qDebug() << "url: " << urlstring;
    if(!urlstring.isEmpty()){
        url.setUrl(urlstring);
        emit publishUrl(urlstring);

    }
}

void NetworkConnection::setPort(QString portstring){
    if(!portstring.isEmpty()){
        url.setPort(portstring.toInt());
        emit publishPort(portstring);
    }
    //qDebug() << "port: " << portstring;

}


QStringList NetworkConnection::getAvailableNetWorks(){
    return availableNetworks;
}

QNetworkConfiguration NetworkConnection::getActiveConfiguration(){
    return activeConfiguration;
}

QUrl NetworkConnection::getUrl(){
    return url;

}
