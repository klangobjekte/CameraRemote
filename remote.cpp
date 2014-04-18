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
    _networkConnection = networkConnection;
    offset = 0;
    start=0;
    end=0;
    _loadpreviewpic = true;

    url = networkConnection->getUrl();

    timer = new QTimer;
    timer->setInterval(80);
    connect(timer,SIGNAL(timeout()), this,SLOT(buildLiveViewPic()));

    getEventTimer = new QTimer;
    getEventTimer->setInterval(7000);
    getEventTimer->setSingleShot(true);
    connect(getEventTimer,SIGNAL(timeout()),
            this,SLOT(getEvent()));
    //connect(getEventTimer,SIGNAL(timeout()),
    //        this,SLOT(getAvailableApiList()));

    startRecordModeTimer = new QTimer;
    startRecordModeTimer->setInterval(2000);
    startRecordModeTimer->setSingleShot(true);
    connect(startRecordModeTimer,SIGNAL(timeout()),
            this,SLOT(startRecMode()));


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

    initActEnabelMethods();
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
    static bool once = true;
    static bool cameraReady = false;
    static bool connected = false;
    static int buildid = 20;
    QJsonDocument jdocument = QJsonDocument::fromJson(str.toUtf8());
    QJsonObject jobject = jdocument.object();
    QJsonValue jResult = jobject.value(QString("result"));
    QJsonValue jid = jobject.value(QString("id"));
    QJsonValue jError = jobject.value(QString("Error"));
    double id = jid.toDouble();
    qDebug() << "\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                "\nonManagerFinished response: "<< methods.key(id) << " "<< str << "Error: "<< reply->errorString();
    if(jobject.value(QString("Error")) == QString("Network unreachable")){
        initActEnabelMethods();
    }

    //qDebug()<<"\nQJsonValue of jResult: "<< jResult << jResult.type();
    //qDebug()<< "\nQJsonValue of jid: " << jid << jid.type();
    //qDebug()<< "\nQJsonValue of qstring: " << qstring;

    QJsonArray resultJarray = jResult.toArray();
    qDebug() << "\nQJsonArray resultJarray: " << resultJarray<< "size: " << resultJarray.size() << "\n";

    QVariantList results = resultJarray.toVariantList();
    //qDebug() << "\nQVariantList results: " << results;
    //qDebug() << "QJsonArray id: " << id;
    QStringList sresults;


    if(methods.value("getAvailableWhiteBalance") == id){
        for(int i = 0; i< jResult.toArray().size();i++){
            if(jResult.toArray().at(i).isArray()){
                QJsonArray array = jResult.toArray().at(i).toArray();
                qDebug() << "HAHA: "<<  jResult.toArray().at(i) << "size: " << array.size();
                QStringList wbCandidates;
                QStringList tmpCandidates;
                for(int y = 0; y<array.size();y++){
                    QJsonObject jobject3 = array[y].toObject();
                    //qDebug() << jobject3;
                    wbCandidates.append(jobject3.value("whiteBalanceMode").toString());
                }
                emit publishAvailableWhiteBalanceModes(wbCandidates);
            }
        }
    }

    for(int i =0;i<resultJarray.size();i++){
        QJsonObject jobject2 = resultJarray[i].toObject();
        //qDebug() << "jobject2" << jobject2.toVariantMap() << "i: " << i ;
        //qDebug() << "jobject2.keys()" << jobject2.keys();
        if(jobject2.keys().contains(QString("cameraStatus"))){
            if(jobject2.value(QString("cameraStatus")) == QString("NotReady")){
                qDebug() << "Camera NotReady";
                //getEvent("false");
                cameraReady = false;
            }
        }
        if(jobject2.keys().contains(QString("fNumberCandidates"))){
             QVariantList vlist = jobject2.value("fNumberCandidates").toArray().toVariantList();
             QStringList slist;
             foreach (QVariant var, vlist) {
                 slist.append(var.toString());
             }
             qDebug() << "fNumberCandidates: " << slist;
             emit publishAvailableFNumber(slist);
        }
        if(jobject2.keys().contains(QString("currentFNumber"))){
             qDebug() << "currentFNumber: " << jobject2.value("currentFNumber").toString();
             emit publishCurrentFNumber(jobject2.value("currentFNumber").toString());
        }
        if(jobject2.keys().contains(QString("shutterSpeedCandidates"))){

             QVariantList vlist = jobject2.value("shutterSpeedCandidates").toArray().toVariantList();
             QStringList slist;
             foreach (QVariant var, vlist) {
                 slist.append(var.toString());
             }
             qDebug() << "shutterSpeedCandidates: " << slist;
             emit publishAvailableShutterSpeed(slist);
        }
        if(jobject2.keys().contains(QString("currentShutterSpeed"))){
             qDebug() << "currentShutterSpeed: " << jobject2.value("currentShutterSpeed").toString();
             emit publishCurrentShutterSpeed(jobject2.value("currentShutterSpeed").toString());
        }

        if(jobject2.keys().contains(QString("isoSpeedRateCandidates"))){
             QVariantList vlist = jobject2.value("isoSpeedRateCandidates").toArray().toVariantList();
             QStringList slist;
             foreach (QVariant var, vlist) {
                 slist.append(var.toString());
             }
             qDebug() << "isoSpeedRateCandidates: " << slist;
             emit publishAvailableIsoSpeedRates(slist);
        }
        if(jobject2.keys().contains(QString("currentIsoSpeedRate"))){
             qDebug() << "currentIsoSpeedRate: " << jobject2.value("currentIsoSpeedRate").toString();
             emit publishCurrentIsoSpeedRates(jobject2.value("currentIsoSpeedRate").toString());
        }



        if(jobject2.keys().contains(QString("names"))){
             QVariantList vlist = jobject2.value("names").toArray().toVariantList();
             foreach (QVariant var, vlist) {
                 if(!methods.keys().contains(var.toString())){
                     methods[var.toString()] = buildid;
                     buildid++;
                 }
                 qDebug() << "names: " << var.toString() << "id: "<<  methods.value(var.toString());
             }
        }

        if(jobject2.keys().contains(QString("currentWhiteBalanceMode"))){
            getAvailableWhiteBalance(methods.value("getAvailableWhiteBalance"));
            qDebug() << "currentWhiteBalanceMode: " << jobject2.value("currentWhiteBalanceMode").toString();
            emit publishCurrentWhiteBalanceModes(jobject2.value("currentWhiteBalanceMode").toString());
        }

        if(jobject2.keys().contains(QString("whiteBalanceMode"))){
            for (QJsonObject::iterator it = jobject2.begin(); it != jobject2.end(); ++it){
                qDebug() << "whiteBalanceMode: " << it.value();
            }
            emit publishCurrentWhiteBalanceModes(jobject2.value("whiteBalanceMode").toString());
        }
    }







    foreach (QVariant result, results) {
        //qDebug() << "Variant: result: "  << result;
        if(result.isValid()){
            //qDebug() << "result.type: " << result.type();
            switch(result.type()){
            case QVariant::Double:
                //! Recognize startRecMode
                if(methods.value("startRecMode") == id){
                    //getAvailableApiList();
                    //initActEnabelMethods();
                    //startLiveview();
                    qDebug() <<" ++++++++++ startRecMode finished";
                    getEventTimer->start();
                    connected = true;
                }
                break;
            case QVariant::List:
                sresults = result.toStringList();
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
                }



                break;
            case QVariant::String:
                if(methods.value("startLiveview")  == id){

                        liveViewRequest = result.toString();
                        //liveViewRequest = "http://192.168.122.1:8080/liveview/liveviewstream.JPG?%211234%21http%2dget%3a%2a%3aimage%2fjpeg%3a%2a%21%21%21%21%21";
                        streamReply = liveViewManager->get(QNetworkRequest(QUrl(liveViewRequest)));
                        connect(streamReply, SIGNAL(readyRead()), this, SLOT(onLiveViewManagerReadyRead()));
                        //getAvailableApiList();
                        //getEventTimer->start();

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
                        //once = false;
                    }
                    if(id==2){
                        startRecMode();
                    }
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
    //while(true)
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

