#ifndef REMOTE_H
#define REMOTE_H
#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QGraphicsView;
class QGraphicsScene;
class QLabel;
class QTcpSocket;
class QTcpServer;
class QTimer;
//class QUrl;
/*
"getVersions",
"getMethodTypes",
"getApplicationInfo",
"getAvailableApiList",
"getEvent","actTakePicture",
"stopRecMode",
"startLiveview",
"stopLiveview",
"startLiveviewWithSize",
"actZoom","setSelfTimer",
"getSelfTimer",
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
    explicit Remote(QObject *parent = 0);
    void startRecMode();
    void stopRecMode();
    void actTakePicture();
    void startLiveView();
    void stopLiveView();

    void getMethodTypes();
    void getAvailableApiList();
    void getApplicationInfo();
    void getShootMode();
    void setShootMode();
    void getAvailableShootMode();
    void getSupportedShootMode();
    void startMovieRec();
    void stopMovieRec();
    void actZoom();
    void getEvent();

signals:
    void publishLoadPreview(QNetworkReply* reply,QString previePicName);
    void publishUrl(QString);
    void publishPort(QString);
    void publishStartLiveView(QNetworkReply* reply,QString previePicName);
    void publishLiveViewBytes(QByteArray bytes);

public slots:
    void setLoadPreviewPic(bool loadpreviewpic);
    void setUrl(QString urlstring);
    void setPort(QString portstring);

private slots:
    void replyFinished(QNetworkReply* reply);
    void onPicmanagerFinished(QNetworkReply* reply);
    void slotReadyRead();
    void refreshLiveView();

private:
    QNetworkAccessManager *picmanager;
    QNetworkAccessManager *manager;
    QNetworkAccessManager *liveViewManager;
    QNetworkRequest construcRequest(QByteArray postDataSize);
    QNetworkReply *streamReply;
    QTimer *timer;
    QByteArray inputStream;
    QByteArray imageArray;
    void buildPreviewPicName(QString url);


    QString liveViewRequest;
    QString previePicName;
    bool _loadpreviewpic;
    QUrl url;
    int offset;
    uint64_t start;
    uint64_t end;
};

#endif // REMOTE_H
