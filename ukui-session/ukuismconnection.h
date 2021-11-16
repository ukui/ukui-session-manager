#ifndef UKUISMCONNECTION_H
#define UKUISMCONNECTION_H

#include <QSocketNotifier>
#include <X11/ICE/ICElib.h>


class UKUISMConnection : public QSocketNotifier
{
public:
    UKUISMConnection(IceConn conn) : QSocketNotifier(IceConnectionNumber(conn), QSocketNotifier::Read )
    {
        iceConn = conn;
    }

    IceConn iceConn;
};

class UKUISMListener : public QSocketNotifier
{
public:
    UKUISMListener(IceListenObj obj) : QSocketNotifier(IceGetListenConnectionNumber(obj), QSocketNotifier::Read)
    {
        listenObj = obj;
    }

    IceListenObj listenObj;
};

#endif // UKUISMCONNECTION_H
