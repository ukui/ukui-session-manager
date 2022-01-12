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
#include "ukuipower.h"
#include "powerprovider.h"
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>

UkuiPower::UkuiPower(QObject *parent) : QObject(parent)
{
    m_systemdProvider = new SystemdProvider(this);
}

UkuiPower::~UkuiPower()
{
}

bool UkuiPower::canAction(UkuiPower::Action action) const
{
    //以下为代码结构调整
    QString command;
    switch (action) {
    case PowerSwitchUser:
        command = QLatin1String("canSwitch");
        break;
    case PowerHibernate:
        command = QLatin1String("canHibernate");
        break;
    case PowerSuspend:
        command = QLatin1String("canSuspend");
        break;
    case PowerLogout:
        command = QLatin1String("canLogout");
        break;
    case PowerReboot:
        command = QLatin1String("canReboot");
        break;
    case PowerShutdown:
        command = QLatin1String("canPowerOff");
        break;
    default:
        break;
    }

    QDBusInterface *sessionInterface = new QDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                                          "org.gnome.SessionManager", QDBusConnection::sessionBus());

    if (!sessionInterface->isValid()) {
        qWarning() << "dbusCall: Session QDBusInterface is invalid";
        return false;
    }

    QDBusReply<bool> reply = sessionInterface->call(command);
    if (!reply.isValid()) {
        //解决老版本升级到新版本接口不兼容的问题，在session接口不存在的情况下，调用systemd的接口
        QDBusError error =  reply.error();
        if (error.type() == QDBusError::UnknownMethod) {
            return m_systemdProvider->canAction(action);

        }
        return false;
    }

    return reply.value();
}

bool UkuiPower::doAction(UkuiPower::Action action)
{
    QString command;
    switch (action) {
    case PowerSwitchUser:
        command = QLatin1String("switchUser");
        break;
    case PowerHibernate:
        command = QLatin1String("hibernate");
        break;
    case PowerSuspend:
        command = QLatin1String("suspend");
        break;
    case PowerLogout:
        command = QLatin1String("logout");
        break;
    case PowerReboot:
        command = QLatin1String("reboot");
        break;
    case PowerShutdown:
        command = QLatin1String("powerOff");
        break;
    default:
        break;
    }

    QDBusInterface *sessionInterface = new QDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                                          "org.gnome.SessionManager", QDBusConnection::sessionBus());

    if (!sessionInterface->isValid()) {
        qWarning() << "dbusCall: Session QDBusInterface is invalid";
        return false;
    }

    QDBusMessage mes = sessionInterface->call(command);

    if (!(mes.errorName().isEmpty())) {
        //本来应该判断错误类别，考虑到运行效率，不做该判断
        return m_systemdProvider->doAction(action);
    }

    return true;
}
