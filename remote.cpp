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
#include <QTime>
#include <QFile>
//#include <QSslKey>

#include <QCryptographicHash>
#include "networkconnection.h"

//#define LOG_CATCHH_ERROR
#ifdef LOG_CATCHH_ERROR
#   define LOG_CATCHH_ERROR_DEBUG qDebug()
#else
#   define LOG_CATCHH_ERROR_DEBUG nullDebug()
#endif


//#define LOG_SPECIAL_RESULT
#ifdef LOG_SPECIAL_RESULT
#   define LOG_SPECIAL_RESULT_DEBUG qDebug()
#else
#   define LOG_SPECIAL_RESULT_DEBUG nullDebug()
#endif

//! Original:
//#define LOG_RESPONSE
#ifdef LOG_RESPONSE
#   define LOG_RESPONSE_DEBUG qDebug()
#else
#   define LOG_RESPONSE_DEBUG nullDebug()
#endif

//! Decoded:
//#define LOG_RESULT
#ifdef LOG_RESULT
#   define LOG_RESULT_DEBUG qDebug()
#else
#   define LOG_RESULT_DEBUG nullDebug()
#endif

//#define LOG_REQUEST
#ifdef LOG_REQUEST
#   define LOG_REQUEST_DEBUG qDebug()
#else
#   define LOG_REQUEST_DEBUG nullDebug()
#endif

//! OSX:
//! Turn off wifi:
// networksetup -setairportpower en0 off
//! List available wifi networks:
// /System/Library/PrivateFrameworks/Apple80211.framework/Versions/A/Resources/airport scan
//! Join a wifi network
// networksetup -setairportnetwork en0 WIFI_SSID_I_WANT_TO_JOIN WIFI_PASSWORD
//! Find your network interface name:
// networksetup -listallhardwareports




Remote::Remote(NetworkConnection *networkConnection,LiveViewConsumer *liveViewConsumer, QObject *parent) :
    QObject(parent),
    _liveViewConsumer(liveViewConsumer)
{
    _networkConnection = networkConnection;
    connectionErrorCounterTimeOut = 40;
    connectionErrorCounterOffset = 20;
    connected = false;
    connecting = false;
    cameraready = false;
    inited = false;
    startLiveviewInProgress = false;
    getAvailableWhiteBalanceInProgress = false;


    manualLiveViewStart = false;
    liveViewStreamAlive = false;
    connectionstatus = _CONNECTIONSTATE_DISCONNECTED;

    offset = 0;
    start=0;
    end=0;
    _loadpreviewpic = true;
    timelapsmode = false;


    qDebug() << "networkConnection->getUrl(): " << networkConnection->getUrl();
    liveviewrate = 80;
    timer = new QTimer;
    timer->setInterval(liveviewrate);
    connect(timer,SIGNAL(timeout()), this,SLOT(buildLiveViewPic()));

    getEventTimerPeriodic = new QTimer;
    getEventTimerPeriodic->setSingleShot(false);
    getEventTimerPeriodic->setInterval(10000);
    connect(getEventTimerPeriodic,SIGNAL(timeout()),
            this,SLOT(on_getEventTimer_timeout()));

    getEventTimerSingleshot = new QTimer;
    getEventTimerSingleshot->setInterval(6000);
    getEventTimerSingleshot->setSingleShot(true);
    connect(getEventTimerSingleshot,SIGNAL(timeout()),
            this,SLOT(on_getEventTimer_timeout()));



    initialEventTimer = new QTimer;
    initialEventTimer->setInterval(1000);
    connect(initialEventTimer,SIGNAL(timeout()),
            this,SLOT(initialEvent()));

    time = new QTime;
    time->start();


    manager = new QNetworkAccessManager;
    manager->setConfiguration(networkConnection->getActiveConfiguration());
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onManagerFinished(QNetworkReply*)));



    picmanager = new QNetworkAccessManager();
    picmanager->setConfiguration(networkConnection->getActiveConfiguration());
    connect(picmanager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onPicmanagerFinished(QNetworkReply*)));

#ifdef _USEPRODUCERTHREAD
    liveViewProducer = new LiveViewProducer(_networkConnection,this);
#else
    liveViewManager = new QNetworkAccessManager;
    liveViewManager->setConfiguration(networkConnection->getActiveConfiguration());


#endif

    //! init the static methoods - map
    methods["Intital Empty Reply"] = 0;
    methods["initActEnableMethods"] = 1;
    methods["actEnableMethods"] = 2;
    methods["Reserved3"] = 3;
    methods["Reserved4"] = 4;
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
    methods["awaitTakePicture"] = 19;
    methods["getEvent Type 66"] = 66;
    methods["getEvent Type 77"] = 77;

#ifdef _USEPRODUCERTHREAD
    connect(liveViewProducer,SIGNAL(publishProducerStarted()),
            _liveViewConsumer,SLOT(awakeConsumer()),Qt::DirectConnection);
    connect(liveViewProducer,SIGNAL(publishLiveViewStreamStatus(bool)),
            this,SLOT(setLiveViewStatus(bool)),Qt::DirectConnection);
#endif
}

Remote::~Remote(){
    stopLiveview();
    stopRecMode();
    delete timer;
    delete getEventTimerPeriodic;
    delete getEventTimerSingleshot;
    #ifdef _USEPRODUCERTHREAD
#else
    delete liveViewManager;
 #endif
    delete picmanager;
    delete manager;
}

void Remote::setActiveNetworkConnection(){
    manager->setConfiguration(_networkConnection->getActiveConfiguration());
    picmanager->setConfiguration(_networkConnection->getActiveConfiguration());
#ifdef _USEPRODUCERTHREAD
#else
    liveViewManager->setConfiguration(_networkConnection->getActiveConfiguration());
#endif
}

void Remote::initialEvent(){
    if(connectionstatus == _CONNECTIONSTATE_WATING ||
            connectionstatus == _CONNECTIONSTATE_DISCONNECTED ||
            connectionstatus != _CONNECTIONSTATE_CAMERA_DETECTED)
    {
        getEvent("false",66);
   }
}

QMap<QString,int> Remote::getMethods(){
    return methods;
}

void Remote::getEventDelayed(int ms){
    getEventTimerSingleshot->setInterval(ms);
    getEventTimerSingleshot->start();

}


void Remote::on_getEventTimer_timeout(){
    qDebug() << "on_getEventTimer_timeout timelapsmode:" << timelapsmode;
    if(!timelapsmode ){
            getEvent("false",77);
        }
}


void Remote::setConnectionStatus(int status,QString message){
    Q_UNUSED(message);
    connectionstatus = status;
    qDebug() << "Remote::setConnectionStatus: " << status;
    switch(status){
    case _CONNECTIONSTATE_CONNECTET:
        break;

    case _CONNECTIONSTATE_CAMERA_DETECTED:
        if(!connected && !connecting){
            connecting = true;
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

int Remote::getConnectionStatus(){
    return connectionstatus;
}

bool Remote::getConnectionState(){
    return connected;
}

void Remote::setConnetcionState(bool state){
    if(!state){
        connected = false;
    }

}


void Remote::initActEnableMethods(bool manual){
    Q_UNUSED(manual);
    inited = true;
    QByteArray jSonString = "{\"version\": \"1.0\", \"params\": [{\"developerName\": \"\", \"sg\": \"\", \"methods\": \"\", \"developerID\": \"\"}], \"method\": \"actEnableMethods\", \"id\": 1}";
    QByteArray postDataSize = QByteArray::number(jSonString.size());
    QNetworkRequest request = constructAccessControlRequest(postDataSize);
    int hours = time->elapsed()/(60*60*1000);
    int rest = time->elapsed()%(60*60*1000);
    int min = rest/60000;
    rest = rest%60000;
    int sec = rest/1000;
    rest = rest%1000;
    int ms = rest;
    //LOG_SPECIAL_RESULT_DEBUG << "onManagerFinished  reply                        : " << hours << ":"<<min<<":"<<sec<<"."<<ms;
    LOG_REQUEST_DEBUG << "initActEnableMethods request: " << jSonString << "time:   " << hours << ":"<<min<<":"<<sec<<"."<<ms << "\n";
    manager->post(request,jSonString);
}

void Remote::actEnableMethods(QByteArray key){
    QByteArray jSonString2 = "{\"version\": \"1.0\", \"params\": [{\"developerName\": \"Sony Corporation\", ";
    jSonString2.append("\"sg\": \"");
    jSonString2.append(key);
    jSonString2.append("\", ");
    jSonString2.append("\"methods\": \"camera/setFlashMode:camera/getFlashMode:camera/getSupportedFlashMode:"
                       "camera/getAvailableFlashMode:camera/setExposureCompensation:camera/getExposureCompensation:"
                       "camera/getSupportedExposureCompensation:camera/getAvailableExposureCompensation:camera/setSteadyMode:"
                       "camera/getSteadyMode:camera/getSupportedSteadyMode:camera/getAvailableSteadyMode:camera/setViewAngle:"
                       "camera/getViewAngle:camera/getSupportedViewAngle:camera/getAvailableViewAngle:camera/setMovieQuality:"
                       "camera/getMovieQuality:camera/getSupportedMovieQuality:camera/getAvailableMovieQuality:camera/setFocusMode:"
                       "camera/getFocusMode:camera/getSupportedFocusMode:camera/getAvailableFocusMode:camera/setStillSize:"
                       "camera/getStillSize:camera/getSupportedStillSize:camera/getAvailableStillSize:camera/setBeepMode:"
                       "camera/getBeepMode:camera/getSupportedBeepMode:camera/getAvailableBeepMode:camera/setCameraFunction:"
                       "camera/getCameraFunction:camera/getSupportedCameraFunction:camera/getAvailableCameraFunction:"
                       "camera/setLiveviewSize:camera/getLiveviewSize:camera/getSupportedLiveviewSize:camera/getAvailableLiveviewSize:"
                       "camera/setTouchAFPosition:camera/getTouchAFPosition:camera/cancelTouchAFPosition:camera/setFNumber:"
                       "camera/getFNumber:camera/getSupportedFNumber:camera/getAvailableFNumber:camera/setShutterSpeed:"
                       "camera/getShutterSpeed:camera/getSupportedShutterSpeed:camera/getAvailableShutterSpeed:camera/setIsoSpeedRate:"
                       "camera/getIsoSpeedRate:camera/getSupportedIsoSpeedRate:camera/getAvailableIsoSpeedRate:camera/setExposureMode:"
                       "camera/getExposureMode:camera/getSupportedExposureMode:camera/getAvailableExposureMode:camera/setWhiteBalance:"
                       "camera/getWhiteBalance:camera/getSupportedWhiteBalance:camera/getAvailableWhiteBalance:camera/setProgramShift:"
                       "camera/getSupportedProgramShift:camera/getStorageInformation:camera/startLiveviewWithSize:"
                       "camera/startIntervalStillRec:camera/stopIntervalStillRec:camera/actFormatStorage:system/setCurrentTime\", "
                       "\"developerID\": \"7DED695E-75AC-4ea9-8A85-E5F8CA0AF2F3\"}], \"method\": \"actEnableMethods\", \"id\": 2}");



    //jSonString2.append("\"methods\": \"camera/actTakePicture:system/getSystemInformation\", \"developerID\": \"34567890-1234-1010-8000-5c6d20c00961\"}], \"method\": \"actEnableMethods\", \"id\": 2}");

    //qDebug() << "\njSonString2: "<< jSonString2 << "\n";
    QByteArray postDataSize2 = QByteArray::number(jSonString2.size());
    QNetworkRequest request2 = constructAccessControlRequest(postDataSize2);
    int hours = time->elapsed()/(60*60*1000);
    int rest = time->elapsed()%(60*60*1000);
    int min = rest/60000;
    rest = rest%60000;
    int sec = rest/1000;
    rest = rest%1000;
    int ms = rest;
    //LOG_SPECIAL_RESULT_DEBUG << "onManagerFinished  reply                        : " << hours << ":"<<min<<":"<<sec<<"."<<ms;
    LOG_REQUEST_DEBUG << "actEnableMethods request: " << jSonString2 << "time:   " << hours << ":"<<min<<":"<<sec<<"."<<ms  << "\n";
    manager->post(request2,jSonString2);
}


void Remote::commandFabrikMethod(QByteArray command, int id, QByteArray params){
    static int connectionErrorCounter=connectionErrorCounterOffset;
    if(id == 66){
        connectionErrorCounter++;
        qDebug() << "connectionErrorCounter" << connectionErrorCounter;
        if(connectionErrorCounter >= connectionErrorCounterTimeOut){

            qDebug() << "Check Camera/Wifi Connection and restart the Application!";
            connectionErrorCounter = 0;
            QString message;
            emit publishConnetionError(message);
        }

    }
    //QByteArray jSonString = "{\"method\":\"getMethodTypes\",\"params\":[],\"id\":3}";
    QByteArray jSonString = "{\"method\":\"";
    jSonString.append(command);
    jSonString.append("\",\"params\":[");
    jSonString.append(params);
    jSonString.append("],\"id\":");
    QByteArray idNum;
    jSonString.append(idNum.setNum(id));
    jSonString.append("}");
    int hours = time->elapsed()/(60*60*1000);
    int rest = time->elapsed()%(60*60*1000);
    int min = rest/60000;
    rest = rest%60000;
    int sec = rest/1000;
    rest = rest%1000;
    int ms = rest;
    //LOG_SPECIAL_RESULT_DEBUG << "onManagerFinished  reply                        : " << hours << ":"<<min<<":"<<sec<<"."<<ms;
    LOG_REQUEST_DEBUG << "commandFabrikMethod request           : " << jSonString << "time:   " << hours << ":"<<min<<":"<<sec<<"."<<ms ;
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

void Remote::onManagerReadyRead(){
    qDebug() << "+++++++++++++++++++++ onManagerReadyRead +++++++++++++++++++++++++++++";
}


void Remote::onManagerFinished(QNetworkReply* reply){
    //_reply = reply;
    //connect(_reply, SIGNAL(readyRead()), this, SLOT(onManagerReadyRead()));
    QByteArray bts = reply->readAll();
    QString str(bts);
    static int buildid = 20;

    //getAvailableApiList();
    //cameraready = false;
    QJsonDocument jdocument = QJsonDocument::fromJson(str.toUtf8());
    QJsonObject jobject = jdocument.object();
    QJsonValue jResult = jobject.value(QString("result"));
    QJsonValue jid = jobject.value(QString("id"));
    QJsonValue jtype = jobject.value(QString("type"));
    QJsonValue jError = jobject.value(QString("error"));

    LOG_RESULT_DEBUG << "jError "<< jError;
    QJsonArray jErrorArray = jError.toArray();
    QJsonArray jResultArray = jResult.toArray();

    double id = jid.toDouble();
    //!to minimize whitebalance requests

    if(methods.key(id) == QString("actTakePicture")){
        LOG_SPECIAL_RESULT_DEBUG << "\n\n+++++++++++++++++++++++++++++++  onManagerFinished:   +++++++++++++++++++++++++++++++++\n"
                    << methods.key(id) << reply->url() << "\n"<< str
                    << "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    }


    LOG_RESPONSE_DEBUG << "\n\n+++++++++++++++++++++++++++++++  onManagerFinished:   +++++++++++++++++++++++++++++++++\n"
                << methods.key(id) << reply->url() << "\n"<< str
                << "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    //LOG_SPECIAL_RESULT_DEBUG << "onManagerFinished";
    int hours = time->elapsed()/(60*60*1000);
    int rest = time->elapsed()%(60*60*1000);
    int min = rest/60000;
    rest = rest%60000;
    int sec = rest/1000;
    rest = rest%1000;
    int ms = rest;
    //LOG_SPECIAL_RESULT_DEBUG << "onManagerFinished  reply                        : " << hours << ":"<<min<<":"<<sec<<"."<<ms;
    LOG_SPECIAL_RESULT_DEBUG << "onManagerFinished  reply              :   method: " << methods.key(id )<< "    id:" << id <<"    Error: "  << jError << "time:   " << hours << ":"<<min<<":"<<sec<<"."<<ms  << "\n";
    //LOG_SPECIAL_RESULT_DEBUG << methods.key(id)<<" " <<"REPLY ERROR: " << reply->errorString();

    if(jError.isUndefined()){
        //LOG_SPECIAL_RESULT_DEBUG << "+++++++++++++++++++++++++++++++ NO ERROR ++++++++++++++++++++++++++++++++++++++";
        cameraready = true;
    }

    //Remote::onManagerFinished jError:  QJsonValue(array, QJsonArray([503,"service unavailable"]) )
    //Remote::onManagerFinished jError:  QJsonValue(array, QJsonArray([5,"illegal request"]) )
    if(jErrorArray.at(1) == QString("Not Available Now")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: Not Available Now";
        connecting = false;
        cameraready = false;
        QString method = methods.key(id);
        //if(id!=0 && !_CONNECTIONSTATE_DISCONNECTED)
        //    commandFabrikMethod(method.toUtf8(),id);

    }
    if(jErrorArray.at(1) == QString("Timed out")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: Timed out";
        connecting = false;
        cameraready = false;
        QString method = methods.key(id);
        //if(id!=0 && !_CONNECTIONSTATE_DISCONNECTED)
        //    commandFabrikMethod(method.toUtf8(),id);

    }


    if(jErrorArray.at(1) == QString("illegal request")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: illegal request";
        cameraready = false;
    }
    if(jErrorArray.at(1) == QString("illegal argument")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR:  illegal argument";
        cameraready = false;
    }

    if(jErrorArray.at(1) == QString("Forbidden")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: Forbidden";
        initialEventTimer->start();
        cameraready = false;
    }

    if(reply->errorString() == QString("The specified configuration cannot be used.")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: The specified configuration cannot be used";

        if(getEventTimerPeriodic->isActive())

            getEventTimerPeriodic->stop();

        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        cameraready = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }
    else if(reply->errorString() == QString("Network unreachable")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: Network unreachable";

        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();

        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        cameraready = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }
    else if(reply->errorString() == QString("Socket operation timed out")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: Socket operation timed out";

        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();

        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        cameraready = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }
    else if(reply->errorString() == QString("Host  not found")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: Host  not found";

        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();

        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        cameraready = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }
    else if(reply->errorString().contains("is unknown")){
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: Protocol \"\" is unknown";

        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();

        if(!initialEventTimer->isActive())
            initialEventTimer->start();
        connected = false;
        connecting = false;
        cameraready = false;
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_WATING);
    }
    else if (reply->errorString() ==  QString("Unknown error")){
        //! Means no Error
        LOG_CATCHH_ERROR_DEBUG << methods.key(id) <<"CATCH ERROR: " << reply->errorString();
        if(id==66 && !connecting){
            if(connectionstatus != _CONNECTIONSTATE_CAMERA_DETECTED){
                _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_CAMERA_DETECTED);
            }
            connecting = true;
            initialEventTimer->stop();
        }
    }



    getCommand(id);



    //! initActEnableMethods  result.type: QVariant::Map
    if(methods.key(id) == "initActEnableMethods"){
        //LOG_SPECIAL_RESULT_DEBUG << "initActEnableMethods jdg                    : " << jdg;
        //LOG_SPECIAL_RESULT_DEBUG << "initActEnableMethods jResult                : " << jResult;
        //LOG_SPECIAL_RESULT_DEBUG << "initActEnableMethods jResult.type()         : " << jResult.type();
        //LOG_SPECIAL_RESULT_DEBUG << "initActEnableMethods jResultArray           : " << jResultArray;
        for(int i= 0;i<jResultArray.size();i++){
            QJsonArray array = jResult.toArray().at(i).toArray();
            LOG_SPECIAL_RESULT_DEBUG << "jResultArray.at(i).type()             : "<<jResultArray.at(i).type();
            LOG_SPECIAL_RESULT_DEBUG << "jResultArray.at(i)                    : "<<jResultArray.at(i);
            LOG_SPECIAL_RESULT_DEBUG << "jResult.toArray().at(i).toArray()     : "<<jResult.toArray().at(i).toArray();
            QJsonObject jobject3 = jResultArray[i].toObject();
            QString keyanswer = jobject3.value("dg").toString();
            LOG_SPECIAL_RESULT_DEBUG << "jobject3.value(dg)                    : "<< keyanswer;
            //! Calculate the KEY
            QByteArray KEYDG="90adc8515a40558968fe8318b5b023fdd48d3828a2dda8905f3b93a3cd8e58dc";
            KEYDG.append(keyanswer);
            qDebug()<< "initActEnableMethods result: " << keyanswer;
            qDebug()<< "initActEnableMethods result: " << KEYDG;
            QByteArray encoded = QCryptographicHash::hash(KEYDG, QCryptographicHash::Sha256);
            actEnableMethods(encoded.toBase64());
        }
    }
    //! actEnableMethods - set the Forbidden to Allowed - id 2!
    if(methods.key(id) == "actEnableMethods"){
        if(availableMetods.contains("startRecMode")){
            startRecMode();
        }
        else if(availableMetods.contains("setCameraFunction")){
            setCameraFunction();
        }
        else{
            if(!getEventTimerSingleshot->isActive()){
                getEventTimerSingleshot->setInterval(2000);
                getEventTimerSingleshot->start();
            }
            if(!getEventTimerPeriodic->isActive())
                getEventTimerPeriodic->start();
            connected = true;
            LOG_SPECIAL_RESULT_DEBUG << "connected                             : " << connected;
        }
        connecting = true;
        initialEventTimer->stop();
    }



    if(methods.key(id) == "getAvailableApiList"){
        //LOG_SPECIAL_RESULT_DEBUG << "jResult                               : " << jResult << "id: "<<  id;
        //LOG_SPECIAL_RESULT_DEBUG << "jResultArray                          : " << jResultArray << "id: "<<  id;
        for(int i= 0;i<jResultArray.size();i++){
            QJsonArray array = jResult.toArray().at(i).toArray();
            for(int y = 0; y<array.size();y++){
                QString sresult = array.at(y).toString();
                if(!methods.keys().contains(sresult)){
                    methods[sresult] = buildid;

                    LOG_SPECIAL_RESULT_DEBUG << "getAvailableApiList append to methods : "<< methods.key(id) << buildid;
                    buildid++;
                }
            }
        }
    }

    //! Recognize startRecMode result.type: QVariant::Double
    if(methods.value("startRecMode") == id){
        //qDebug() <<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
        //           "\nstartRecMode response\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        if(!getEventTimerSingleshot->isActive()){
            getEventTimerSingleshot->setInterval(4000);
            getEventTimerSingleshot->start();
        }
        if(!getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->start();
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_CONNECTING);
        connected = true;
        LOG_SPECIAL_RESULT_DEBUG << "connected                             : " << connected;

    }

    if(methods.key(id) == QString("stopRecMode")){
        emit publishDisconnected();
        connected = false;
        LOG_SPECIAL_RESULT_DEBUG << "connected                             : " << connected;
    }

    if(methods.value("setCameraFunction") == id){
        //qDebug() <<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
        //           "\nsetCameraFunction response\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        if(!getEventTimerSingleshot->isActive()){
            getEventTimerSingleshot->setInterval(4000);
            getEventTimerSingleshot->start();
        }
        if(!getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->start();
        _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_CONNECTING);
        connected = true;
        LOG_SPECIAL_RESULT_DEBUG << "connected                             : " << connected;
    }

    if(methods.key(id) == "startLiveview"){
        for(int i= 0;i<jResultArray.size();i++){
            liveViewRequest = jResultArray.at(i).toString();
            qDebug() << "liveViewRequest                       :  " << liveViewRequest;
            //liveViewRequest = "http://192.168.122.1:8080/liveview/liveviewstream.JPG?%211234%21http%2dget%3a%2a%3aimage%2fjpeg%3a%2a%21%21%21%21%21";

#ifdef _USEPRODUCERTHREAD
            liveViewProducer->setLiveViewRequest(liveViewRequest);
            if(!liveViewProducer->isRunning()){
                liveViewProducer->start();
                startLiveviewInProgress = true;
            }
            //_liveViewConsumer->start();

#else
            streamReply = liveViewManager->get(QNetworkRequest(QUrl(liveViewRequest)));
            qDebug()<< "streamReply->readBufferSize(): " << streamReply->readBufferSize();
            connect(streamReply, SIGNAL(readyRead()), this, SLOT(onLiveViewManagerReadyRead()));
//streamReply->readyRead();
#endif

        }
    }

    if(methods.key(id) == "getAvailableWhiteBalance"){
        getAvailableWhiteBalanceInProgress = false;
        for(int i = 0; i< jResultArray.size();i++){
            if(jResult.toArray().at(i).isArray()){
                QJsonArray array = jResult.toArray().at(i).toArray();
                //qDebug() << "HAHA: "<<  jResult.toArray().at(i) << "size: " << array.size();
                for(int y = 0; y<array.size();y++){
                    QJsonObject jobject3 = array[y].toObject();
                    QString wb = jobject3.value("whiteBalanceMode").toString();
                    if(!whiteBalanceModes.contains(wb))
                        whiteBalanceModes.append(wb);
                }
                LOG_RESULT_DEBUG << "getAvailableWhiteBalance: " << whiteBalanceModes;
                emit publishAvailableWhiteBalanceModes(whiteBalanceModes);
                emit publishCurrentWhiteBalanceModes(currentWhiteBalanceMode);
            }
        }
    }

    if(methods.key(id) == "actTakePicture" && _loadpreviewpic){
        for(int i= 0;i<jResultArray.size();i++){
            QJsonArray array = jResult.toArray().at(i).toArray();
            for(int y = 0; y<array.size();y++){
                QString picurl = array.at(y).toString();
                buildPreviewPicName(picurl);
                picmanager->get(QNetworkRequest(QUrl(picurl)));
                LOG_SPECIAL_RESULT_DEBUG << "array.at(y).toString()                : "<<picurl;
            }
        }
    }
    if(methods.key(id) == "setShutterSpeed"){
        getEvent();
    }

    if(methods.key(id) == "setIsoSpeedRate"){
        getEvent();
    }

    if(methods.key(id) == "setFNumber"){
        //getEvent();
    }

    if(methods.key(id) == "setExposureMode"){
        //getEvent();
    }

    if(methods.key(id) == "setSelfTimer"){
        //getEvent();
    }

    if(methods.key(id) == "setPostviewImageSize"){
        //getEvent();
    }

    for(int i =0;i<jResultArray.size();i++){
        QJsonObject jobject2 = jResultArray[i].toObject();
        if(jobject2.value(QString("type")) ==(QString("availableApiList"))){
            availableMetods.clear();
            QVariantList vlist = jobject2.value("names").toArray().toVariantList();
            foreach (QVariant var, vlist) {
                availableMetods.append(var.toString());
                if(!methods.keys().contains(var.toString())){
                    methods[var.toString()] = buildid;
                    //LOG_SPECIAL_RESULT_DEBUG << "getAvailableApiList append to methods :  "<< methods.key(buildid) << buildid;

                    buildid++;
                }
                LOG_RESULT_DEBUG << "names: " << var.toString() << "id: "<<  methods.value(var.toString());
             }

             if(id == 66 && !availableMetods.isEmpty()){
                    if(!inited && availableMetods.size() < 20){
                        initActEnableMethods();
                    }
                    else{
                        if(!getEventTimerPeriodic->isActive())
                            getEventTimerPeriodic->start();
                        if(!getEventTimerSingleshot->isActive()){
                            getEventTimerSingleshot->setInterval(2000);
                            getEventTimerSingleshot->start();
                        }
                        connected = true;
                    }
                    return;
             }
             else if(id == 66 &&availableMetods.isEmpty() ){
                 getEvent("false",88);
                 return;
             }
             if(id == 88 && availableMetods.isEmpty()){
                 getEvent("false",99);
                 return;
             }
             else if(id == 88 && !availableMetods.isEmpty()){
                getEvent("false",99);
                return;
             }
             if(id == 99 && !availableMetods.isEmpty()){
                 if(!inited)
                    initActEnableMethods();
                 return;
             }
             _networkConnection->notifyConnectionStatus(_CONNECTIONSTATE_CONNECTET);
        }
        if(jobject2.value(QString("type")) == QString("cameraStatus")){
            camerastatus = jobject2.value(QString("cameraStatus")).toString();
            LOG_SPECIAL_RESULT_DEBUG << "cameraStatus                          : " << camerastatus;
            if(!connected)
                emit publishCameraStatus(camerastatus);
            if(camerastatus == QString("IDLE")){
                //cameraready = true;
            }
            if(camerastatus == QString("NotReady")){
                cameraready = false;
                getEvent();
            }
            if(camerastatus == QString("StillCapturing")){
                cameraready = false;
            }
        }
        if(jobject2.value(QString("type")) == QString("zoomInformation")){

            LOG_SPECIAL_RESULT_DEBUG << "zoomNumberBox                         : " << jobject2.value(QString("zoomNumberBox")).toInt();
            LOG_SPECIAL_RESULT_DEBUG << "zoomIndexCurrentBox                   : " << jobject2.value(QString("zoomIndexCurrentBox")).toInt();
            LOG_SPECIAL_RESULT_DEBUG << "zoomPosition                          : " << jobject2.value(QString("zoomPosition")).toInt();
            LOG_SPECIAL_RESULT_DEBUG << "zoomPositionCurrentBox                : " << jobject2.value(QString("zoomPositionCurrentBox")).toInt();
            //if(id==20)
            //if(jobject2.value(QString("zoomPositionCurrentBox")).toInt()>0)
            emit publishZoomPosition(jobject2.value(QString("zoomPositionCurrentBox")).toInt());
        }
        if(jobject2.value(QString("type")) == QString("liveviewStatus")){
            liveviewstatus = jobject2.value(QString("liveviewStatus")).toBool();
            LOG_RESULT_DEBUG << "liveviewStatus                        :  " << liveviewstatus;
            LOG_SPECIAL_RESULT_DEBUG << "liveviewStatus                        : " << liveviewstatus;
            LOG_SPECIAL_RESULT_DEBUG << "connected                             : " << connected;
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
        //{"type":"postviewImageSize","postviewImageSizeCandidates":["Original","2M"],"currentPostviewImageSize":"2M"}
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
                //whiteBalanceModes.clear();

                LOG_SPECIAL_RESULT_DEBUG << "whiteBalance                          : " << jobject2.value("checkAvailability");
                currentWhiteBalanceMode = jobject2.value("currentWhiteBalanceMode").toString();
                if(!currentWhiteBalanceMode.isEmpty() && !whiteBalanceModes.contains(currentWhiteBalanceMode))
                    whiteBalanceModes.append(currentWhiteBalanceMode);
                LOG_SPECIAL_RESULT_DEBUG << "currentWhiteBalanceMode               : " << currentWhiteBalanceMode;
                emit publishAvailableWhiteBalanceModes(whiteBalanceModes);
                emit publishCurrentWhiteBalanceModes(currentWhiteBalanceMode);
            }
        }
        if(jobject2.value(QString("type")) == (QString("touchAFPosition"))){
             bool touchAFPositionSet =    jobject2.value("currentSet").toBool();
             LOG_RESULT_DEBUG << "touchAFPosition currentSet: " << touchAFPositionSet;
             //QVariantList vlist = jobject2.value("currentTouchCoordinates").toArray().toVariantList();
             double vlist = jobject2.value("currentTouchCoordinates").toDouble();
             QStringList slist;
             //foreach (QVariant var, vlist) {
             //    slist.append(var.toString());
             //}
             emit publishCurrentTouchCoordinates(slist);
             LOG_RESULT_DEBUG << "currentTouchCoordinates: " << vlist;
             emit publishTouchAFPositionSet(touchAFPositionSet);
        }

        //{"type":"focusMode","focusModeCandidates":["AF-S","AF-C","DMF","MF"],"currentFocusMode":"AF-S"}
        if(jobject2.value(QString("type")) == (QString("focusMode"))){
            QVariantList vlist = jobject2.value("focusModeCandidates").toArray().toVariantList();
            QStringList slist;
            foreach (QVariant var, vlist) {
                slist.append(var.toString());
            }
            LOG_RESULT_DEBUG << "focusModeCandidates: " << slist;
            emit publishAvailableFocusModeCandidates(slist);
            LOG_RESULT_DEBUG << "currentFocusMode: " << jobject2.value("currentFocusMode").toString();
            emit publishCurrentFocusMode(jobject2.value("currentFocusMode").toString());
        }






    }
}



void Remote::getCommand(int id){
    static int connectionErrorCounter = connectionErrorCounterOffset;
    if(id == 66){
        connectionErrorCounter++;
        qDebug() << "connectionErrorCounter" << connectionErrorCounter;
        if(connectionErrorCounter >= connectionErrorCounterTimeOut){
            qDebug() << "Check Camera/Wifi Connection and restart the Application!";
            connectionErrorCounter = 0;
            QString message;
            emit publishConnetionError(message);
        }
        return;
    }

    if(connected
        && !liveViewStreamAlive
        && !timelapsmode//)
        && !startLiveviewInProgress)
    {
        startLiveviewInProgress = true;
        startLiveview();
        return;
    }


    //qDebug() << "++++++++++++++++++++++++++++++++++++++++++++++++++++whiteBalanceModes" << whiteBalanceModes;
    if(whiteBalanceModes.size()<2
            && connected
            && !timelapsmode
            && !getAvailableWhiteBalanceInProgress)
    {
        if(methods.value("getAvailableWhiteBalance") != 0)
        {
            getAvailableWhiteBalance(methods.value("getAvailableWhiteBalance"));
            getAvailableWhiteBalanceInProgress = true;
        }
        return;
    }
    //commandFabrikMethod("startIntervalStillRec",100);
    //commandFabrikMethod("getAvailableFocusMode",101);
}

void Remote::onLiveViewManagerReadyRead(){
    //qDebug() << "onLiveViewManagerReadyRead";
    //qDebug() << "streamReply->bytesAvailable(): "<< streamReply->bytesAvailable();

    startLiveviewInProgress = false;
    liveViewStreamAlive = false;
    if(streamReply->isRunning())
    {
        liveViewStreamAlive = true;
    }
    //inputStream.append(streamReply->readAll());
    //QDataStream in(streamReply->readAll());
   // _liveViewConsumer->setInputStream(streamReply);
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


//! Call by Timer
void Remote::buildLiveViewPic(){
    //qDebug() << "\n++++++++++++++++++++++++++++++++++++++";
    //qDebug() << "buildLiveViewPic";
    //qDebug()<< "offset" << offset << "start: " << start << "end: " << end;
    QByteArray array;
    int jpegSize = 34048;
//#define __USE_STANDARD_HEADER
#ifdef  __USE_STANDARD_HEADER
    int commonHeaderLength = 1 + 1 + 2 + 4;
    int payloadHeaderLength = 4 + 3 + 1 + 4 + 1 + 115;
    QByteArray commonHeader;
    QByteArray payloadHeader;

    for(int i=0;i<commonHeaderLength;i++)
        commonHeader[i] = inputStream[i];
    if (commonHeader.isNull() || commonHeader.length() != commonHeaderLength) {
              qDebug() << "Cannot read stream for common header.";
          }
          if (commonHeader[0] != (char) 0xFF) {
              qDebug() << "Unexpected data format. (Start byte)";
          }
          if (commonHeader[1] != (char) 0x01) {
              qDebug() << "Unexpected data format. (Payload byte)";
          }


    for(int i=0;i<payloadHeaderLength;i++)
        payloadHeader[i] = inputStream[i+commonHeaderLength];
    if (payloadHeader.isNull() || payloadHeader.length() != payloadHeaderLength) {
        qDebug() << "Cannot read stream for payload header.";
    }
    if (payloadHeader[0] != (char) 0x24
            || payloadHeader[1] != (char) 0x35
            || payloadHeader[2] != (char) 0x68
            || payloadHeader[3] != (char) 0x79) {
        qDebug() << "Unexpected data format. (Start code)";
    }

    jpegSize = bytesToInt(payloadHeader, 4, 3);
    //int paddingSize = bytesToInt(payloadHeader, 7, 1);

       // Payload Data
       //byte[] jpegData = readBytes(mInputStream, jpegSize);
       //byte[] paddingData = readBytes(mInputStream, paddingSize);

    for(int i=0;i < jpegSize; i++){
        //array.append(inputStream.at(i));
        int y = i+commonHeaderLength+payloadHeaderLength;
        array[i] = inputStream[y];
    }
#endif

    bool found = false;
    bool foundstart = false;
    bool foundend = false;

    if(!inputStream.isEmpty()){
        //! suche nach Bildanfang ab Stelle offset
        while(!found && offset<inputStream.length()-1 ){
#ifdef Q_OS_ANDROID
            if(inputStream.at(offset) == 255  && inputStream.at(offset+1) == 216){
#else
            if(inputStream.at(offset) == -1  && inputStream.at(offset+1) == -40){
#endif
                start = offset;
                found = true;
                foundstart = true;
            }
            offset++;
        }//while
        found = false;
        int arrayiter=0;
        //! suche nach Bildende ab Stelle offset
        while(!found && offset<inputStream.length()-1){

#ifdef Q_OS_ANDROID
            if(inputStream.at(offset) == 255 && inputStream.at(offset+1) == 217){
#else
            if(inputStream.at(offset) == -1 && inputStream.at(offset+1) == -39){
#endif
                end = offset;
                found = true;
                foundend = true;
            }
            array[arrayiter] = inputStream.at(offset-1);
            arrayiter++;
            offset++;
        }//while
        //! Bild gefunden
        if(found){
            jpegSize = end-start;
            //for(int i=0,y=start;i < jpegSize; i++,y++){
            //    array[i] = inputStream.at(y);
            //}
            emit publishLiveViewBytes(array);
            if(offset > (jpegSize*1000/liveviewrate*3)){
                inputStream.clear();
                offset = 0;
                start = 0;
                end = 0;
                qDebug() << "Buffer Cleared";
            }
        }
    }
    else
        liveViewStreamAlive = false;
}

void Remote::onPicmanagerFinished(QNetworkReply *reply){
    camerastatus = "FINISHED";
    cameraready = true;
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
        qDebug() << "previePicName                         : " << previePicName;
}

QString Remote::cameraStatus(){
    //qDebug() << "cameraStatus:" << camerastatus;
    return camerastatus;
}

bool Remote::cameraReady(){
    return cameraready;
}


void Remote::setTimeLapsMode(bool on){
    timelapsmode = on;
    if(on){
        if(getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->stop();
    }
    else{
        if(!getEventTimerPeriodic->isActive())
            getEventTimerPeriodic->start();
    }
}

void Remote::setRefreshInterval(int ms){
    getEventTimerPeriodic->stop();
    getEventTimerPeriodic->setInterval(ms);
    getEventTimerPeriodic->start();
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
    cameraready = false;
     commandFabrikMethod("actTakePicture",id);

}

void Remote::awaitTakePicture(int id){
    commandFabrikMethod("awaitTakePicture",id);
}

void Remote::startRecMode(int id){
    commandFabrikMethod("startRecMode",id);
}

void Remote::stopRecMode(int id){
    commandFabrikMethod("stopRecMode",id);
    connected = false;
    connecting = false;
    //getEventTimerPeriodic->stop();
    _networkConnection->notifyConnectionStatus();
    LOG_SPECIAL_RESULT_DEBUG << "connected                             : " << connected;
}

void Remote::startLiveview(int id){
    commandFabrikMethod("startLiveview",id);
    inputStream.clear();
    //timer->start();
    offset = 0;
    start=0;
    end=0;
}

void Remote::stopLiveview(int id){
    commandFabrikMethod("stopLiveview",id);
    //timer->stop();
    _liveViewConsumer->exit();
    liveViewProducer->stop();
    liveviewstatus = false;
    liveViewStreamAlive = false;
}

void Remote::startLiveviewWithSize(int id){
    commandFabrikMethod("startLiveviewWithSize",id);
}

void Remote::setCameraFunction(int id){
    commandFabrikMethod("setCameraFunction",id,"\"Remote Shooting\"");
}

void Remote::actZoom(int id){
    commandFabrikMethod("actZoom",id);
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

void Remote::setTouchAFPosition(int id){
    commandFabrikMethod("setTouchAFPosition",id);
}

void Remote::getTouchAFPosition(int id){
    commandFabrikMethod("getTouchAFPosition",id);
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

void Remote::setLiveViewStartToManual(bool state){
    manualLiveViewStart = state;
}

bool Remote::liveviewStatus(){
    return liveviewstatus;
}

void Remote::setLiveViewStatus(bool status){
    liveviewstatus = status;
    liveViewStreamAlive = status;
}
