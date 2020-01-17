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

IdleWatcher::IdleWatcher(int secs, QObject *parent) :
    QObject(parent),
    mSecs(secs)
{
    connect(KIdleTime::instance(),
            static_cast<void (KIdleTime::*)(int)>(&KIdleTime::timeoutReached),
            this,
            &IdleWatcher::timeoutReached);

    setup();
}

IdleWatcher::~IdleWatcher()
{
    KIdleTime::instance()->removeAllIdleTimeouts();
}

void IdleWatcher::setup()
{
    KIdleTime::instance()->addIdleTimeout(1000 * mSecs);
}

void IdleWatcher::timeoutReached(int identifier)
{
    qDebug() << "Timeout Reached, emit StatusChanged signal!";
    emit StatusChanged(3);
}

void IdleWatcher::reset(int timeout)
{
    qDebug() << "Idle timeout reset to " << timeout;
    KIdleTime::instance()->removeAllIdleTimeouts();
    mSecs = timeout;
    setup();
}
