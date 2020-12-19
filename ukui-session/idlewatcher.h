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
#ifndef IDLEWATCHER_H
#define IDLEWATCHER_H

#include <QObject>
#include <QDBusInterface>


//class ScreenSaver;

class IdleWatcher : public QObject
{
    Q_OBJECT
public:
    explicit IdleWatcher(int idle, int power, QObject *parent = nullptr);
    virtual ~IdleWatcher();

    void reset(int idle, int power);
    QDBusInterface *interface;

private slots:
    void resumingFromIdle();
    void timeoutReached(int identifier, int timeout);
    void setup();

Q_SIGNALS:
    void StatusChanged(uint status);

private:
    int mSecsidle;
    int mSecspower;
};

#endif // IDLEWATCHER_H
