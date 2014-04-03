#include "remote.h"
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QDebug>





Remote::Remote(QObject *parent) :
    QObject(parent)
{
    _loadpreviewpic = true;
    QNetworkConfigurationManager mgr;
    QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations(QNetworkConfiguration::Active);
    QNetworkConfiguration useConfig;

    if (activeConfigs.count() > 0){
        Q_ASSERT(mgr.isOnline());
        foreach (QNetworkConfiguration config, activeConfigs) {
            qDebug()<< config.bearerTypeName() << config.name();
            if(config.name() == "en1")
               useConfig =  config;
        }
    }
    else
    {
        Q_ASSERT(!mgr.isOnline());
    }
    picmanager = new QNetworkAccessManager();
    picmanager->setConfiguration(useConfig);
    connect(picmanager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(loadPreview(QNetworkReply*)));

    manager = new QNetworkAccessManager;
    manager->setConfiguration(useConfig);
    qDebug() << "currentconnection: "<< manager->configuration().name();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    getApplicationInfo();
    getAvailableApiList();
    getMethodTypes();
    getShootMode();
    getAvailableShootMode();
    getSupportedShootMode();

}

void Remote::getApplicationInfo(){
    QByteArray jSonString = "{\"method\":\"getApplicationInfo\",\"params\":[],\"id\":0}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);

}

void Remote::getAvailableApiList(){
    QByteArray jSonString = "{\"method\":\"getAvailableApiList\",\"params\":[],\"id\":1}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}



void Remote::getMethodTypes(){
    QByteArray jSonString = "{\"method\":\"getMethodTypes\",\"params\":[],\"id\":2}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}



void Remote::startRecMode(){
    QByteArray jSonString = "{\"method\":\"startRecMode\",\"params\":[],\"id\":3}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::stopRecMode(){
    QByteArray jSonString = "{\"method\":\"stopRecMode\",\"params\":[],\"id\":4}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::actTakePicture(){
    QByteArray jSonString = "{\"method\":\"actTakePicture\",\"params\":[],\"id\":5}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::startLiveView(){
    QByteArray jSonString = "{\"method\":\"startLiveview\",\"params\":[],\"id\":6}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::stopLiveView(){
    QByteArray jSonString = "{\"method\":\"stopLiveview\",\"params\":[],\"id\":7}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}


void Remote::getShootMode(){
    QByteArray jSonString = "{\"method\":\"getShootMode\",\"params\":[],\"id\":8}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::setShootMode(){
    QByteArray jSonString = "{\"method\":\"setShootMode\",\"params\":[],\"id\":9}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::getAvailableShootMode(){
    QByteArray jSonString = "{\"method\":\"getAvailableShootMode\",\"params\":[],\"id\":10}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::getSupportedShootMode(){
    QByteArray jSonString = "{\"method\":\"getSupportedShootMode\",\"params\":[],\"id\":11}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::startMovieRec(){
    QByteArray jSonString = "{\"method\":\"startMovieRec\",\"params\":[],\"id\":12}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::stopMovieRec(){
    QByteArray jSonString = "{\"method\":\"stopMovieRec\",\"params\":[],\"id\":13}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::actZoom(){
    QByteArray jSonString = "{\"method\":\"actZoom\",\"params\":[],\"id\":14}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}

void Remote::getEvent(){
    QByteArray jSonString = "{\"method\":\"getEvent\",\"params\":[],\"id\":15}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
}


QNetworkRequest Remote::construcRequest(QByteArray postDataSize){
    QUrl url("http://192.168.122.1/sony/camera");
    url.setPort(8080);
    QNetworkRequest request(url);
    request.setRawHeader("","HTTP/1.1");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/xml");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postDataSize);
    //qDebug() << request.url().toString();
    //qDebug() << request.rawHeaderList();
    return request;
}

void Remote::replyFinished(QNetworkReply* reply){
    QByteArray bts = reply->readAll();
    QString str(bts);
    qDebug() << reply->url()<< str << reply->errorString();

    QJsonDocument jdocument = QJsonDocument::fromJson(str.toUtf8());
    QJsonObject jobject = jdocument.object();
    QJsonValue value = jobject.value(QString("result"));
    //qDebug()<< "document.isEmpty(): "<< endl << jdocument.isEmpty() << "jobject.isEmpty(): " << jobject.isEmpty();
    //qDebug()<<"QJsonValue of \"result\": " << value << "type: " << value.type();
    QJsonArray jarray = value.toArray();
    QVariantList variantlist = jarray.toVariantList();
    //qDebug() << "QJsonArray variantlist: " << variantlist;
    QStringList stringlist;
    foreach (QVariant entry, variantlist) {
        if(entry.isValid()){
            stringlist = entry.toStringList();
            foreach (QString sentry, stringlist) {
                qDebug() << "result: " << sentry;
                if(_loadpreviewpic){
                    buildPreviewPicName(sentry);
                    picmanager->get(QNetworkRequest(QUrl(sentry)));
                }
            }
        }
    }
}

void Remote::loadPreview(QNetworkReply *reply){
    emit publishLoadPreview(reply,previePicName);
}

void Remote::setLoadPreviewPic(bool loadpreviewpic){
    _loadpreviewpic = loadpreviewpic;
}

void Remote::buildPreviewPicName(QString url){
    QString tmppicname;
    int pos = url.lastIndexOf("/");
    tmppicname = url.right(url.size() - pos);
    if(tmppicname == "/liveviewstream")
        tmppicname.append(".mjpeg");
    previePicName = tmppicname;
    qDebug() << "pos: " <<pos << previePicName;
}
