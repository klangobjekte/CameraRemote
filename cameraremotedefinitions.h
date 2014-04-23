#ifndef CAMERAREMOTEDEFINITIONS_H
#define CAMERAREMOTEDEFINITIONS_H

class NullDebug
{
public:
    template <typename T>
    NullDebug& operator<<(const T&) { return *this; }
};

inline NullDebug nullDebug() { return NullDebug(); }

//static QString qsspath = ":/qss/";
//static QString qsspath = ":/";
//static QString qsspath = "./qss/";

enum _CONNECTIONSTATE{
    _CONNECTIONSTATE_ERROR,
    _CONNECTIONSTATE_DISCONNECTED,
    _CONNECTIONSTATE_CONNECTET,
    _CONNECTIONSTATE_CONNECTING,
    _CONNECTIONSTATE_WATING,
    _CONNECTIONSTATE_SSDP_ALIVE_RECEIVED,
    _CONNECTIONSTATE_SSDP_BYEBYE_RECEIVED,
    _CONNECTIONSTATE_CAMERA_DETECTED
};


#endif // CAMERAREMOTEDEFINITIONS_H
