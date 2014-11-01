#include "ringbuffer.h"
#include "cameraremotedefinitions.h"
#include <QDebug>


//#define LOG_RINGBUFFER
#ifdef LOG_RINGBUFFER
#   define LOG_RINGBUFFER_DEBUG qDebug()
#else
#   define LOG_RINGBUFFER_DEBUG nullDebug()
#endif


RingBuffer::RingBuffer( qint64 buffSize, QObject *parent) :
    QObject(parent)
{
    //_buffer = (unsigned char*)buffer;
    _buffer = 0;
    _buffer = new char[buffSize];
    _producePointer = _consumePointer = _buffer;
    _bufferSize = buffSize;
    _buffEnd = _buffer + buffSize;
    _maxConsume = _dataInBuffer = 0;
    updateState();
}

// Try to add data to the buffer. After the call, 'len' contains
// the amount of bytes actually buffered.
qint64 RingBuffer::addData(const void* data,  qint64 len )
{

    qint64 _len = len;
    if ( _len > (qint64)_bufferSize ){
        _len = (qint64)_bufferSize;
    }
    LOG_RINGBUFFER_DEBUG << "addData " << _len;
    memcpy( _producePointer, data, (size_t)_len );
    //memmove(_producePointer, data, (size_t)_len );
    _producePointer += _len;
    _dataInBuffer += _len;
    updateState();
    return _len;
}

// Request 'len' bytes from the buffer. After the call,
// 'len' contains the amount of bytes actually copied.
qint64 RingBuffer::getData( void* outData, qint64 len )
{
    qint64 _len = len;
    if ( len > (qint64)_maxConsume ){
        _len = (qint64)_maxConsume;
    }
    LOG_RINGBUFFER_DEBUG << "getData " << _len;

    memcpy( outData, _consumePointer, _len );
    //memmove( outData, _consumePointer, _len );
    _consumePointer += _len;
    _dataInBuffer -= _len;
    updateState();
    return _len;
}

// Tries to skip len bytes. After the call,
// 'len' contains the realized skip.
void RingBuffer::skipData( qint64& len )
{
    qint64 requestedSkip = len;
    for ( int i=0; i<2; ++i ) // This may wrap  so try it twice
    {
        int skip = (int)len;
        if ( skip > _maxConsume )
            skip = _maxConsume;
        _consumePointer += skip;
        _dataInBuffer -= skip;
        len -= skip;
        updateState();
    }
    len = requestedSkip - len;
}

void RingBuffer::updateState()
{
    if (_consumePointer == _buffEnd){
        LOG_RINGBUFFER_DEBUG << "_consumePointer == _buffEnd";
        _consumePointer = &_buffer[0];}

    if (_producePointer == _buffEnd){
        LOG_RINGBUFFER_DEBUG << "_producePointer == _buffEnd";
        _producePointer = &_buffer[0];}

    if (_producePointer == _consumePointer){
        LOG_RINGBUFFER_DEBUG << "_producePointer == _consumePointer";
        if ( _dataInBuffer > 0 ){
            _bufferSize = 0;
            _maxConsume = _buffEnd - _consumePointer;
        }
        else{
            _bufferSize = _buffEnd - _producePointer;
            _maxConsume = 0;
        }
    }
    else if ( _producePointer > _consumePointer ){
        _bufferSize = _buffEnd - _producePointer;
        _maxConsume = _producePointer - _consumePointer;
    }
    else{
        _bufferSize = _consumePointer - _producePointer;
        _maxConsume = _buffEnd - _consumePointer;
    }
}
