#include "liveviewthread.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
//#include "ringbuffer.h"
#include "networkconnection.h"


#include <stdio.h>
#include <stdlib.h>

QSemaphore freeSpace(1);
QSemaphore usedSpace;

//const int DataSize = 100000;
//const int BufferSize = 8192;
//char buffer[BufferSize];


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
    qDebug() << "\n++++++++++++++++++++++++++++++++++++++";
    qDebug() << "LiveViewProducer run\n" << _liveViewRequest;
    QByteArray ba("test");
    //for (char c : ba) {
    //    qDebug() << QString("0x%1").arg((int)c, 0, 16);
    //}

    liveViewManager = new QNetworkAccessManager;
    liveViewManager->setConfiguration(_networkConnection->getActiveConfiguration());
    QUrl localurl(_liveViewRequest);
    QNetworkRequest request(localurl);
    liveReply = liveViewManager->get(request);
    connect(liveReply, SIGNAL(readyRead()), this, SLOT(onLiveViewManagerReadyRead()));
    //freeSpace.acquire();
    exec();
}

void LiveViewProducer::onLiveViewManagerReadyRead(){
    //qDebug() << "onLiveViewManagerReadyRead";
    //qDebug() << "liveReply->bytesAvailable(): "<< liveReply->bytesAvailable();
#ifdef _USESTREAM
    // TODO
#else

    qint64 replycount = liveReply->bytesAvailable();
    //qDebug() << "replycount before: " << replycount;
    if(replycount > 0){
        //qDebug() << liveReply->read(replycount);
        replyBuffer.append(liveReply->read(replycount));
        if(replyBuffer.size() >= producerLoadSize){
            freeSpace.acquire();
            qint64 loadlength = ringBuffer->addData((void*)replyBuffer.data(),replyBuffer.size());
            qDebug() <<  "replyBuffer.size(): " << replyBuffer.size() << "loadlength: " << loadlength ;
            replyBuffer.remove(0,loadlength);

            qDebug() <<  "replyBuffer.size(): " << replyBuffer.size();
            if(replyBuffer.size() > 0){
                loadlength = ringBuffer->addData((void*)replyBuffer.data(),replyBuffer.size());
                replyBuffer.remove(0,loadlength);
                qDebug() <<  "replyBuffer.size() 2: " << replyBuffer.size();
            }

            usedSpace.release();
            emit bufferLoaded();
        }
    }
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
     consumerP=0;
     consumerP=new char[consumerBufferLen+1];
     // use it
     //delete[] buff;
}


void LiveViewConsumer::run(){
    qDebug() << "\n++++++++++++++++++++++++++++++++++++++";
    qDebug() << "LiveViewConsumer run";
    while(true){
        buildLiveView();
        msleep(sleepTime);
    }
}

void LiveViewConsumer::buildLiveView(){
    //qDebug() << "\n++++++++++++++++++++++++++++++++++++++";
    //qDebug() << "buildLiveViewPic";
    //qDebug()<< "offset" << offset << "starttime: " << starttime << "endtime: " << endtime;
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
              qDebug() << "Cannot read stream for common header.";
              return;
          }
          if (commonHeader[0] != (char) 0xFF) {
              qDebug() << "Unexpected data format. (Start byte)";
              return;
          }
          if (commonHeader[1] != (char) 0x01) {
              qDebug() << "Unexpected data format. (Payload byte)";
              return;
          }


    for(int i=0;i<payloadHeaderLength;i++)
        payloadHeader[i] = inputStream[i+commonHeaderLength];
    if (payloadHeader.isNull() || payloadHeader.length() != payloadHeaderLength) {
        qDebug() << "Cannot read stream for payload header.";
        return;
    }
    if (payloadHeader[0] != (char) 0x24
            || payloadHeader[1] != (char) 0x35
            || payloadHeader[2] != (char) 0x68
            || payloadHeader[3] != (char) 0x79) {
        qDebug() << "Unexpected data format. (Start code)";
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

    qDebug() << "consumerBufferLen: " << consumerBufferLen;
    //qDebug() << "ringBuffer->Free_Space():     " << ringBuffer->Free_Space();
    //qDebug() << "ringBuffer->Buffered_Bytes(): " <<ringBuffer->Buffered_Bytes();

    usedSpace.acquire();
    //usedSpace.acquire(ringBuffer->Buffered_Bytes());
    qint64 loadedBytes = ringBuffer->getData(consumerP,consumerBufferLen);
    qDebug() << "loadedBytes: " << loadedBytes;
    //inputStream.setRawData(consumerP,consumerBufferLen);
    inputStream.append(consumerP,loadedBytes);
    if(loadedBytes < consumerBufferLen){
       loadedBytes = ringBuffer->getData(consumerP,consumerBufferLen-loadedBytes);
        inputStream.append(consumerP,loadedBytes);

    }

    if(!inputStream.isEmpty()){
        //qDebug() << "!inputStream.isEmpty()" << inputStream.size();
        //! search in  tmp Buffer
        while (offset< (inputStream.length() -1)) {
            //! suche nach Bildanfang ab Stelle offset
            while(!foundstart && offset< (inputStream.length() -1) ){
                if (inputStream.at(offset) == (char) 0xFF && inputStream.at(offset+1) == (char) 0x01) {
                    qDebug() << "Start byte Found at " << offset;
                    if (inputStream.at(offset+8) == (char) 0x24
                            && inputStream.at(offset+9) == (char) 0x35
                            && inputStream.at(offset+10) == (char) 0x68
                            && inputStream.at(offset+11) == (char) 0x79) {
                            hjpegSize = bytesToInt(inputStream, 12, 3);
                            paddingSize = bytesToInt(inputStream, 15, 1);
                            qDebug() << "Payloadheader Found at " << offset+8;
                    }
                }




    #ifdef Q_OS_ANDROID
                if(inputStream.at(offset) == 255  && inputStream.at(offset+1) == 216){
    #else
                if(inputStream.at(offset) == -1  && inputStream.at(offset+1) == -40){
    #endif
                    starttime = offset;
                    //found = true;
                    foundstart = true;
                }
                offset++;
            }//while
            found = false;
            int arrayiter=0;
            //! suche nach Bildende ab Stelle offset
            while(foundstart && !foundend && offset<(inputStream.length()-1)){

    #ifdef Q_OS_ANDROID
                if(inputStream.at(offset) == 255 && inputStream.at(offset+1) == 217){
    #else
                if(inputStream.at(offset) == -1 && inputStream.at(offset+1) == -39){
    #endif
                    endtime = offset;
                    foundend = true;
                }
                array[arrayiter] = inputStream.at(offset-1);
                arrayiter++;
                offset++;
            }//while

            //! Bild gefunden
            if(foundstart && foundend){
                jpegSize = endtime-starttime;
                qDebug() << "Picture Found           " <<"\n"
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
                qDebug() << "inputStream new size:   " << inputStream.size() << "\n";
                //qDebug() << "publishLiveViewBytes";
            }
            /*
            if(offset >= (inputStream.size()-1)){
                //inputStream.clear();
                offset = 0;
                //starttime = 0;
                //endtime = 0;
                //qDebug() << "Buffer Cleared";
            }*/
            freeSpace.release();
        } // while search in tmp buffer;


    }// !inputStream.isEmpty()
   #endif

}

void LiveViewConsumer::setInputStream(QNetworkReply *reply ){
    static qint64 border=0;
    _reply = reply;
    qint64 maxlen = _reply->bytesAvailable();
    //qDebug() << "maxlen before: " << maxlen;
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
    //qDebug() << "maxlen after: " << maxlen;
    //if(ringBuffer->Free_Space() < maxlen){
        if(!isRunning()){
            this->start();
        }
    }
    delete[] bytearrayP;

#endif
}
