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
unsigned int gRealLoadSize =0;
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
    static int counter =0;
    //emit newByteReached();
    //LOG_LIVEVIETHREAD_DEBUG << "onLiveViewManagerReadyRead" << counter;
    counter++;
    //LOG_LIVEVIETHREAD_DEBUG << "Producer liveReply->bytesAvailable(): "<< liveReply->bytesAvailable();
#ifdef _USESTREAM
    // TODO
#else
    //forever{
        // bytesAvailable are increased until read or readall is called!
        qint64 replycount = liveReply->bytesAvailable()-1;
        if(replycount > 0){

            qint64 loadlength =0;
            //if(replyBuffer.size() >= producerLoadSize){
            if(replycount > estJpegSize){
                //QByteArray tmpBuffer(liveReply->read(replycount));
                //QByteArray tmpBuffer;
                //tmpBuffer.fromRawData(liveReply->read(replycount),replycount);
                char tmpBuffer[replycount];
                char *tmpBufferP = tmpBuffer;
                memcpy(tmpBuffer,liveReply->read(replycount).data(),replycount);
                    LOG_LIVEVIETHREAD_DEBUG << "";

                    LOG_LIVEVIETHREAD_DEBUG << "Producer replycount:    " << replycount;
                loadlength = ringBuffer->addData((void*)tmpBuffer,replycount);
                gRealLoadSize = loadlength;
                    LOG_LIVEVIETHREAD_DEBUG << "Producer loadlength:    " << loadlength;

                if(loadlength<replycount-1){
                    tmpBufferP + loadlength;
                    loadlength = ringBuffer->addData((void*)tmpBufferP,replycount-loadlength);
                    LOG_LIVEVIETHREAD_DEBUG << "Producer loadlength 2:  " << loadlength;
                    //LOG_LIVEVIETHREAD_DEBUG <<  "replyBuffer.size() 2: " << replycount-loadlength;
                    gRealLoadSize += loadlength;
                }
                //usedSpace.release();
            bufferIsNotEmpty.wakeAll();
            //}//replyBuffersize
            }
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
     foundstartOffset =0;
     arrayiter=0;
     //consumerP=0;
     foundstart = false;
     foundend = false;

     jpegSize = 0;//34048;
     hjpegSize = 0;
     paddingSize =0;
     carry = false;

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

#ifdef  __USE_STANDARD_HEADER
    int commonHeaderLength = 1 + 1 + 2 + 4;
    int payloadHeaderLength = 4 + 3 + 1 + 4 + 1 + 115;
    // Header = 136 Bytes;
    int paddingsize = 0;
    QByteArray commonHeader;
    QByteArray payloadHeader;
    jpegSize = 0;
    offset = 0;
    mutex.lock();
    bufferIsNotEmpty.wait(&mutex);
    char consumerP[gRealLoadSize];
    qint64 loadedBytes = ringBuffer->getData(consumerP,gRealLoadSize);
    LOG_LIVEVIETHREAD_DEBUG << "";
    LOG_LIVEVIETHREAD_DEBUG << "Consumer loadedBytes:   " << loadedBytes;
    QByteArray tmpArray;
    tmpArray.resize(loadedBytes);
    tmpArray.setRawData(consumerP,loadedBytes);
    if(loadedBytes >0){
        inputStream.append(tmpArray,loadedBytes);
    }

    if(loadedBytes < gRealLoadSize){
        char consumerP[gRealLoadSize-loadedBytes];
        loadedBytes = ringBuffer->getData(consumerP,gRealLoadSize-loadedBytes);
    LOG_LIVEVIETHREAD_DEBUG << "loadedBytes 2:          " << loadedBytes;
        if(loadedBytes >0){
            tmpArray.resize(loadedBytes);
            tmpArray.setRawData(consumerP,loadedBytes);
            inputStream.append(tmpArray);
        }
    }
    LOG_LIVEVIETHREAD_DEBUG << "inputStream.size() 1:   " << inputStream.size() << "\n";

    if(!inputStream.isNull()){
        //int looplength = inputStream.length() -1;
         while (offset < inputStream.length() -15) {
             if (inputStream.at(offset) == (char) 0xFF) {
                        //LOG_LIVEVIETHREAD_DEBUG << "Start byte Found at:    " << offset;
                 switch (inputStream.at(offset+1)) {
                    case (char) 0x12:
                        LOG_LIVEVIETHREAD_DEBUG << "Informati Header at:    " << offset+1;
                        break;
                    case (char) 0x01:
                        LOG_LIVEVIETHREAD_DEBUG << "Start byte Found at:    " << offset;
                        //offset+=8;
                        break;
                    case (char) 0x11:
                        LOG_LIVEVIETHREAD_DEBUG << "Payload           at:    " << offset+1;
                        //offset+=8;
                        break;
                    default:
                        break;
                    }
                 if (inputStream.at(offset+8) == (char) 0x24
                         && inputStream.at(offset+9) == (char) 0x35
                         && inputStream.at(offset+10) == (char) 0x68
                         && inputStream.at(offset+11) == (char) 0x79) {
                         hjpegSize = bytesToInt(inputStream, offset+12, 3);
                         paddingSize = bytesToInt(inputStream, offset+15, 1);
                         LOG_LIVEVIETHREAD_DEBUG << "Payloadheader Found at: " << offset+8;

                         offset+=8;
                         offset+=128;
                 }
             }
             offset++;
             //LOG_LIVEVIETHREAD_DEBUG << "offset:                 " << offset;
             if(hjpegSize > 0 && hjpegSize < (inputStream.size() - offset)){
                LOG_LIVEVIETHREAD_DEBUG << "hjpegSize:              " << hjpegSize;
                int arraysize = 0;
                 for (; arraysize < hjpegSize; arraysize++,offset++){
                     array[arraysize] = inputStream.at(offset-1);
                 }

                 emit publishLiveViewBytes(array);
                 inputStream.remove(0,offset-1);
                 LOG_LIVEVIETHREAD_DEBUG << "publishLiveViewBytes:   " << arraysize;
                 LOG_LIVEVIETHREAD_DEBUG << "inputStream.size()  :   " << inputStream.size() << "\n";
                 hjpegSize = 0;
                 offset = 0;
             }

         }//while
    }// if not empty
    mutex.unlock();

#else

    //bool foundstart = false;
    //bool foundend = false;

    //starttime=0;
    //endtime=0;
    //foundstartOffset =0;


    //LOG_LIVEVIETHREAD_DEBUG << "consumerBufferLen: " << consumerBufferLen;
    //LOG_LIVEVIETHREAD_DEBUG << "ringBuffer->Free_Space():     " << ringBuffer->Free_Space();
    //LOG_LIVEVIETHREAD_DEBUG << "ringBuffer->Buffered_Bytes(): " <<ringBuffer->Buffered_Bytes();


    mutex.lock();
    bufferIsNotEmpty.wait(&mutex);
    char consumerP[gRealLoadSize];
    qint64 loadedBytes = ringBuffer->getData(consumerP,gRealLoadSize);
    LOG_LIVEVIETHREAD_DEBUG << "Consumer loadedBytes: " << loadedBytes;
    QByteArray tmpArray;
    tmpArray.resize(loadedBytes);
    tmpArray.setRawData(consumerP,loadedBytes);
    if(loadedBytes >0){
        inputStream.append(tmpArray,loadedBytes);
    }

    if(loadedBytes < gRealLoadSize){
        char consumerP[gRealLoadSize-loadedBytes];
       loadedBytes = ringBuffer->getData(consumerP,gRealLoadSize-loadedBytes);
        LOG_LIVEVIETHREAD_DEBUG << "loadedBytes 2: " << loadedBytes;
        if(loadedBytes >0){
            tmpArray.resize(loadedBytes);
            tmpArray.setRawData(consumerP,loadedBytes);
            inputStream.append(tmpArray);
        }
    }
    LOG_LIVEVIETHREAD_DEBUG     << "inputStream.size() 1:   " << inputStream.size() << "\n";
    if(!inputStream.isEmpty()){
        //LOG_LIVEVIETHREAD_DEBUG << "!inputStream.isEmpty()" << inputStream.size();
        //! search in Buffer, perhaps we have more than one image
        while (offset < (inputStream.length() -1)) {
            //! suche nach Bildanfang ab Stelle offset
            while(!foundstart && offset < (inputStream.length() -1) ){
                if (inputStream.at(offset) == (char) 0xFF) {
                    LOG_LIVEVIETHREAD_DEBUG << "Start byte Found at:    " << offset;
                    switch (inputStream.at(offset+1)) {
                    case (char) 0x12:
                        LOG_LIVEVIETHREAD_DEBUG << "Information Header at:    " << offset+1;
                        break;
                    case (char) 0x01:
                        LOG_LIVEVIETHREAD_DEBUG << "Nothing            at:    " << offset+1;
                        break;
                    case (char) 0x11:
                        LOG_LIVEVIETHREAD_DEBUG << "Payload             at:    " << offset+1;
                        break;
                    default:
                        break;
                    }

                    if (inputStream.at(offset+8) == (char) 0x24
                            && inputStream.at(offset+9) == (char) 0x35
                            && inputStream.at(offset+10) == (char) 0x68
                            && inputStream.at(offset+11) == (char) 0x79) {
                            //char calculatejpeg[3];

                            hjpegSize = bytesToInt(inputStream, offset+12, 3);
                            paddingSize = bytesToInt(inputStream, offset+15, 1);
                            LOG_LIVEVIETHREAD_DEBUG << "Payloadheader Found at: " << offset+8;
                    }
                }

                // osx: -1,40 Android: 255,216
                if(inputStream.at(offset) == (char) 0xff  && inputStream.at(offset+1) == (char)0xd8){
                    starttime = offset;
                    foundstart = true;
                    LOG_LIVEVIETHREAD_DEBUG << "start Found at:         " << starttime << "\n";
                    foundstartOffset = offset;
                }

                offset++;

            }//while


            LOG_LIVEVIETHREAD_DEBUG << "actual arrayiter:       " << arrayiter;
            //! suche nach Bildende ab Stelle offset
            while(foundstart && !foundend && offset<(inputStream.length()-1)){
                // osx: -1,39 Android: 255,217
                if(inputStream.at(offset) == (char)0xff && inputStream.at(offset+1) == (char)0xd9){
                    endtime = offset;
                    foundend = true;
                    LOG_LIVEVIETHREAD_DEBUG << "end Found at:           " << endtime;
                }
                array[arrayiter] = inputStream.at(offset-1);
                arrayiter++;
                offset++;
            }//while
            /*
            for (int i = 0; i< hjpegSize;i++){
                array[i] = inputStream.at(foundstartOffset);
                foundstartOffset++;
            }*/

            //! Bild gefunden
            if(foundstart && foundend){
                if(carry){
                    jpegSize = endtime-starttime;
                }
                else
                    jpegSize = endtime-starttime;
                LOG_LIVEVIETHREAD_DEBUG << "Picture Found           " <<"\n"
                         << "arrayiter:              " << arrayiter << "\n"
                         << "Starttime:              " << starttime << "\n"
                         << "endtime:                " << endtime << "\n"
                         << "offset:                 " << offset << "\n"
                         << "inputStream.size():     " << inputStream.size() << "\n"
                         << "jpegSize calculated:    " << jpegSize << "\n"
                         << "jpegSize from Header:   " << hjpegSize << "\n"
                         << "paddingSize:            " << paddingSize << "\n"
                         << "jpegSize full size:     " << endtime << "\n"
                         << "inputStream.size():     " << inputStream.size() << "\n";
                emit publishLiveViewBytes(array);

                inputStream.remove(0,endtime);
                offset = 0;
                arrayiter = 0;
                foundend = false;
                foundstart = false;
                starttime = 0;
                endtime = 0;
                carry = false;
                LOG_LIVEVIETHREAD_DEBUG << "inputStream new size:   " << inputStream.size() << "\n";
                //LOG_LIVEVIETHREAD_DEBUG << "publishLiveViewBytes";
            }
            else if (foundstart && !foundend){
                carry = true;
                //arrayiter = foundstartOffset;
                //offset = 0;
                LOG_LIVEVIETHREAD_DEBUG << "arrayiter               " << arrayiter <<"\n";
            }
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
