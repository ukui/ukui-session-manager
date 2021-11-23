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

#define SYSTEMD_SERVICE   "org.freedesktop.login1"
#define SYSTEMD_PATH      "/org/freedesktop/login1/session/auto"
#define SYSTEMD_INTERFACE "org.freedesktop.login1.Session"
#define PROPERTY          "Active"

IdleWatcher::IdleWatcher(int idle, QObject *parent) : QObject(parent)
                                                    , mSecsidle(idle)
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

    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForSleep"), this, SLOT(weakupFromSleep(bool)));

    setup();

    interface = new QDBusInterface(
                "org.gnome.SessionManager",
                "/org/gnome/SessionManager",
                "org.gnome.SessionManager",
                QDBusConnection::sessionBus());

    args.append(QVariant(SYSTEMD_INTERFACE));
    args.append(QVariant(PROPERTY));
}

IdleWatcher::~IdleWatcher()
{
    KIdleTime::instance()->removeAllIdleTimeouts();
}

void IdleWatcher::setup()
{
    KIdleTime::instance()->addIdleTimeout(1000 * mSecsidle);
}

void IdleWatcher::weakupFromSleep(bool a){
    if (!a) {
        qDebug() << "模拟用户操作";
        KIdleTime::instance()->simulateUserActivity();
    }
}

void IdleWatcher::timeoutReached(int identifier , int timeout)
{
    quint32 inhibit_idle = 8;
    bool isinhibited = false;
    QDBusReply<bool> reply = interface->call("IsInhibited",inhibit_idle);
    if (reply.isValid()) {
        // use the returned value
        qDebug() << "Is inhibit by someone: " << reply.value();
        isinhibited = reply.value();
    } else {
        qDebug() << reply.value();
    }

    if (isinhibited == true) {
        qDebug() << "some applications inhibit idle.";
        return;
    }
    if (isinhibited == false) {
        //Inactive user do not send StatusChanged signal.
        QDBusMessage mesg = QDBusMessage::createMethodCall(SYSTEMD_SERVICE,SYSTEMD_PATH,"org.freedesktop.DBus.Properties","Get");
        mesg.setArguments(args);
        QDBusMessage ret = QDBusConnection::systemBus().call(mesg);
        if (ret.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Error getting property value.";
            return;
        }
        bool value = ret.arguments()[0].value<QDBusVariant>().variant().value<bool>();
        if (!value) return;

        KIdleTime::instance()->catchNextResumeEvent();
        if (timeout == 1000 * mSecsidle) {
            qDebug() << "idle Timeout Reached, emit StatusChanged 3 signal!";
            emit StatusChanged(3);
        }
    }
}

void IdleWatcher::resumingFromIdle()
{
    qDebug() << "Somethings happened, emit StatusChanged 0 signal!";
    emit StatusChanged(0);
}

void IdleWatcher::reset(int idle )
{
    qDebug() << "Idle timeout reset to " << idle;
    KIdleTime::instance()->removeAllIdleTimeouts();
    mSecsidle = idle;
    setup();
}

