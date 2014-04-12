#include "remote.h"
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
//#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QDebug>
#include <QTcpServer>
#include <QTimer>
#include <QFile>
#include <QMessageBox>




Remote::Remote(QObject *parent) :
    QObject(parent)
{
    offset = 0;
    start=0;
    end=0;
    //QUrl url("http://192.168.122.1/sony/camera");
    //url.setPort(8080
    url.setUrl("http://192.168.122.1/sony/camera");
    url.setPort(8080);
    _loadpreviewpic = true;
    QNetworkConfigurationManager mgr;
    QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations(QNetworkConfiguration::Active);
    QNetworkConfiguration useConfig;

    timer = new QTimer;
    timer->setInterval(80);

    connect(timer,SIGNAL(timeout()), this,SLOT(refreshLiveView()));

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

    manager = new QNetworkAccessManager;
    manager->setConfiguration(useConfig);
    qDebug() << "currentconnection: "<< manager->configuration().name();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    picmanager = new QNetworkAccessManager();
    picmanager->setConfiguration(useConfig);
    connect(picmanager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onPicmanagerFinished(QNetworkReply*)));

    liveViewManager = new QNetworkAccessManager;
    liveViewManager->setConfiguration(useConfig);

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
    timer->start();
    inputStream.clear();
    offset = 0;
    start=0;
    end=0;
}

void Remote::stopLiveView(){
    QByteArray jSonString = "{\"method\":\"stopLiveview\",\"params\":[],\"id\":7}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcRequest(postDataSize);
    manager->post(request,jSonString);
    timer->stop();
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
    //QUrl url("http://192.168.122.1/sony/camera");
    //url.setPort(8080);
    QNetworkRequest request(url);
    request.setRawHeader("","HTTP/1.1");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/xml");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postDataSize);
    //qDebug() << request.url().toString();
    //qDebug() << request.rawHeaderList();
    return request;
}

void Remote::setUrl(QString urlstring){
    qDebug() << "url: " << urlstring;
    if(!urlstring.isEmpty()){
        url.setUrl(urlstring);
        //emit publishUrl(urlstring);
    }
}

void Remote::setPort(QString portstring){
    if(!portstring.isEmpty()){
        url.setPort(portstring.toInt());
    }
    qDebug() << "port: " << portstring;

}


void Remote::replyFinished(QNetworkReply* reply){

    QByteArray bts = reply->readAll();
    QString str(bts);
    qDebug() << "replyFinished Url:" << reply->url()<< "reply String: "<< str << "reply Error String: "<< reply->errorString();

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
                qDebug() << "replyFinished result: " << sentry;
                if(!(sentry.contains("liveview"))&&_loadpreviewpic){
                    buildPreviewPicName(sentry);
                    picmanager->get(QNetworkRequest(QUrl(sentry)));
                }
                if(sentry.contains("liveview")){
                        liveViewRequest = sentry;
                        //liveViewRequest = "http://192.168.122.1:8080/liveview/liveviewstream.JPG?%211234%21http%2dget%3a%2a%3aimage%2fjpeg%3a%2a%21%21%21%21%21";
                        streamReply = liveViewManager->get(QNetworkRequest(QUrl(liveViewRequest)));
                        connect(streamReply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
                }
            }
        }
    }
}

void Remote::slotReadyRead(){
    //QByteArray bts = streamReply->readAll();
    inputStream.append(streamReply->readAll());

#ifdef __STORESTREAM
        QFile file("test.mjpeg");
        if (!file.open(QIODevice::WriteOnly)) {

            }
        else{
        QDataStream out(&file);

        out.writeRawData(inputStream,inputStream.size());
        //out << inputStream;
        file.close();
        }
#endif
}



void Remote::refreshLiveView(){
    //qDebug() << "\n++++++++++++++++++++++++++++++++++++++";
    //qDebug() << "refreshLiveView";
    //qDebug()<< "offset" << offset << "start: " << start << "end: " << end;
    QByteArray array;
    int jpegSize = 34048;
#ifdef  __USE_SATANDARD_HEADER
    int commonHeaderLength = 1 + 1 + 2 + 4;
    int payloadHeaderLength = 4 + 3 + 1 + 4 + 1 + 115;
    int paddingsize = 0;
    QByteArray commonHeader;
    QByteArray payloadHeader;

    for(int i=0;i<commonHeaderLength;i++)
        commonHeader[i] = inputStream[i];

    for(int i=0;i<payloadHeaderLength;i++)
        payloadHeader[i] = inputStream[i+commonHeaderLength];

    for(int i=0;i < jpegSize; i++){
        //array.append(inputStream.at(i));
        int y = i+commonHeaderLength+payloadHeaderLength;
        array[i] = inputStream[y];
    }
#endif

    bool found = false;
    if(!inputStream.isEmpty()){
        while(!found && offset<inputStream.length()-1 ){
            if(inputStream.at(offset) == -1  && inputStream.at(offset+1) == -40){
                start = offset;
                found = true;
            }
            offset++;
        }
        found = false;
        while(!found && offset<inputStream.length()-1){
            if(inputStream.at(offset) == -1 && inputStream.at(offset+1) == -39){
                end = offset;
                found = true;
            }
            offset++;
        }
        if(found){
            jpegSize = end-start;
            for(int i=0,y=start;i < jpegSize; i++,y++){
                array[i] = inputStream.at(y);
            }
            emit publishLiveViewBytes(array);
            if(offset > (jpegSize*16)){
                inputStream.clear();
                offset = 0;
                start = 0;
                end = 0;
                //qDebug() << "Buffer Cleared";
            }
        }
    }
}


void Remote::onPicmanagerFinished(QNetworkReply *reply){
    if(reply->bytesAvailable())
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
    qDebug() << "buildPreviewPicName: " <<pos << previePicName;
}



