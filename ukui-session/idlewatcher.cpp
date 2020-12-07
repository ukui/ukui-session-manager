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
#include <QDBusInterface>
#include <QDebug>

IdleWatcher::IdleWatcher(int secs, QObject *parent) :
    QObject(parent),
    mSecs(secs)
{
    connect(KIdleTime::instance(),
            &KIdleTime::resumingFromIdle,
            this,
            &IdleWatcher::resumingFromIdle);
    connect(KIdleTime::instance(),
            static_cast<void (KIdleTime::*)(int)>(&KIdleTime::timeoutReached),
            this,
            &IdleWatcher::timeoutReached);

    setup();

    QDBusInterface *interface = new QDBusInterface(
                "org.gnome.SessionManager",
                "/org/gnome/SessionManager",
                "org.gnome.SessionManager",
                QDBusConnection::sessionBus());

    connect(interface, SIGNAL(inhibitadded(quint32)),this, SLOT(addflags(quint32)));
    connect(interface, SIGNAL(inhibitremove(quint32)),this, SLOT(removeflags(quint32)));

    num = 0;
}

IdleWatcher::~IdleWatcher()
{
    KIdleTime::instance()->removeAllIdleTimeouts();
}

void IdleWatcher::addflags(uint flags){
    if((flags & 8) == 8){
        num ++;
    }
}

void IdleWatcher::removeflags(uint flags){
    if(num < 0){
        return;
    }
    if((flags & 8) == 8){
        num--;
    }
}

void IdleWatcher::setup()
{
    KIdleTime::instance()->addIdleTimeout(1000 * mSecs);
}

void IdleWatcher::timeoutReached(int identifier)
{
    if(num > 0){
        qDebug() <<"some applications inhibit idle.";
        return;
    }
    if(num == 0){
        KIdleTime::instance()->catchNextResumeEvent();
        qDebug() << "Timeout Reached, emit StatusChanged 3 signal!";
        emit StatusChanged(3);
    }
}

void IdleWatcher::resumingFromIdle(){
    qDebug() << "Somethings happened, emit StatusChanged 0 signal!";
    emit StatusChanged(0);
}

void IdleWatcher::reset(int timeout)
{
    qDebug() << "Idle timeout reset to " << timeout;
    KIdleTime::instance()->removeAllIdleTimeouts();
    mSecs = timeout;
    setup();
}

