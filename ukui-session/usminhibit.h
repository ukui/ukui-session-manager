#ifndef USMINHIBIT_H
#define USMINHIBIT_H

#include <QDBusInterface>
#include <QHash>

class inhibit{
public:
    inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags ,quint32 cookie ,QString inhibitorName);
    ~inhibit();

    QString app_id;
    quint32 toplevel_xid;
    QString reason;
    quint32 flags;
    quint32 cookie;
    QString inhibitorName;
};

class usminhibit
{
    //Q_OBJECT
public:
//    enum InhibitorFlag{
//        GSM_INHIBITOR_FLAG_LOGOUT      = 1 << 0,
//        GSM_INHIBITOR_FLAG_SWITCH_USER = 1 << 1,
//        GSM_INHIBITOR_FLAG_SUSPEND     = 1 << 2,
//        GSM_INHIBITOR_FLAG_IDLE        = 1 << 3
//    }
    usminhibit();

    int inhibitor_serial;
    ~usminhibit();
    uint generate_cookie();
    QHash<quint32 , inhibit> hash;

    quint32 addinhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags);
    uint uninhibit(quint32 cookie);
    QStringList getinhibitor();

private:
    //QDBusInterface dbus;
    uint get_next_inhibitor_serial();
};

#endif // USMINHIBIT_H
