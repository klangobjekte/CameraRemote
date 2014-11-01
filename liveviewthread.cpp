#include "liveviewthread.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QWaitCondition>
#include <QMutex>
//#include "ringbuffer.h"
#include "networkconnection.h"


#include <stdio.h>
#include <stdlib.h>


#define LOG_LIVEVIETHREAD
#ifdef LOG_LIVEVIETHREAD
#   define LOG_LIVEVIETHREAD_DEBUG qDebug()
#else
#   define LOG_LIVEVIETHREAD_DEBUG nullDebug()
#endif


QSemaphore freeSpace(1);
QSemaphore usedSpace;
unsigned int realLoadSize =0;
//QWaitCondition bufferIsNotFull;
QWaitCondition bufferIsNotEmpty;
QMutex mutex;

RingBuffer *ringBuffer = new RingBuffer(gRingBufferSize);

int bytesToInt(QByteArray byteData, int startIndex, int count) {
    int ret = 0;
    for (int i = startIndex; i < startIndex + count; i++) {
        ret = (ret << 8) | (byteData[i] & 0xff);
    }
    return ret;
}





LiveViewProducer::LiveViewProducer(
        NetworkConnection *networkConnection,
        QObject *parent) :
    QThread(parent),
    _networkConnection(networkConnection){
}

void LiveViewProducer::run(){
    LOG_LIVEVIETHREAD_DEBUG << "\n++++++++++++++++++++++++++++++++++++++";
    LOG_LIVEVIETHREAD_DEBUG << "LiveViewProducer run\n" << _liveViewRequest;
    QByteArray ba("test");
    //for (char c : ba) {
    //    LOG_LIVEVIETHREAD_DEBUG << QString("0x%1").arg((int)c, 0, 16);
    //}

    QNetworkAccessManager liveViewManager;// = new QNetworkAccessManager;
    liveViewManager.setConfiguration(_networkConnection->getActiveConfiguration());
    QUrl localurl(_liveViewRequest);
    QNetworkRequest request(localurl);
    liveReply = liveViewManager.get(request);
    //QMetaObject::invokeMethod(liveReply,SLOT(onLiveViewManagerReadyRead()));
    connect(liveReply, SIGNAL(readyRead()), this, SLOT(onLiveViewManagerReadyRead()));
    emit bufferLoaded();
    QThread::exec();
}

void LiveViewProducer::onLiveViewManagerReadyRead(){

    //emit newByteReached();
   // LOG_LIVEVIETHREAD_DEBUG << "onLiveViewManagerReadyRead";
    //LOG_LIVEVIETHREAD_DEBUG << "liveReply->bytesAvailable(): "<< liveReply->bytesAvailable();
#ifdef _USESTREAM
    // TODO
#else
    //forever{
        qint64 replycount = liveReply->bytesAvailable();
        if(replycount > 0){
            LOG_LIVEVIETHREAD_DEBUG << "onLiveViewManagerReadyRead replycount : " << replycount;

            //LOG_LIVEVIETHREAD_DEBUG << liveReply->read(replycount);
            replyBuffer.append(liveReply->read(replycount));
            qint64 loadlength =0;
            if(replyBuffer.size() >= producerLoadSize){
                realLoadSize = replyBuffer.size();

                loadlength = ringBuffer->addData((void*)replyBuffer.data(),realLoadSize);
                LOG_LIVEVIETHREAD_DEBUG <<  "replyBuffer.size(): " << replyBuffer.size() << "loadlength: " << loadlength ;
                //LOG_LIVEVIETHREAD_DEBUG <<  "loadlength: " << loadlength ;

                replyBuffer.remove(0,loadlength);

                //LOG_LIVEVIETHREAD_DEBUG <<  "replyBuffer.size(): " << replyBuffer.size();
                if(realLoadSize > 0){
                    loadlength = ringBuffer->addData((void*)replyBuffer.data(),replyBuffer.size());
                    replyBuffer.remove(0,loadlength);
                    LOG_LIVEVIETHREAD_DEBUG <<  "replyBuffer.size() 2: " << replyBuffer.size();
                }

                //usedSpace.release();
            bufferIsNotEmpty.wakeAll();
            }//replyBuffersize
        }// replycount

    //}//forever

#endif
    //exit();
}


void LiveViewProducer::setLiveViewRequest(QString liveViewRequest){
    _liveViewRequest = liveViewRequest;
}







LiveViewConsumer::LiveViewConsumer(QObject *parent) :
    QThread(parent)
{
     liveViewStreamAlive = false;
     offset=0;
     starttime=0;
     endtime=0;
     //consumerP=0;

     // use it
     //delete[] buff;
}


void LiveViewConsumer::run(){
    LOG_LIVEVIETHREAD_DEBUG << "\n++++++++++++++++++++++++++++++++++++++";
    LOG_LIVEVIETHREAD_DEBUG << "LiveViewConsumer run";
    while(true){
        buildLiveView();
        msleep(sleepTime);
    }
}

void LiveViewConsumer::awakeConsumer(){
    if(!isRunning())
        start();
}

void LiveViewConsumer::sleepConsumer(){
    wait();
}

void LiveViewConsumer::buildLiveView(){
    //LOG_LIVEVIETHREAD_DEBUG << "\n++++++++++++++++++++++++++++++++++++++";
    //LOG_LIVEVIETHREAD_DEBUG << "buildLiveViewPic";
    //LOG_LIVEVIETHREAD_DEBUG<< "offset" << offset << "starttime: " << starttime << "endtime: " << endtime;
    QByteArray array;
#ifdef _USESTREAM
    // TODO ?
#else
    // TODO ?
#endif

#ifdef  __USE_STANDARD_HEADER
    int commonHeaderLength = 1 + 1 + 2 + 4;
    int payloadHeaderLength = 4 + 3 + 1 + 4 + 1 + 115;
    // Header = 136 Bytes;
    int paddingsize = 0;
    QByteArray commonHeader;
    QByteArray payloadHeader;
    if(!inputStream.isNull()){
#ifdef _USESTREAM
    inputStream.readRawData((char*)commonHeader,commonHeaderLength);
#else

    ringBuffer->getData(consumerP,commonHeaderLength);
    inputStream.setRawData(consumerP,consumerBufferLen);
    for(int i=0;i<commonHeaderLength;i++)
        commonHeader[i] = inputStream[i];
#endif
    if (commonHeader.isNull() || commonHeader.length() != commonHeaderLength) {
              LOG_LIVEVIETHREAD_DEBUG << "Cannot read stream for common header.";
              return;
          }
          if (commonHeader[0] != (char) 0xFF) {
              LOG_LIVEVIETHREAD_DEBUG << "Unexpected data format. (Start byte)";
              return;
          }
          if (commonHeader[1] != (char) 0x01) {
              LOG_LIVEVIETHREAD_DEBUG << "Unexpected data format. (Payload byte)";
              return;
          }


    for(int i=0;i<payloadHeaderLength;i++)
        payloadHeader[i] = inputStream[i+commonHeaderLength];
    if (payloadHeader.isNull() || payloadHeader.length() != payloadHeaderLength) {
        LOG_LIVEVIETHREAD_DEBUG << "Cannot read stream for payload header.";
        return;
    }
    if (payloadHeader[0] != (char) 0x24
            || payloadHeader[1] != (char) 0x35
            || payloadHeader[2] != (char) 0x68
            || payloadHeader[3] != (char) 0x79) {
        LOG_LIVEVIETHREAD_DEBUG << "Unexpected data format. (Start code)";
        return;
    }

    int jpegSize = bytesToInt(payloadHeader, 4, 3);
    int paddingSize = bytesToInt(payloadHeader, 7, 1);

     // Payload Data
     //byte[] jpegData = readBytes(mInputStream, jpegSize);
     //byte[] paddingData = readBytes(mInputStream, paddingSize);

    for(int i=0;i < jpegSize+paddingSize; i++){
        //array.append(inputStream.at(i));
        int y = i+commonHeaderLength+payloadHeaderLength;
        array[i] = inputStream[y];
    }
    emit publishLiveViewBytes(array);
    }

#else
    bool found = false;
    bool foundstart = false;
    bool foundend = false;
    int jpegSize = 0;//34048;
    int hjpegSize = 0;
    int paddingSize =0;
    starttime = 0;
    endtime = 0;

    //LOG_LIVEVIETHREAD_DEBUG << "consumerBufferLen: " << consumerBufferLen;
    //LOG_LIVEVIETHREAD_DEBUG << "ringBuffer->Free_Space():     " << ringBuffer->Free_Space();
    //LOG_LIVEVIETHREAD_DEBUG << "ringBuffer->Buffered_Bytes(): " <<ringBuffer->Buffered_Bytes();


    mutex.lock();
    bufferIsNotEmpty.wait(&mutex);
    //consumerP=new char[realLoadSize+1];
    char consumerP[realLoadSize+1];
    qint64 loadedBytes = ringBuffer->getData(consumerP,realLoadSize);
    LOG_LIVEVIETHREAD_DEBUG << "Consumer loadedBytes: " << loadedBytes;
    //inputStream.setRawData(consumerP,consumerBufferLen);
    if(loadedBytes >0){
        inputStream.append(consumerP,loadedBytes);
    }
    //delete[] consumerP;
    //consumerP =0;
    if(loadedBytes < realLoadSize){
        //consumerP=new char[realLoadSize-loadedBytes+1];
        char consumerP[realLoadSize-loadedBytes+1];
       loadedBytes = ringBuffer->getData(consumerP,realLoadSize-loadedBytes);
        LOG_LIVEVIETHREAD_DEBUG << "loadedBytes 2: " << loadedBytes;
        if(loadedBytes >0){
            inputStream.append(consumerP,loadedBytes);
        }
        //delete[] consumerP;
        //consumerP =0;
    }

    if(!inputStream.isEmpty()){
        //LOG_LIVEVIETHREAD_DEBUG << "!inputStream.isEmpty()" << inputStream.size();
        //! search in  tmp Buffer
        while (offset< (inputStream.length() -1)) {
            //! suche nach Bildanfang ab Stelle offset
            while(!foundstart && offset < (inputStream.length() -1) ){
                if (inputStream.at(offset) == (char) 0xFF && inputStream.at(offset+1) == (char) 0x01) {
                    LOG_LIVEVIETHREAD_DEBUG << "Start byte Found at " << offset;
                    if (inputStream.at(offset+8) == (char) 0x24
                            && inputStream.at(offset+9) == (char) 0x35
                            && inputStream.at(offset+10) == (char) 0x68
                            && inputStream.at(offset+11) == (char) 0x79) {
                            hjpegSize = bytesToInt(inputStream, 12, 3);
                            paddingSize = bytesToInt(inputStream, 15, 1);
                            LOG_LIVEVIETHREAD_DEBUG << "Payloadheader Found at " << offset+8;
                    }
                }
                // osx: -1,40 Android: 255,216
                if(inputStream.at(offset) == (char) 0xff  && inputStream.at(offset+1) == (char)0xd8){
                    starttime = offset;
                    foundstart = true;
                    LOG_LIVEVIETHREAD_DEBUG << "start Found";
                }
                offset++;
            }//while
            found = false;
            int arrayiter=0;
            //! suche nach Bildende ab Stelle offset
            while(foundstart && !foundend && offset<(inputStream.length()-1)){
                // osx: -1,39 Android: 255,217
                if(inputStream.at(offset) == (char)0xff && inputStream.at(offset+1) == (char)0xd9){
                    endtime = offset;
                    foundend = true;
                    LOG_LIVEVIETHREAD_DEBUG << "end Found";
                }
                array[arrayiter] = inputStream.at(offset-1);
                arrayiter++;
                offset++;
            }//while

            //! Bild gefunden
            if(foundstart && foundend){

                jpegSize = endtime-starttime;
                LOG_LIVEVIETHREAD_DEBUG << "Picture Found           " <<"\n"
                         << "Starttime:              " << starttime << "\n"
                         << "endtime:                " << endtime << "\n"
                         << "offset:                 " << offset << "\n"
                         << "inputStream.size():     " << inputStream.size() << "\n"
                         << "jpegSize calculated:    " << jpegSize << "\n"
                         << "jpegSize from Header:   " << hjpegSize << "\n"
                         << "paddingSize:            " << paddingSize << "\n"
                         << "jpegSize full size:     " << endtime;
                emit publishLiveViewBytes(array);
                found = false;
                foundend = false;
                foundstart = false;
                inputStream.remove(0,endtime);
                offset = 0;
                LOG_LIVEVIETHREAD_DEBUG << "inputStream new size:   " << inputStream.size() << "\n";
                //LOG_LIVEVIETHREAD_DEBUG << "publishLiveViewBytes";
            }
            //freeSpace.release();
        } // while search in tmp buffer;


    }// !inputStream.isEmpty()
    mutex.unlock();
   #endif

}

void LiveViewConsumer::setInputStream(QNetworkReply *reply ){
    static qint64 border=0;
    _reply = reply;
    qint64 maxlen = _reply->bytesAvailable();
    //LOG_LIVEVIETHREAD_DEBUG << "maxlen before: " << maxlen;
    char *bytearrayP=0;
    bytearrayP = new char[maxlen];
    //liveReply->readAll();
    _reply->read(bytearrayP,maxlen);
#ifdef _USESTREAM
    inputStream.writeBytes(bytearrayP,maxlen);
#else
    //inputStream = reply->readAll();
    //liveReply->open(QIODevice::WriteOnly);
    //if(ringBuffer->Buffered_Bytes()<gRingBufferSize)

    ringBuffer->addData((void*)bytearrayP,maxlen);

    border+=maxlen;
    if (border >= gRingBufferSize){
    //LOG_LIVEVIETHREAD_DEBUG << "maxlen after: " << maxlen;
    //if(ringBuffer->Free_Space() < maxlen){
        if(!isRunning()){
            this->start();
        }
    }
    delete[] bytearrayP;

#endif
}
