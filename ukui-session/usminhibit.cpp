/*****************************************************************
ukuismserver - the UKUI session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>
Copyright 2021 KylinSoft Co., Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "usminhibit.h"
#include "QDBusConnection"
#include <QRandomGenerator>
#include <limits.h>
#include <QDebug>

inhibit::inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags ,quint32 cookie ,QString inhibitorName)
{
    this->app_id = app_id;
    this->toplevel_xid = toplevel_xid;
    this->reason = reason;
    this->flags = flags;
    this->cookie = cookie;
    this->inhibitorName = inhibitorName;
}

inhibit::~inhibit()
{
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

    inhibit_logout_num = 0;
    inhibit_switchuser_num = 0;
    inhibit_suspend_num = 0;
    inhibit_idle_num = 0;
}

usminhibit::~usminhibit()
{
}

bool usminhibit::isInhibited(quint32 flags)
{
    bool isinhib = false;
    if ((flags & GSM_INHIBITOR_FLAG_LOGOUT) == GSM_INHIBITOR_FLAG_LOGOUT) {
        if (inhibit_logout_num > 0)
            isinhib = true;
    }
    if ((flags & GSM_INHIBITOR_FLAG_SWITCH_USER) == GSM_INHIBITOR_FLAG_SWITCH_USER) {
        if (inhibit_switchuser_num > 0)
            isinhib = true;
    }
    if ((flags & GSM_INHIBITOR_FLAG_SUSPEND) == GSM_INHIBITOR_FLAG_SUSPEND) {
        if (inhibit_suspend_num > 0)
            isinhib = true;
    }
    if ((flags & GSM_INHIBITOR_FLAG_IDLE) == GSM_INHIBITOR_FLAG_IDLE) {
        if (inhibit_idle_num > 0)
            isinhib = true;
    }
    return isinhib;
}

quint32 usminhibit::addInhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags)
{
    if (app_id.isEmpty()) {
        return -1;
    }
    if (reason.isEmpty()) {
        return -1;
    }
    if (flags == 0) {
        return -1;
    }

    bool flag = false;
    if ((flags & GSM_INHIBITOR_FLAG_LOGOUT) == GSM_INHIBITOR_FLAG_LOGOUT) {
        inhibit_logout_num++;
        flag = true;
    }
    if ((flags & GSM_INHIBITOR_FLAG_SWITCH_USER) == GSM_INHIBITOR_FLAG_SWITCH_USER) {
        inhibit_switchuser_num++;
        flag = true;
    }
    if ((flags & GSM_INHIBITOR_FLAG_SUSPEND) == GSM_INHIBITOR_FLAG_SUSPEND) {
        inhibit_suspend_num++;
        flag = true;
    }
    if ((flags & GSM_INHIBITOR_FLAG_IDLE) == GSM_INHIBITOR_FLAG_IDLE) {
        inhibit_idle_num++;
        flag = true;
    }
    if (flag == false) {
        return -1;
    }

    quint32 cookie = generateCookie();
    QString inhibitorName = "/org/gnome/SessionManager/inhibitor" + QString::number(get_next_inhibitor_serial());
    inhibit a(app_id, toplevel_xid, reason, flags, cookie, inhibitorName);
    hash.insert(cookie, a);
    qDebug() << "app_id=" << app_id << "; toplevel_xid=" << QString::number(toplevel_xid) << "; reason=" << reason << "; flag=" << QString::number(flags);
    return cookie;
}

uint usminhibit::generateCookie()
{
    quint32 cookie;
    do {
        cookie = QRandomGenerator::global()->bounded(1, 4294967295);//std::numeric_limits<quint32>::max()
    } while (hash.contains(cookie)) ;
    return cookie;
}

uint usminhibit::unInhibit(quint32 cookie)
{
    uint flags = 0;
    QHash<quint32, inhibit>::iterator i = hash.find(cookie);
    while (i != hash.end() && i.key() == cookie) {
        flags = i->flags;
        i = hash.erase(i);
    }
    if (flags == 0) {
        return flags;
    }
    if ((flags & GSM_INHIBITOR_FLAG_LOGOUT) == GSM_INHIBITOR_FLAG_LOGOUT) {
        inhibit_logout_num--;
    }
    if ((flags & GSM_INHIBITOR_FLAG_SWITCH_USER) == GSM_INHIBITOR_FLAG_SWITCH_USER) {
        inhibit_switchuser_num--;
    }
    if ((flags & GSM_INHIBITOR_FLAG_SUSPEND) == GSM_INHIBITOR_FLAG_SUSPEND) {
        inhibit_suspend_num--;
    }
    if ((flags & GSM_INHIBITOR_FLAG_IDLE) == GSM_INHIBITOR_FLAG_IDLE) {
        inhibit_idle_num--;
    }
    emit inhibitRemove();
    return flags;
}

QStringList usminhibit::getInhibitor()
{
    //do not show inhibitorName to user
    //in case we don't know who is inhibiting
    QHashIterator<quint32, inhibit> i(hash);
    QStringList inhibitors;
    while (i.hasNext()) {
        i.next();
        QString messagelist;
        messagelist = i.value().app_id + "/" + QString::number(i.value().toplevel_xid) + "/"
                    + i.value().reason + "/" + QString::number(i.value().flags) + "/" + QString::number(i.value().cookie);
        inhibitors << messagelist;
    }
    return inhibitors;
}


#include "usminhibit.moc"



