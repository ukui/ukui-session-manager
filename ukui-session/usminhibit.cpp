#include "usminhibit.h"
#include "QDBusConnection"
#include <QRandomGenerator> 
#include <limits.h>

inhibit::inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags ,quint32 cookie ,QString inhibitorName){
    this->app_id = app_id;
    this->toplevel_xid = toplevel_xid;
    this->reason = reason;
    this->flags = flags;
    this->cookie = cookie;
    this->inhibitorName = inhibitorName;
}

inhibit::~inhibit(){}

QString inhibit::get_app_id(){
    return this->app_id;
}
quint32 inhibit::get_toplevel_xid(){
    return this->toplevel_xid;
}
QString inhibit::get_reason(){
    return this->reason;
}
quint32 inhibit::get_flags(){
    return this->flags;
}
quint32 inhibit::get_cookie(){
    return this->cookie;
}
QString inhibit::get_inhibitorName(){
    return this->inhibitorName;
}

uint usminhibit::get_next_inhibitor_serial()
{
        uint serial;

        serial = inhibitor_serial++;

        if ((int32_t)inhibitor_serial < 0) {
                inhibitor_serial = 1;
        }

        return serial;
}

usminhibit::usminhibit()
{
//    dbus = new QDBusInterface("org.gnome.SessionManager",
//                              "/org/gnome/SessionManager",
//                              "org.gnome.SessionManager",
//                              QDBusConnection::sessionBus());
    inhibitor_serial = 1;//默认inhibitor序号从1开始
}

usminhibit::~usminhibit(){}

quint32 usminhibit::addinhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags){
    if(app_id.isEmpty()){
        return -1;
    }
    if(reason.isEmpty()){
        return -1;
    }
    if(flags == 0){
        return -1;
    }
    quint32 cookie = generate_cookie();
    QString inhibitorName = "/org/gnome/SessionManager/inhibitor" + QString::number(get_next_inhibitor_serial());
    inhibit a = inhibit(app_id,toplevel_xid,reason,flags,cookie,inhibitorName);
    hash.insert(cookie,a);
    return cookie;
}

uint usminhibit::generate_cookie(){
    quint32 cookie;
    do{
        cookie = QRandomGenerator::bounded(0,1000);//std::numeric_limits<quint32>::max()
    }while (!hash.contains(cookie)) ;
    return cookie;
}

bool usminhibit::uninhibit(quint32 cookie){
    QHashIterator<quint32, inhibit> i(hash);
    while (i.hasNext()) {
        i.next();
        if(i.key() == cookie){
            hash.erase(i);
            return true;
        }
    }
    return false;
}

QStringList usminhibit::getinhibitor(){
    QHashIterator<quint32, inhibit> i(hash);
    QStringList inhibitors;
    while (i.hasNext()) {
        QString name = i.value().get_inhibitorName();
        inhibitors<<name;
    }
    return inhibitors;
}


#include "usminhibit.moc"



