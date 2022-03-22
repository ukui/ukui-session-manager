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

class PowerProvider;

class UkuiPower : public QObject
{
    Q_OBJECT
public:
    enum Action {
        PowerSwitchUser,
        PowerHibernate,
        PowerSuspend,
        PowerMonitorOff,        
        PowerLogout,
        PowerReboot,
        PowerShutdown,
    };

    explicit UkuiPower(QObject *parent = nullptr);
    virtual ~UkuiPower();

    bool canAction(Action action) const;

public slots:
    bool doAction(Action action);

private:
    PowerProvider *m_systemdProvider;
};

#endif // UKUIPOWER_H
