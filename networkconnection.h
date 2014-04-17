#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H
#include <QObject>
#include <QUrl>
#include <QStringList>
#include <QNetworkConfiguration>

class QNetworkConfigurationManager;


class NetworkConnection : public QObject
{
     Q_OBJECT
public:
    explicit NetworkConnection();
    ~NetworkConnection();
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

private slots:

private:
    QNetworkConfigurationManager *networkConfigurationManager;
    QNetworkConfiguration activeConfiguration;
    QStringList availableNetworks;
    QString _networkName;
    QUrl url;

};

#endif // NETWORKCONNECTION_H
