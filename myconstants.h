#ifndef MYSONSTANTS_H
#define MYSONSTANTS_H

const unsigned int bufferLoadCount = 3;
// The Size of the Ringbuffer
const unsigned int estJpegSize = 34816;
//const unsigned int estJpegSize = 36000;
//const unsigned int  gRingBufferSize = 1024 * 34*bufferLoadCount;
const unsigned int  gRingBufferSize = estJpegSize*bufferLoadCount;

const  qint64 consumerBufferLen = gRingBufferSize - estJpegSize;
const  qint64 producerLoadSize = gRingBufferSize - estJpegSize;

// The datasize of the Producer
//const unsigned int dataSize = gRingBufferSize*3;

const unsigned int sleepTime = 20;
//#define _USESTREAM
#define _USEPRODUCERTHREAD
#define __USE_STANDARD_HEADER

#endif // MYSONSTANTS_H
