/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 * Copyright: 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *            2012 Razor team
 * Authors:
 *   Christian Surlykke <christian@surlykke.dk>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */
#include "idlewatcher.h"

#include <KIdleTime>
#include <QDebug>
#include <QDBusReply>

IdleWatcher::IdleWatcher(int idle, int power ,QObject *parent) :
    QObject(parent),
    mSecsidle(idle),
    mSecspower(power)
{
    connect(KIdleTime::instance(),
            &KIdleTime::resumingFromIdle,
            this,
            &IdleWatcher::resumingFromIdle);
//    connect(KIdleTime::instance(),
//            static_cast<void (KIdleTime::*)(int)>(&KIdleTime::timeoutReached),
//            this,
//            &IdleWatcher::timeoutReached);
    connect(KIdleTime::instance(), SIGNAL(timeoutReached(int,int)),
            this, SLOT(timeoutReached(int,int)));

    setup();

    interface = new QDBusInterface(
                "org.gnome.SessionManager",
                "/org/gnome/SessionManager",
                "org.gnome.SessionManager",
                QDBusConnection::sessionBus());
}

IdleWatcher::~IdleWatcher()
{
    KIdleTime::instance()->removeAllIdleTimeouts();
}

void IdleWatcher::setup()
{
    KIdleTime::instance()->addIdleTimeout(1000 * mSecsidle);
    KIdleTime::instance()->addIdleTimeout(1000 * mSecspower);
}

void IdleWatcher::timeoutReached(int identifier , int timeout)
{
    quint32 inhibit_idle = 8;
    bool isinhibited = false;
    QDBusReply<bool> reply = interface->call("IsInhibited",inhibit_idle);
    if (reply.isValid()){
        // use the returned value
        qDebug()<<"Is inhibit by someone: "<<reply.value();
        isinhibited = reply.value();
    }
    else{
        qDebug()<<reply.value();
    }

    if(isinhibited == true){
        qDebug() <<"some applications inhibit idle.";
        return;
    }
    if(isinhibited == false){
        KIdleTime::instance()->catchNextResumeEvent();
        if(timeout == 1000 * mSecsidle){
            qDebug() << "idle Timeout Reached, emit StatusChanged 3 signal!";
            emit StatusChanged(3);
        }
        if(timeout == 1000 * mSecspower){
            qDebug() << "power Timeout Reached, emit StatusChanged 5 signal!";
            emit StatusChanged(5);
        }
    }
}

void IdleWatcher::resumingFromIdle(){
    qDebug() << "Somethings happened, emit StatusChanged 0 signal!";
    emit StatusChanged(0);
}

void IdleWatcher::reset(int idle , int power)
{
    qDebug() << "Idle timeout reset to " << idle << " ,Power timeout reset to "<<power;
    KIdleTime::instance()->removeAllIdleTimeouts();
    mSecsidle = idle;
    mSecspower = power;
    setup();
}

