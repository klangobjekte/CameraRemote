#ifndef LIVEVIEWTHREAD_H
#define LIVEVIEWTHREAD_H

#include <QObject>
#include <QThread>

#include <QDebug>
//#include <QSemaphore>
#include <QTime>
#include "common.h"



class QNetworkAccessManager;
class QNetworkReply;
class RingBuffer;
class NetworkConnection;

class LiveViewProducer : public QThread
{
    Q_OBJECT
public:
    explicit LiveViewProducer(NetworkConnection *networkConnection,QObject *parent);
    void run();
    void setLiveViewRequest(QString liveViewRequest);

private slots:
    void onLiveViewManagerReadyRead();
signals:
    void bufferLoaded();
    void newByteReached();

private:
    //QNetworkAccessManager *liveViewManager;
    QNetworkReply *liveReply;
    NetworkConnection *_networkConnection;

    QString _liveViewRequest;
    //QByteArray replyBuffer;


};

class LiveViewConsumer : public QThread
{
    Q_OBJECT
public:
    explicit LiveViewConsumer(QObject *parent = 0);
    void setInputStream(QNetworkReply *reply);


signals:
    void publishLiveViewBytes(QByteArray bytes);

public slots:
    void awakeConsumer();
    void sleepConsumer();

private:
    //RingBuffer *ringBuffer;

    QNetworkReply *_reply;

    void buildLiveView();
    virtual void run();
    int offset;
    uint64_t starttime;
    uint64_t endtime;
    uint64_t foundstartOffset;
    int arrayiter;

    bool foundstart;
    bool foundend;
    bool carry;

    int jpegSize ;//34048;
    int hjpegSize ;
    int paddingSize ;

    bool liveViewStreamAlive;
    //char *consumerP;

#ifdef _USESTREAM
    QDataStream inputStream;
#else
       QByteArray inputStream;
#endif


};

#endif // LIVEVIEWTHREAD_H
