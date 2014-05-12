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
//#include <QSslKey>

#include <QCryptographicHash>
#include "networkconnection.h"

//#define LOG_RESULT
#ifdef LOG_RESULT
#   define LOG_RESULT_DEBUG qDebug()
#else
#   define LOG_RESULT_DEBUG nullDebug()
#endif

//#define LOG_RESPONSE
#ifdef LOG_RESPONSE
#   define LOG_RESPONSE_DEBUG qDebug()
#else
#   define LOG_RESPONSE_DEBUG nullDebug()
#endif


//#define LOG_REQUEST
#ifdef LOG_REQUEST
#   define LOG_REQUEST_DEBUG qDebug()
#else
#   define LOG_REQUEST_DEBUG nullDebug()
#endif



Remote::Remote(NetworkConnection *networkConnection,QObject *parent) :
    QObject(parent)
{

    connected = false;
    connecting = false;
    manualLiveViewStart = false;
    liveViewStreamAlive = false;
    _networkConnection = networkConnection;
    offset = 0;
    start=0;
    end=0;
    _loadpreviewpic = true;


    qDebug() << "networkConnection->getUrl(): " << networkConnection->getUrl();

    timer = new QTimer;
    timer->setInterval(80);
    connect(timer,SIGNAL(timeout()), this,SLOT(buildLiveViewPic()));

    getEventTimerPeriodic = new QTimer;
    getEventTimerPeriodic->setInterval(5000);
    connect(getEventTimerPeriodic,SIGNAL(timeout()),
            this,SLOT(getEvent()));

    getEventTimerSingleshot = new QTimer;
    getEventTimerSingleshot->setInterval(7000);
    getEventTimerSingleshot->setSingleShot(true);
    connect(getEventTimerSingleshot,SIGNAL(timeout()),
            this,SLOT(getEvent()));



    initialEventTimer = new QTimer;
    initialEventTimer->setInterval(1000);
    connect(initialEventTimer,SIGNAL(timeout()),
            this,SLOT(initialEvent()));
    //connect(initialEventTimer,SIGNAL(timeout()),
    //        this,SLOT(initActEnabelMethods()));


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
    methods["setCameraFunction"] = 18;
}

Remote::~Remote(){
    stopLiveview();
    stopRecMode();
    delete timer;
    delete getEventTimerPeriodic;
    delete getEventTimerSingleshot;
    delete liveViewManager;
    delete picmanager;
    delete manager;
}

void Remote::setActiveNetworkConnection(){
    manager->setConfiguration(_networkConnection->getActiveConfiguration());
    picmanager->setConfiguration(_networkConnection->getActiveConfiguration());
    liveViewManager->setConfiguration(_networkConnection->getActiveConfiguration());
}

void Remote::initialEvent(){
    getEvent("false",66);
    //initialEventTimer->start();
}

QMap<QString,int> Remote::getMethods(){
    return methods;
}


void Remote::startDevice(){
    qDebug() << "start Device ++++++++++++++++++++++++++++++++++" << connected;
    if(!connected){
        initActEnabelMethods();
    }
}

void Remote::setDevice(QString device){
    deviceFriendlyName = device;
}


void Remote::setConnectionStatus(int status,QString message){
    Q_UNUSED(message);
    //qDebug() << "Remote::setConnectionStatus: " << status;
    switch(status){
    case _CONNECTIONSTATE_CONNECTET:
        break;
    case _CONNECTIONSTATE_CAMERA_DETECTED:
        if(!connected && !connecting){
            connecting = true;
            deviceFriendlyName = message;
            qDebug() << "deviceFriendlyName: " << deviceFriendlyName;
            //startDevice();
            initialEvent();

        }
        break;
    case _CONNECTIONSTATE_CONNECTING:
        break;
    case _CONNECTIONSTATE_DISCONNECTED:
        break;
    case _CONNECTIONSTATE_ERROR:
        break;
    case _CONNECTIONSTATE_SSDP_ALIVE_RECEIVED:
        break;
    case _CONNECTIONSTATE_SSDP_BYEBYE_RECEIVED:
        break;
    case _CONNECTIONSTATE_WATING:
        break;
    }

}

bool Remote::getConnectionStatus(){
    return connected;
}


void Remote::initActEnabelMethods(){

    QByteArray jSonString = "{\"version\": \"1.0\", \"params\": [{\"developerName\": \"\", \"sg\": \"\", \"methods\": \"\", \"developerID\": \"\"}], \"method\": \"actEnableMethods\", \"id\": 1}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = constructAccessControlRequest(postDataSize);
    LOG_REQUEST_DEBUG << "initActEnabelMethods request: " << jSonString << "\n";

    manager->post(request,jSonString);

}

void Remote::actEnabelMethods(QByteArray key){
    QByteArray jSonString2 = "{\"version\": \"1.0\", \"params\": [{\"developerName\": \"Sony Corporation\", ";
    jSonString2.append("\"sg\": \"");
    jSonString2.append(key);
    jSonString2.append("\", ");
    jSonString2.append("\"methods\": \"camera/setFlashMode:camera/getFlashMode:camera/getSupportedFlashMode:camera/getAvailableFlashMode:camera/setExposureCompensation:camera/getExposureCompensation:camera/getSupportedExposureCompensation:camera/getAvailableExposureCompensation:camera/setSteadyMode:camera/getSteadyMode:camera/getSupportedSteadyMode:camera/getAvailableSteadyMode:camera/setViewAngle:camera/getViewAngle:camera/getSupportedViewAngle:camera/getAvailableViewAngle:camera/setMovieQuality:camera/getMovieQuality:camera/getSupportedMovieQuality:camera/getAvailableMovieQuality:camera/setFocusMode:camera/getFocusMode:camera/getSupportedFocusMode:camera/getAvailableFocusMode:camera/setStillSize:camera/getStillSize:camera/getSupportedStillSize:camera/getAvailableStillSize:camera/setBeepMode:camera/getBeepMode:camera/getSupportedBeepMode:camera/getAvailableBeepMode:camera/setCameraFunction:camera/getCameraFunction:camera/getSupportedCameraFunction:camera/getAvailableCameraFunction:camera/setLiveviewSize:camera/getLiveviewSize:camera/getSupportedLiveviewSize:camera/getAvailableLiveviewSize:camera/setTouchAFPosition:camera/getTouchAFPosition:camera/cancelTouchAFPosition:camera/setFNumber:camera/getFNumber:camera/getSupportedFNumber:camera/getAvailableFNumber:camera/setShutterSpeed:camera/getShutterSpeed:camera/getSupportedShutterSpeed:camera/getAvailableShutterSpeed:camera/setIsoSpeedRate:camera/getIsoSpeedRate:camera/getSupportedIsoSpeedRate:camera/getAvailableIsoSpeedRate:camera/setExposureMode:camera/getExposureMode:camera/getSupportedExposureMode:camera/getAvailableExposureMode:camera/setWhiteBalance:camera/getWhiteBalance:camera/getSupportedWhiteBalance:camera/getAvailableWhiteBalance:camera/setProgramShift:camera/getSupportedProgramShift:camera/getStorageInformation:camera/startLiveviewWithSize:camera/startIntervalStillRec:camera/stopIntervalStillRec:camera/actFormatStorage:system/setCurrentTime\", \"developerID\": \"7DED695E-75AC-4ea9-8A85-E5F8CA0AF2F3\"}], \"method\": \"actEnableMethods\", \"id\": 2}");
    //jSonString2.append("\"methods\": \"camera/actTakePicture:system/getSystemInformation\", \"developerID\": \"34567890-1234-1010-8000-5c6d20c00961\"}], \"method\": \"actEnableMethods\", \"id\": 2}");

    //qDebug() << "\njSonString2: "<< jSonString2 << "\n";
    QByteArray postDataSize2 = QByteArray::number(jSonString2.size());
    QNetworkRequest request2 = constructAccessControlRequest(postDataSize2);
    LOG_REQUEST_DEBUG << "actEnabelMethods request: " << jSonString2 << "\n";
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
    LOG_REQUEST_DEBUG << "commandFabrikMethod request: "<< params << " " << jSonString << "\n";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = construcCameraRequest(postDataSize);
    manager->post(request,jSonString);
}

QNetworkRequest Remote::constructAccessControlRequest(QByteArray postDataSize){
    QUrl url = _networkConnection->getUrl();
    QString constructedurl = url.toString();
    constructedurl.append("/sony/accessControl");
    QUrl localurl(constructedurl);
    localurl.setPort(url.port());
    QNetworkRequest request(localurl);
    request.setRawHeader("","HTTP/1.1");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/xml");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postDataSize);
    //qDebug()<< "request.url().toString() "<< request.url().toString();
    //qDebug() << request.rawHeaderList();
    return request;
}

QNetworkRequest Remote::construcCameraRequest(QByteArray postDataSize){
    QUrl url = _networkConnection->getUrl();
    QString constructedurl = url.toString();
    constructedurl.append("/sony/camera");
    QUrl localurl(constructedurl);
    localurl.setPort(url.port());
    QNetworkRequest request(localurl);
    request.setRawHeader("","HTTP/1.1");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/xml");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postDataSize);
    //qDebug()<< "request.url().toString() "<< request.url().toString();
    //qDebug() << request.rawHeaderList();
    return request;
}



void Remote::onManagerFinished(QNetworkReply* reply){
    QByteArray bts = reply->readAll();
    QString str(bts);
    static bool cameraReady = false;
    static int buildid = 20;
    static bool once = false;
    QJsonDocument jdocument = QJsonDocument::fromJson(str.toUtf8());
    QJsonObject jobject = jdocument.object();
    QJsonValue jResult = jobject.value(QString("result"));
    QJsonValue jid = jobject.value(QString("id"));
    QJsonValue jtype = jobject.value(QString("type"));
    QJsonValue jError = jobject.value(QString("error"));

    LOG_RESULT_DEBUG << "jError "<< jError;
    QJsonArray jErrorArray = jError.toArray();

    double id = jid.toDouble();

    LOG_RESPONSE_DEBUG << "\n\n+++++++++++++++++++++++++++++++  onManagerFinished:   +++++++++++++++++++++++++++++++++\n"
                << methods.key(id) << reply->url() << "\n"<< str
                << "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";


    if(!jError.isUndefined()){
        //qDebug() << "+++++++++++++++++++++++++++++++ ERROR ++++++++++++++++++++++++++++++++++++++++";
        //connected = false;
        //connecting = false;
    }
    /*
    if(reply->errorString() == QString("Unknown error")){
        // everything is fine
    }
    else
        qDebug() << methods.key(id)<<" " <<"REPLY ERROR: " << reply->errorString();
    */

    if(reply->errorString() == QString("The specified configuration cannot be used.")){
        qDebug() << methods.key(id) <<"CATCH ERROR: The specified configuration cannot be used";
        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();
        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }

    if(reply->errorString() == QString("Network unreachable")){
        qDebug() << methods.key(id) <<"CATCH ERROR: Network unreachable";
        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();
        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }
    if(jErrorArray.at(1) == QString("Not Available Now")){
        qDebug() << methods.key(id) <<"CATCH ERROR: Not Available Now";
        //connected = false;
        connecting = false;
    }
    if(jErrorArray.at(1) == QString("illegal request")){
        qDebug() << methods.key(id) <<"CATCH ERROR: illegal request";
        //connected = false;
        //connecting = false;
    }
    if(jErrorArray.at(1) == QString("illegal argument")){
        qDebug() << methods.key(id) <<"CATCH ERROR:  illegal argument";
        //connected = false;
        //connecting = false;
    }

    if(jErrorArray.at(1) == QString("Forbidden")){
        qDebug() << methods.key(id) <<"CATCH ERROR: Forbidden";
        //connected = false;
        stopRecMode();
        stopLiveview();
        initialEventTimer->start();
        //connecting = false;
    }

    if(reply->errorString() == QString("Socket operation timed out")){
        qDebug() << methods.key(id) <<"CATCH ERROR: Socket operation timed out";
        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();
        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }

    if(reply->errorString() == QString("Host  not found")){
        qDebug() << methods.key(id) <<"CATCH ERROR: Host  not found";
        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();
        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }

    if(reply->errorString().contains("is unknown")){
        qDebug() << methods.key(id) <<"CATCH ERROR: Protocol \"\" is unknown";
        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();
        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }





    if(methods.key(id) == "actEnableMethods"){
        connecting = true;
        initialEventTimer->stop();
    }
    //qDebug()<<"\nQJsonValue of jResult: "<< jResult << jResult.type();
    //qDebug()<< "\nQJsonValue of jid: " << jid << jid.type();
    //qDebug()<< "\nQJsonValue of qstring: " << qstring;

    QJsonArray resultJarray = jResult.toArray();
    //qDebug() << "\nQJsonArray resultJarray: " << resultJarray<< "size: " << resultJarray.size() << "\n";

    QVariantList results = resultJarray.toVariantList();
    //qDebug() << "\nQVariantList results: " << results;
    //qDebug() << "QJsonArray id: " << id;
    QStringList sresults;


    if(methods.value("getAvailableWhiteBalance") == id){
        QStringList wbCandidates;
        for(int i = 0; i< jResult.toArray().size();i++){
            if(jResult.toArray().at(i).isArray()){
                QJsonArray array = jResult.toArray().at(i).toArray();
                //qDebug() << "HAHA: "<<  jResult.toArray().at(i) << "size: " << array.size();
                for(int y = 0; y<array.size();y++){
                    QJsonObject jobject3 = array[y].toObject();
                    //qDebug() << jobject3;
                    wbCandidates.append(jobject3.value("whiteBalanceMode").toString());
                }
                if(wbCandidates.isEmpty()){
                    wbCandidates.append(currentWhiteBalanceMode);
                }
                LOG_RESULT_DEBUG << "getAvailableWhiteBalance: " << wbCandidates;
                emit publishAvailableWhiteBalanceModes(wbCandidates);
                emit publishCurrentWhiteBalanceModes(currentWhiteBalanceMode);
            }
        }
    }

    for(int i =0;i<resultJarray.size();i++){
        QJsonObject jobject2 = resultJarray[i].toObject();
        //qDebug() << "jobject2" << jobject2.toVariantMap() << "i: " << i ;
        //qDebug() << "jobject2.keys()" << jobject2.keys();
        if(jobject2.value(QString("type")) ==(QString("availableApiList"))){

            availableMetods.clear();
            QVariantList vlist = jobject2.value("names").toArray().toVariantList();

             foreach (QVariant var, vlist) {
                 availableMetods.append(var.toString());

                 if(!methods.keys().contains(var.toString())){
                     methods[var.toString()] = buildid;
                     buildid++;
                 }
                 LOG_RESULT_DEBUG << "names: " << var.toString() << "id: "<<  methods.value(var.toString());
             }
             //qDebug() << "availableMetods: " << availableMetods << "\n";
             if(id == 66 && availableMetods.isEmpty()){
                 once = true;
                 return;
             }
             if(id == 66 && !availableMetods.isEmpty()){
                 initialEventTimer->stop();
                 initActEnabelMethods();
                 return;
             }
             _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_CONNECTET);
        }
        if(jobject2.value(QString("type")) == QString("cameraStatus")){
            LOG_RESULT_DEBUG << "cameraStatus: " << jobject2.value(QString("cameraStatus")).toString();
            if(jobject2.value(QString("cameraStatus")) == QString("NotReady")){
                cameraReady = false;
            }
        }
        if(jobject2.value(QString("type")) == QString("zoomInformation")){
            LOG_RESULT_DEBUG << "zoomNumberBox: " << jobject2.value(QString("zoomNumberBox")).toInt();
            LOG_RESULT_DEBUG << "zoomIndexCurrentBox: " << jobject2.value(QString("zoomIndexCurrentBox")).toInt();
            LOG_RESULT_DEBUG << "zoomPosition: " << jobject2.value(QString("zoomPosition")).toInt();
            LOG_RESULT_DEBUG << "zoomPositionCurrentBox: " << jobject2.value(QString("zoomPositionCurrentBox")).toInt();
            emit publishZoomPosition(jobject2.value(QString("zoomPositionCurrentBox")).toInt());
        }
        if(jobject2.value(QString("type")) == QString("liveviewStatus")){
            LOG_RESULT_DEBUG << "liveviewStatus: " << jobject2.value(QString("liveviewStatus")).toBool();
            bool status = jobject2.value(QString("liveviewStatus")).toBool();
            if(!liveViewStreamAlive && status == true){
                stopLiveview();
                status = false;
            }
            if((status == bool(false) && !manualLiveViewStart)){
                startLiveview(methods.value("startLiveview"));
            }
            emit publishLiveViewStatus(status);
        }
        if(jobject2.value(QString("type")) == QString("exposureMode")){
            QVariantList vlist = jobject2.value("exposureModeCandidates").toArray().toVariantList();
            QStringList slist;
            foreach (QVariant var, vlist) {
                slist.append(var.toString());
            }
            if(slist.isEmpty())
               slist.append(jobject2.value("currentExposureMode").toString());
            LOG_RESULT_DEBUG << "exposureModeCandidates: " << slist;
            emit publishAvailableExposureModes(slist);
            LOG_RESULT_DEBUG << "currentExposureMode: " << jobject2.value("currentExposureMode").toString();
            emit publishCurrentExposureMode(jobject2.value("currentExposureMode").toString());
        }
        if(jobject2.value(QString("type")) == QString("postviewImageSize")){
            QVariantList vlist = jobject2.value("postviewImageSizeCandidates").toArray().toVariantList();
            QStringList slist;
            foreach (QVariant var, vlist) {
                slist.append(var.toString());
            }
            LOG_RESULT_DEBUG << "postviewImageSizeCandidates: " << slist;
            emit publishAvailablePostviewImageSizeCandidates(slist);
            LOG_RESULT_DEBUG << "currentPostviewImageSize: " << jobject2.value("currentPostviewImageSize").toString();
            emit publishCurrentPostviewImageSize(jobject2.value("currentPostviewImageSize").toString());
        }
        if(jobject2.value(QString("type")) == QString("selfTimer")){
            QVariantList vlist = jobject2.value("selfTimerCandidates").toArray().toVariantList();
            QStringList slist;
            foreach (QVariant var, vlist) {
                slist.append(var.toString());
            }
            LOG_RESULT_DEBUG << "selfTimerCandidates: " << slist;
            emit publishAvailablselfTimerCandidates(slist);
            int current = jobject2.value("currentSelfTimer").toInt();
            LOG_RESULT_DEBUG << "currentSelfTimer: " << current;
            QString currentS;
            emit publishCurrentSelfTimer(currentS.setNum(current));
        }
        if(jobject2.value(QString("type")) == QString("shootMode")){
            QVariantList vlist = jobject2.value("shootModeCandidates").toArray().toVariantList();
            QStringList slist;
            foreach (QVariant var, vlist) {
                slist.append(var.toString());
            }
            LOG_RESULT_DEBUG << "shootModeCandidates: " << slist;
            emit publishAvailableShootModeCandidates(slist);
            LOG_RESULT_DEBUG << "currentShootMode: " << jobject2.value("currentShootMode").toString();
            emit publishCurrentShootMode(jobject2.value("currentFNumber").toString());
        }
        if(jobject2.value(QString("type")) == QString("exposureCompensation")){
            QStringList slist;
            int step = jobject2.value("stepIndexOfExposureCompensation").toInt();
            int max = jobject2.value("maxExposureCompensation").toInt();
            LOG_RESULT_DEBUG << "stepIndexOfExposureCompensation: " << step;
            LOG_RESULT_DEBUG << "maxExposureCompensation: " << max;
            for(int i=0;i<max;i+=step){
                QString istring;
                slist.append(istring.setNum(i));
            }

            emit publishAvailableExposureCompensation(slist);
            LOG_RESULT_DEBUG << "currentExposureCompensation: " << jobject2.value("currentExposureCompensation").toInt();
            emit publishCurrentExposureCompensation(jobject2.value("currentExposureCompensation").toString());
        }
        if(jobject2.value(QString("type")) == QString("flashMode")){
            QVariantList vlist = jobject2.value("flashModeCandidates").toArray().toVariantList();
            QStringList slist;
            foreach (QVariant var, vlist) {
                slist.append(var.toString());
            }
            LOG_RESULT_DEBUG << "flashModeCandidates: " << slist;
            emit publishAvailableFlashModeCandidates(slist);
            LOG_RESULT_DEBUG << "currentFlashMode: " << jobject2.value("currentFlashMode").toString();
            emit publishCurrentFlashMode(jobject2.value("currentFlashMode").toString());
        }
        if(jobject2.value(QString("type")) == (QString("fNumber"))){
             QVariantList vlist = jobject2.value("fNumberCandidates").toArray().toVariantList();
             QStringList slist;
             foreach (QVariant var, vlist) {
                 slist.append(var.toString());
             }
             LOG_RESULT_DEBUG << "fNumberCandidates: " << slist;
             if(slist.isEmpty())
                slist.append(jobject2.value("currentFNumber").toString());
                emit publishAvailableFNumber(slist);
             LOG_RESULT_DEBUG << "currentFNumber: " << jobject2.value("currentFNumber").toString();
             emit publishCurrentFNumber(jobject2.value("currentFNumber").toString());
        }
        if(jobject2.value(QString("type")) == (QString("isoSpeedRate"))){
             QVariantList vlist = jobject2.value("isoSpeedRateCandidates").toArray().toVariantList();
             QStringList slist;
             foreach (QVariant var, vlist) {
                 slist.append(var.toString());
             }
             LOG_RESULT_DEBUG << "isoSpeedRateCandidates: " << slist;
             if(slist.isEmpty())
                 slist.append(jobject2.value("currentIsoSpeedRate").toString());
             emit publishAvailableIsoSpeedRates(slist);
             LOG_RESULT_DEBUG << "currentIsoSpeedRate: " << jobject2.value("currentIsoSpeedRate").toString();
             emit publishCurrentIsoSpeedRates(jobject2.value("currentIsoSpeedRate").toString());
        }

        if(jobject2.value(QString("type")) == (QString("programShift"))){
             LOG_RESULT_DEBUG << "isShifted: " << jobject2.value("isShifted").toBool();
             emit publishCurrentProgramShift(jobject2.value("isShifted").toBool());
        }
        if(jobject2.value(QString("type")) == (QString("shutterSpeed"))){
             QVariantList vlist = jobject2.value("shutterSpeedCandidates").toArray().toVariantList();
             QStringList slist;
             foreach (QVariant var, vlist) {
                 slist.append(var.toString());
             }
             if(slist.isEmpty())
                slist.append(jobject2.value("currentShutterSpeed").toString());
             LOG_RESULT_DEBUG << "shutterSpeedCandidates: " << slist;
             emit publishAvailableShutterSpeed(slist);
             LOG_RESULT_DEBUG << "currentShutterSpeed: " << jobject2.value("currentShutterSpeed").toString();
             emit publishCurrentShutterSpeed(jobject2.value("currentShutterSpeed").toString());
        }
        if(jobject2.value(QString("type")) ==(QString("whiteBalance"))){
            if(jobject2.value("checkAvailability") == bool(true)){
                QStringList whiteBalanceModes;
                LOG_RESULT_DEBUG << "whiteBalance: " << jobject2.value("checkAvailability");

                currentWhiteBalanceMode = jobject2.value("currentWhiteBalanceMode").toString();
                whiteBalanceModes.append(currentWhiteBalanceMode);
                LOG_RESULT_DEBUG << "currentWhiteBalanceMode: " << currentWhiteBalanceMode;
                emit publishAvailableWhiteBalanceModes(whiteBalanceModes);
                emit publishCurrentWhiteBalanceModes(currentWhiteBalanceMode);
                getAvailableWhiteBalance(methods.value("getAvailableWhiteBalance"));
                connected = true;
                //getEventTimerPeriodic->start();
            }
        }
        if(jobject2.value(QString("type")) == (QString("touchAFPosition"))){
             bool touchAFPositionSet =    jobject2.value("currentSet").toBool();
             LOG_RESULT_DEBUG << "currentSet: " << touchAFPositionSet;
             QVariantList vlist = jobject2.value("currentTouchCoordinates").toArray().toVariantList();
             QStringList slist;
             foreach (QVariant var, vlist) {
                 slist.append(var.toString());
             }
             emit publishCurrentTouchCoordinates(slist);
             LOG_RESULT_DEBUG << "currentTouchCoordinates: " << slist;
             emit publishTouchAFPositionSet(touchAFPositionSet);
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
                    qDebug() <<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                               "\nstartRecMode finished";
                    //commandFabrikMethod("setCameraFunction",methods.value("setCameraFunction"),"Content Transfer");
                    //commandFabrikMethod("setCameraFunction",methods.value("setCameraFunction"),"Remote Shooting");
                    _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_CONNECTING);
                    //getEventTimerSingleshot->start();
                    getEventTimerPeriodic->start();
                    connected = true;
                }
                break;
            case QVariant::List:
                sresults = result.toStringList();
                if(methods.value("actTakePicture") == id && _loadpreviewpic){
                    foreach (QString sresult, sresults) {
                        buildPreviewPicName(sresult);
                        picmanager->get(QNetworkRequest(QUrl(sresult)));
                    }
                }
                if(methods.value("getAvailableApiList")==id){
                    foreach (QString sresult, sresults){
                        if(!methods.keys().contains(sresult)){
                            methods[sresult] = buildid;
                            buildid++;
                        }
                        LOG_RESULT_DEBUG << "result: " << sresult << "id: "<<  methods.value(sresult);
                    }
                }
                break;
            case QVariant::String:
                if(methods.value("startLiveview")  == id){
                        liveViewRequest = result.toString();
                        //liveViewRequest = "http://192.168.122.1:8080/liveview/liveviewstream.JPG?%211234%21http%2dget%3a%2a%3aimage%2fjpeg%3a%2a%21%21%21%21%21";
                        streamReply = liveViewManager->get(QNetworkRequest(QUrl(liveViewRequest)));
                        connect(streamReply, SIGNAL(readyRead()), this, SLOT(onLiveViewManagerReadyRead()));
                }
                break;
            case QVariant::Map:
                //! actEnableMethods2
                if(result.toMap().keys().contains("dg")){
                    QByteArray KEYDG="90adc8515a40558968fe8318b5b023fdd48d3828a2dda8905f3b93a3cd8e58dc";
                    KEYDG.append(result.toMap().value("dg").toByteArray());
                    //qDebug()<< "onManagerFinished result: " << result.toMap().value("dg").toString();
                    //qDebug()<< "onManagerFinished result: " << KEYDG;
                    QByteArray encoded = QCryptographicHash::hash(KEYDG, QCryptographicHash::Sha256);
                    if(id==1){
                        actEnabelMethods(encoded.toBase64());
                        //once = false;
                    }
                    if(id==2){


                        if(availableMetods.contains("startRecMode")){
                            startRecMode();
                        }
                        else{
                            _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_CONNECTING);
                            //getEventTimerSingleshot->setInterval(1000);
                            //getEventTimerSingleshot->start();
                            //commandFabrikMethod("setCameraFunction",99,"Content Transfer");
                            getEventTimerPeriodic->start();
                            commandFabrikMethod("setCameraFunction",99,"Remote Shooting");
                            connected = true;
                        }
                    }
                }
                break;
            }
        }
    }
}

void Remote::onLiveViewManagerReadyRead(){

    if(streamReply->isRunning())
        liveViewStreamAlive = true;



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
#ifdef Q_OS_ANDROID
            if(inputStream.at(offset) == 255  && inputStream.at(offset+1) == 216){
#else
            if(inputStream.at(offset) == -1  && inputStream.at(offset+1) == -40){
#endif
                start = offset;
                found = true;
            }
            offset++;
        }
        found = false;
        while(!found && offset<inputStream.length()-1){
#ifdef Q_OS_ANDROID
            if(inputStream.at(offset) == 255 && inputStream.at(offset+1) == 217){
#else
            if(inputStream.at(offset) == -1 && inputStream.at(offset+1) == -39){
#endif
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
    else
        liveViewStreamAlive = false;
}

void Remote::onPicmanagerFinished(QNetworkReply *reply){
    if(reply->bytesAvailable())
    emit publishLoadPreview(reply,previePicName);
}

void Remote::setLoadPreviewPic(int state){
    if(state == Qt::Unchecked)
        _loadpreviewpic = false;
    else
        _loadpreviewpic = true;
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
    qDebug() << "startRecMode CALLED";
    commandFabrikMethod("startRecMode",id);
}

void Remote::stopRecMode(int id){
    commandFabrikMethod("stopRecMode",id);
    connected = false;
    connecting = false;
    getEventTimerPeriodic->stop();
    _networkConnection->notifyConnectionStatus();
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

void Remote::setLiveViewStartToManual(){
    manualLiveViewStart = true;

}

