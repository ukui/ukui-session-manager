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
#ifndef UKUIPOWER_H
#define UKUIPOWER_H

#include <QObject>
#include <QList>
#include <QSoundEffect>
#include <QTimer>

class PowerProvider;

class UkuiPower : public QObject
{
    Q_OBJECT
public:
    enum Action {
        PowerLogout,
        PowerSwitchUser,
        PowerHibernate,
        PowerReboot,
        PowerShutdown,
        PowerSuspend,
        PowerMonitorOff
    };

    explicit UkuiPower(QObject *parent = nullptr);
    virtual ~UkuiPower();

    bool canAction(Action action) const;

    QSoundEffect *soundplayer;
    void playmusic();

public slots:
    bool doAction(Action action);

private:
    QList<PowerProvider*> mProviders;
};

#endif // UKUIPOWER_H
