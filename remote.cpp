#include "remote.h"
//#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QDebug>

#include <QTimer>
#include <QFile>
#include <QSslKey>
//#include <QMessageAuthenticationCode>
#include <QCryptographicHash>
#include "networkconnection.h"




Remote::Remote(NetworkConnection *networkConnection,QObject *parent) :
    QObject(parent)
{
    offset = 0;
    start=0;
    end=0;
    _loadpreviewpic = true;

    url = networkConnection->getUrl();

    timer = new QTimer;
    timer->setInterval(80);
    connect(timer,SIGNAL(timeout()), this,SLOT(buildLiveViewPic()));

    manager = new QNetworkAccessManager;
    manager->setConfiguration(networkConnection->getActiveConfiguration());
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onManagerFinished(QNetworkReply*)));

    picmanager = new QNetworkAccessManager();
    picmanager->setConfiguration(networkConnection->getActiveConfiguration());
    connect(picmanager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onPicmanagerFinished(QNetworkReply*)));

    liveViewManager = new QNetworkAccessManager;
    liveViewManager->setConfiguration(networkConnection->getActiveConfiguration());
    startRecMode();

    //! init the static methoods - map
    methods["actEnableMethods"] = 1;
    methods["getVersions"] = 5;
    methods["getMethodTypes"] = 6;
    methods["getApplicationInfo"] = 7;
    methods["getAvailableApiList"] = 8;
    methods["getEvent"] = 9;
    methods["actTakePicture"] = 10;
    methods["startRecMode"] = 11;
    methods["stopRecMode"] = 12;
    methods["startLiveview"] = 13;
    methods["stopLiveview"] = 14;
    methods["startLiveviewWithSize"] = 15;
    methods["startMovieRec"] = 16;
    methods["stopMovieRec"] = 17;


}

Remote::~Remote(){
    stopLiveview();
    stopRecMode();
    delete timer;
    delete liveViewManager;
    delete picmanager;
    delete manager;
}

QMap<QString,int> Remote::getMethods(){
    return methods;
}

void Remote::initialCommands(){
    //getMethodTypes();
    //getApplicationInfo();
    getAvailableApiList();
    getEvent("true");
    //startRecMode();
    //startLiveView();
    //getAvailableWhiteBalance(methods.value("getAvailableWhiteBalance"));
    //getAvailableIsoSpeedRate(methods.value("getAvailableIsoSpeedRate"));
    //getAvailablePostviewImageSize();
    //getSupportedShootMode();
    //getShootMode();
    //getAvailableShootMode();
    //getFNumber();
}


void Remote::initActEnabelMethods(){
    QByteArray jSonString = "{\"version\": \"1.0\", \"params\": [{\"developerName\": \"\", \"sg\": \"\", \"methods\": \"\", \"developerID\": \"\"}], \"method\": \"actEnableMethods\", \"id\": 1}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = constructAccessControlRequest(postDataSize);
    manager->post(request,jSonString);

}

void Remote::actEnabelMethods(QByteArray key){
    QByteArray jSonString2 = "{\"version\": \"1.0\", \"params\": [{\"developerName\": \"Sony Corporation\", ";
    jSonString2.append("\"sg\": \"");
    jSonString2.append(key);
    jSonString2.append("\", ");
    jSonString2.append("\"methods\": \"camera/setFlashMode:camera/getFlashMode:camera/getSupportedFlashMode:camera/getAvailableFlashMode:camera/setExposureCompensation:camera/getExposureCompensation:camera/getSupportedExposureCompensation:camera/getAvailableExposureCompensation:camera/setSteadyMode:camera/getSteadyMode:camera/getSupportedSteadyMode:camera/getAvailableSteadyMode:camera/setViewAngle:camera/getViewAngle:camera/getSupportedViewAngle:camera/getAvailableViewAngle:camera/setMovieQuality:camera/getMovieQuality:camera/getSupportedMovieQuality:camera/getAvailableMovieQuality:camera/setFocusMode:camera/getFocusMode:camera/getSupportedFocusMode:camera/getAvailableFocusMode:camera/setStillSize:camera/getStillSize:camera/getSupportedStillSize:camera/getAvailableStillSize:camera/setBeepMode:camera/getBeepMode:camera/getSupportedBeepMode:camera/getAvailableBeepMode:camera/setCameraFunction:camera/getCameraFunction:camera/getSupportedCameraFunction:camera/getAvailableCameraFunction:camera/setLiveviewSize:camera/getLiveviewSize:camera/getSupportedLiveviewSize:camera/getAvailableLiveviewSize:camera/setTouchAFPosition:camera/getTouchAFPosition:camera/cancelTouchAFPosition:camera/setFNumber:camera/getFNumber:camera/getSupportedFNumber:camera/getAvailableFNumber:camera/setShutterSpeed:camera/getShutterSpeed:camera/getSupportedShutterSpeed:camera/getAvailableShutterSpeed:camera/setIsoSpeedRate:camera/getIsoSpeedRate:camera/getSupportedIsoSpeedRate:camera/getAvailableIsoSpeedRate:camera/setExposureMode:camera/getExposureMode:camera/getSupportedExposureMode:camera/getAvailableExposureMode:camera/setWhiteBalance:camera/getWhiteBalance:camera/getSupportedWhiteBalance:camera/getAvailableWhiteBalance:camera/setProgramShift:camera/getSupportedProgramShift:camera/getStorageInformation:camera/startLiveviewWithSize:camera/startIntervalStillRec:camera/stopIntervalStillRec:camera/actFormatStorage:system/setCurrentTime\", \"developerID\": \"7DED695E-75AC-4ea9-8A85-E5F8CA0AF2F3\"}], \"method\": \"actEnableMethods\", \"id\": 2}");
    //qDebug() << "\njSonString2: "<< jSonString2 << "\n";
    QByteArray postDataSize2 = QByteArray::number(jSonString2.size());
    QNetworkRequest request2 = constructAccessControlRequest(postDataSize2);
    manager->post(request2,jSonString2);
}


void Remote::commandFabrikMethod(QByteArray command, int id, QByteArray params){

    //QByteArray jSonString = "{\"method\":\"getMethodTypes\",\"params\":[],\"id\":3}";
    QByteArray jSonString = "{\"method\":\"";
    jSonString.append(command);
    jSonString.append("\",\"params\":[");
    jSonString.append(params);
    jSonString.append("],\"id\":");
    QByteArray idNum;
    jSonString.append(idNum.setNum(id));
    jSonString.append("}");
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcCameraRequest(postDataSize);
    manager->post(request,jSonString);
}

QNetworkRequest Remote::constructAccessControlRequest(QByteArray postDataSize){
    QString constructedurl = url.toString();
    constructedurl.append("/sony/accessControl");
    QUrl localurl(constructedurl);
    localurl.setPort(url.port());
    QNetworkRequest request(localurl);
    request.setRawHeader("","HTTP/1.1");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/xml");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postDataSize);
    //qDebug() << request.url().toString();
    //qDebug() << request.rawHeaderList();
    return request;
}

QNetworkRequest Remote::construcCameraRequest(QByteArray postDataSize){
    QString constructedurl = url.toString();
    constructedurl.append("/sony/camera");
    QUrl localurl(constructedurl);
    localurl.setPort(url.port());
    QNetworkRequest request(localurl);
    request.setRawHeader("","HTTP/1.1");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/xml");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postDataSize);
    //qDebug() << request.url().toString();
    //qDebug() << request.rawHeaderList();
    return request;
}



void Remote::onManagerFinished(QNetworkReply* reply){
    QByteArray bts = reply->readAll();
    QString str(bts);
    qDebug() << "onManagerFinished:" << reply->url()<< "response: "<< str << "Error: "<< reply->errorString();
    static bool once = true;
    QJsonDocument jdocument = QJsonDocument::fromJson(str.toUtf8());
    QJsonObject jobject = jdocument.object();
    QJsonValue jResult = jobject.value(QString("result"));
    QJsonValue jid = jobject.value(QString("id"));
    //qDebug()<< "document.isEmpty(): "<< endl << jdocument.isEmpty() << "jobject.isEmpty(): " << jobject.isEmpty();
    //qDebug()<<"QJsonValue of "result": " << jResult << "type: " << jResult.type();
    //qDebug()<< "QJsonValue of id: " << jid << "type: " << jid.type();
    QJsonArray resultJarray = jResult.toArray();
    double id = jid.toDouble();
    QVariantList results = resultJarray.toVariantList();
    //qDebug() << "QJsonArray results: " << results;
    //qDebug() << "QJsonArray id: " << id;
    QStringList sresults;
    foreach (QVariant result, results) {
        //qDebug() << "Variant: result: "  << result;
        if(result.isValid()){
            //qDebug() << "result.type: " << result.type();
            switch(result.type()){
            case QVariant::Double:
                //! Recognize startRecMode
                if(methods.value("startRecMode") == id)
                    startLiveview();
                break;
            case QVariant::List:
                sresults = result.toStringList();
                static int buildid = 20;
                if(methods.value("actTakePicture") == id &&_loadpreviewpic){
                    foreach (QString sresult, sresults) {
                        buildPreviewPicName(sresult);
                        picmanager->get(QNetworkRequest(QUrl(sresult)));
                    }
                }
                //qDebug() << "onManagerFinished result: " << sresult;
                if(methods.value("getAvailableApiList")==id){
                    foreach (QString sresult, sresults){
                        if(!methods.keys().contains(sresult)){
                            methods[sresult] = buildid;
                            buildid++;
                        }
                        qDebug() << "result: " << sresult << "id: "<<  methods.value(sresult);
                    }
                    getAvailableWhiteBalance(methods.value("getAvailableWhiteBalance"));
                    getAvailableIsoSpeedRate(methods.value("getAvailableIsoSpeedRate"));
                    getAvailableFNumber(methods.value("getAvailableFNumber"));
                    getAvailableShutterSpeed(methods.value("getAvailableShutterSpeed"));
                }
                if(methods.value("getAvailableFNumber")==id){
                    emit publishAvailableFNumber(result.toStringList());
                    //qDebug() << this->parent()->children();
                }
                if(methods.value("getAvailableIsoSpeedRate")==id){

                    emit publishAvailableIsoSpeedRates(result.toStringList());
                }
                if(methods.value("getAvailableShutterSpeed")==id){
                    emit publishAvailableShutterSpeed(result.toStringList());
                }

                if(methods.value("getAvailableWhiteBalance")==id){
                    emit publishAvailableWhiteBalanceModes(result.toStringList());
                }


                break;
            case QVariant::String:
                if(methods.value("startLiveview")  == id){
                        liveViewRequest = result.toString();
                        //liveViewRequest = "http://192.168.122.1:8080/liveview/liveviewstream.JPG?%211234%21http%2dget%3a%2a%3aimage%2fjpeg%3a%2a%21%21%21%21%21";
                        streamReply = liveViewManager->get(QNetworkRequest(QUrl(liveViewRequest)));
                        connect(streamReply, SIGNAL(readyRead()), this, SLOT(onLiveViewManagerReadyRead()));
                        initActEnabelMethods();
                }
                if(methods.value("getAvailableFNumber")==id){
                    emit publishCurrentFNumber(result.toString());
                }

                if(methods.value("getAvailableIsoSpeedRate")==id){
                    emit publishCurrentIsoSpeedRates(result.toString());
                }

                if(methods.value("getAvailableShutterSpeed")==id){
                    emit publishCurrentShutterSpeed(result.toString());
                }

                if(methods.value("getAvailableWhiteBalance")==id){
                    emit publishCurrentWhiteBalanceModes(result.toString());
                }


                break;
            case QVariant::Map:
                if(result.toMap().keys().contains("dg")){
                    QByteArray KEYDG="90adc8515a40558968fe8318b5b023fdd48d3828a2dda8905f3b93a3cd8e58dc";
                    KEYDG.append(result.toMap().value("dg").toByteArray());
                    //qDebug()<< "onManagerFinished result: " << result.toMap().value("dg").toString();
                    qDebug()<< "onManagerFinished result: " << KEYDG;
                    QByteArray encoded = QCryptographicHash::hash(KEYDG, QCryptographicHash::Sha256);
                    if(id==1){
                        actEnabelMethods(encoded.toBase64());
                        once = false;
                    }
                    //if(KEYDG == "90adc8515a40558968fe8318b5b023fdd48d3828a2dda8905f3b93a3cd8e58dc")
                      if(id==2)
                        initialCommands();
                }
                break;
            }
        }
    }
}

void Remote::onLiveViewManagerReadyRead(){
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

void Remote::buildLiveViewPic(){
    //qDebug() << "\n++++++++++++++++++++++++++++++++++++++";
    //qDebug() << "buildLiveViewPic";
    //qDebug()<< "offset" << offset << "start: " << start << "end: " << end;
    QByteArray array;
    int jpegSize = 34048;
#ifdef  __USE_STANDARD_HEADER
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
    if(pos >= 0){
        tmppicname = url.right(url.size() - pos);
        if(tmppicname == "/liveviewstream")
            tmppicname.append(".mjpeg");
    }
    previePicName = tmppicname;
    if(pos >= 0)
        qDebug() << "previePicName: " << previePicName;
}


/////////////////////////////////////////////////////////////////

void Remote::getVersions(int id){
    commandFabrikMethod("getVersions",id);
}

void Remote::getMethodTypes(int id){
    commandFabrikMethod("getMethodTypes",id);
}

void Remote::getApplicationInfo(int id){
    commandFabrikMethod("getApplicationInfo",id);
}

void Remote::getAvailableApiList(int id){
    commandFabrikMethod("getAvailableApiList",id);
}

void Remote::getEvent(QByteArray param,int id){
    commandFabrikMethod("getEvent",id,param);
}

void Remote::actTakePicture(int id){
    commandFabrikMethod("actTakePicture",id);
}

void Remote::startRecMode(int id){
    commandFabrikMethod("startRecMode",id);
}

void Remote::stopRecMode(int id){
    commandFabrikMethod("stopRecMode",id);
}

void Remote::startLiveview(int id){
    commandFabrikMethod("startLiveview",id);
    timer->start();
    inputStream.clear();
    offset = 0;
    start=0;
    end=0;
}

void Remote::stopLiveview(int id){
    commandFabrikMethod("stopLiveview",id);
    timer->stop();
}

void Remote::startLiveviewWithSize(int id){
    commandFabrikMethod("startLiveviewWithSize",id);
}

void Remote::actZoom(int id){
    commandFabrikMethod("actZoom",id);
}

void Remote::awaitTakePicture(int id){
    commandFabrikMethod("awaitTakePicture",id);
}

void Remote::setSelfTimer(int id){
    commandFabrikMethod("setSelfTimer",id);
}

void Remote::getSelfTimer(int id){
    commandFabrikMethod("getSelfTimer",id);
}

void Remote::getAvailableSelfTimer(int id){
    commandFabrikMethod("getAvailableSelfTimer",id);
}

void Remote::getSupportedSelfTimer(int id){
    commandFabrikMethod("getSupportedSelfTimer",id);
}

void Remote::getExposureMode(int id){
    commandFabrikMethod("getExposureMode",id);
}

void Remote::getSupportedExposureMode(int id){
    commandFabrikMethod("getSupportedExposureMode",id);
}

void Remote::getExposureCompensation(int id){
    commandFabrikMethod("getExposureCompensation",id);
}

void Remote::getAvailableExposureCompensation(int id){
    commandFabrikMethod("getAvailableExposureCompensation",id);
}

void Remote::getSupportedExposureCompensation(int id){
    commandFabrikMethod("getSupportedExposureCompensation",id);
}

void Remote::setFNumber(int id){
    commandFabrikMethod("setFNumber",id);
}

void Remote::getFNumber(int id){
    commandFabrikMethod("getFNumber",id);
}

void Remote::getAvailableFNumber(int id){
    commandFabrikMethod("getAvailableFNumber",id);
}

void Remote::getSupportedFNumber(int id){
    commandFabrikMethod("getSupportedFNumber",id);
}

void Remote::setIsoSpeedRate(int id){
    commandFabrikMethod("setIsoSpeedRate",id);
}

void Remote::getIsoSpeedRate(int id){
    commandFabrikMethod("getIsoSpeedRate",id);
}

void Remote::getAvailableIsoSpeedRate(int id){
    commandFabrikMethod("getAvailableIsoSpeedRate",id);
}

void Remote::getSupportedIsoSpeedRate(int id){
    commandFabrikMethod("getSupportedIsoSpeedRate",id);
}

void Remote::getLiveviewSize(int id){
    commandFabrikMethod("getLiveviewSize",id);
}

void Remote::getAvailableLiveviewSize(int id){
    commandFabrikMethod("getAvailableLiveviewSize",id);
}

void Remote::getSupportedLiveviewSize(int id){
    commandFabrikMethod("getSupportedLiveviewSize",id);
}

void Remote::setPostviewImageSize(int id){
    commandFabrikMethod("setPostviewImageSize",id);
}

void Remote::getPostviewImageSize(int id){
    commandFabrikMethod("getPostviewImageSize",id);
}


void Remote::getAvailablePostviewImageSize(int id){
    commandFabrikMethod("getAvailablePostviewImageSize",id);
}

void Remote::getSupportedPostviewImageSize(int id){
    commandFabrikMethod("getSupportedPostviewImageSize",id);
}

void Remote::getSupportedProgramShift(int id){
    commandFabrikMethod("getSupportedProgramShift",id);
}


void Remote::setShootMode(int id){
    commandFabrikMethod("setShootMode",id);
}

void Remote::getShootMode(int id){
    commandFabrikMethod("getShootMode",id);
}

void Remote::getAvailableShootMode(int id){
    commandFabrikMethod("getAvailableShootMode",id);
}

void Remote::getSupportedShootMode(int id){
    commandFabrikMethod("getSupportedShootMode",id);
}

void Remote::setShutterSpeed(int id){
    commandFabrikMethod("setShutterSpeed",id);
}

void Remote::getShutterSpeed(int id){
    commandFabrikMethod("getShutterSpeed",id);
}

void Remote::getAvailableShutterSpeed(int id){
    commandFabrikMethod("getAvailableShutterSpeed",id);
}

void Remote::getSupportedShutterSpeed(int id){
    commandFabrikMethod("getSupportedShutterSpeed",id);
}

void Remote::setTouchPosition(int id){
    commandFabrikMethod("setTouchPosition",id);
}

void Remote::getTouchPosition(int id){
    commandFabrikMethod("getTouchPosition",id);
}

void Remote::getWhiteBalance(int id){
    commandFabrikMethod("getWhiteBalance",id);
}

void Remote::getSupportedWhiteBalance(int id){
    commandFabrikMethod("getSupportedWhiteBalance",id);
}

void Remote::getAvailableWhiteBalance(int id){
    commandFabrikMethod("getAvailableWhiteBalance",id);
}

void Remote::getSupportedFlashMode(int id){
    commandFabrikMethod("getSupportedFlashMode",id);
}

void Remote::startMovieRec(int id){
    commandFabrikMethod("startMovieRec",id);
}

void Remote::stopMovieRec(int id){
    commandFabrikMethod("stopMovieRec",id);
}

