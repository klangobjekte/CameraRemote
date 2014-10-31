#ifndef COMMON_H
#define COMMON_H
#include <QSemaphore>
#include "myconstants.h"
#include "ringbuffer.h"

extern int bytesToInt(QByteArray byteData, int startIndex, int count);
extern RingBuffer *ringBuffer;
extern const unsigned int gRingBufferSize;
extern const unsigned int  bufferLCount;
extern QSemaphore freeSpace;
extern QSemaphore usedSpace;

#endif // COMMON_H
