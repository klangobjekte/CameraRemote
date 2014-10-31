#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <QObject>

class RingBuffer : public QObject
{
    Q_OBJECT
public:
    //explicit RingBuffer(QObject *parent = 0);
   explicit  RingBuffer( unsigned int buffSize,QObject *parent = 0 );
    qint64 addData(const void* data,  qint64 len );
    qint64 getData(void* outData, qint64 len );
    void skipData( unsigned int& len );
    // The amount of data the buffer can currently receive on one addData() call.
    inline unsigned int Free_Space()    { return (unsigned int)_bufferSize; }

    // The total amount of data in the buffer. Note that it may not be continuous: you may need
    // two successive calls to getData() to get it all.
    inline unsigned int Buffered_Bytes() { return (unsigned int)_dataInBuffer; }
signals:

public slots:

private:
    void updateState();
    char *_producePointer;
    char *_consumePointer;
    char *_buffEnd;
    char *_buffer;
    int _bufferSize, _maxConsume, _dataInBuffer;



};

#endif // RINGBUFFER_H
