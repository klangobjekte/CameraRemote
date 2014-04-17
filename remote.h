#ifndef REMOTE_H
#define REMOTE_H
#include <QObject>
#include <QUrl>
#include <QStringList>
#include <QMap>


class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QGraphicsView;
class QGraphicsScene;
class QLabel;
class QTcpSocket;
class QTcpServer;
class QTimer;
class NetworkConnection;

/*
"getVersions",
"getMethodTypes",
"getApplicationInfo",
"getAvailableApiList",
"getEvent",
"actTakePicture",
"stopRecMode",
"startLiveview",
"stopLiveview",
"startLiveviewWithSize",
"actZoom",
"
awaitTakePicture" ,
"setSelfTimer",
"getSelfTimer",
"getSelfTimer" ,
"getAvailableSelfTimer",
"getSupportedSelfTimer",
"getExposureMode",
"getSupportedExposureMode",
"getExposureCompensation",
"getAvailableExposureCompensation",
"getSupportedExposureCompensation",
"setFNumber",
"getFNumber",
"getAvailableFNumber",
"getSupportedFNumber",
"setIsoSpeedRate",
"getIsoSpeedRate",
"getAvailableIsoSpeedRate",
"getSupportedIsoSpeedRate",
"getLiveviewSize",
"getAvailableLiveviewSize",
"getSupportedLiveviewSize",
"setPostviewImageSize",
"getPostviewImageSize",
"getAvailablePostviewImageSize",
"getSupportedPostviewImageSize",
"getSupportedProgramShift",
"setShootMode",
"getShootMode",
"getAvailableShootMode",
"getSupportedShootMode",

"setShutterSpeed",
"getShutterSpeed",
"getAvailableShutterSpeed",
"getSupportedShutterSpeed",

"setTouchAFPosition",
"getTouchAFPosition",

"setWhiteBalance",
"getWhiteBalance",
"getSupportedWhiteBalance",
"getAvailableWhiteBalance",
"getSupportedFlashMode"
*/
class Remote : public QObject
{
    Q_OBJECT
public:
    explicit Remote(NetworkConnection *networkConnection, QObject *parent = 0);
    ~Remote();


    void initialCommands();
    void initActEnabelMethods();
    void actEnabelMethods(QByteArray key);
    void commandFabrikMethod(QByteArray command, int id, QByteArray params = QByteArray());

    QMap<QString,int> getMethods();

    //! Static Ids!!!
    void getVersions(int id = 5);
    void getMethodTypes(int id = 6);
    void getApplicationInfo(int id = 7);
    void getAvailableApiList(int id = 8);
    void getEvent(QByteArray param, int id = 9);

    void actTakePicture(int id = 10);

    void startRecMode(int id = 11);
    void stopRecMode(int id = 12);

    void startLiveview(int id = 13);
    void stopLiveview(int id = 14);
    void startLiveviewWithSize(int id = 15);

    void startMovieRec(int id = 16);
    void stopMovieRec(int id = 17);

    //! Dynamic Ids:
    void actZoom(int id = 20);
    void awaitTakePicture(int id = 21);

    void setSelfTimer(int id = 22);
    void getSelfTimer(int id = 23);
    void getAvailableSelfTimer(int id = 24);
    void getSupportedSelfTimer(int id = 25);

    void getExposureMode(int id = 26);
    void getSupportedExposureMode(int id = 27);

    void getExposureCompensation(int id = 28);
    void getAvailableExposureCompensation(int id = 29);
    void getSupportedExposureCompensation(int id = 30);

    void setFNumber(int id = 31);
    void getFNumber(int id = 32);
    void getAvailableFNumber(int id = 33);
    void getSupportedFNumber(int id = 34);

    void setIsoSpeedRate(int id=35);
    void getIsoSpeedRate(int id = 36);
    void getAvailableIsoSpeedRate(int id = 37);
    void getSupportedIsoSpeedRate(int id = 38);


    void getLiveviewSize(int id = 39);
    void getAvailableLiveviewSize(int id = 40);
    void getSupportedLiveviewSize(int id = 41);

    void setPostviewImageSize(int id = 42);
    void getPostviewImageSize(int id = 43);
    void getAvailablePostviewImageSize(int id = 44);
    void getSupportedPostviewImageSize(int id = 45);

    void getSupportedProgramShift(int id = 46);

    void setShootMode(int id = 47);
    void getShootMode(int id = 48);
    void getAvailableShootMode(int id = 49);
    void getSupportedShootMode(int id = 50);

    void setShutterSpeed(int id = 51);
    void getShutterSpeed(int id = 52);
    void getAvailableShutterSpeed(int id = 53);
    void getSupportedShutterSpeed(int id = 54);

    void setTouchPosition(int id = 55);
    void getTouchPosition(int id = 56);

    void setWhiteBalance(int id = 57);
    void getWhiteBalance(int id = 58);
    void getSupportedWhiteBalance(int id = 59);
    void getAvailableWhiteBalance(int id = 60);
    void getSupportedFlashMode(int id = 61);

signals:
    void publishLoadPreview(QNetworkReply* reply,QString previePicName);

    void publishStartLiveView(QNetworkReply* reply,QString previePicName);
    void publishLiveViewBytes(QByteArray bytes);

    void publishAvailableIsoSpeedRates(QStringList speedRates);
    void publishAvailableFNumber(QStringList fNumber);
    void publishAvailableShutterSpeed(QStringList shutterSpeed);
    void publishAvailableWhiteBalanceModes(QStringList whiteBalanceModes);

    void publishCurrentFNumber(QString result);
    void publishCurrentIsoSpeedRates(QString result);
    void publishCurrentShutterSpeed(QString result);
    void publishCurrentWhiteBalanceModes(QString result);

public slots:
    void setLoadPreviewPic(bool loadpreviewpic);



private slots:
    void onManagerFinished(QNetworkReply* reply);
    void onPicmanagerFinished(QNetworkReply* reply);
    void onLiveViewManagerReadyRead();
    void buildLiveViewPic();

private:
    void buildPreviewPicName(QString url);

    QNetworkAccessManager *picmanager;
    QNetworkAccessManager *manager;
    QNetworkAccessManager *liveViewManager;
    QNetworkRequest construcCameraRequest(QByteArray postDataSize);
    QNetworkRequest constructAccessControlRequest(QByteArray postDataSize);
    QNetworkReply *streamReply;
    QTimer *timer;
    QByteArray inputStream;
    QByteArray imageArray;

    QMap<QString,int> methods;
    QUrl url;
    QString liveViewRequest;
    QString previePicName;
    bool _loadpreviewpic;

    int offset;
    uint64_t start;
    uint64_t end;
};

#endif // REMOTE_H
