/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*               2010-2016 LXQt team
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301, USA.
**/
#ifndef SESSIONDBUSADAPTOR_H
#define SESSIONDBUSADAPTOR_H

#include <QtDBus>
#include "../tools/ukuipower.h"
#include "modulemanager.h"

class SessionDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.gnome.SessionManager")

public:
    SessionDBusAdaptor(ModuleManager *manager)
        : QDBusAbstractAdaptor(manager),
          mManager(manager),
          mPower(new UkuiPower())
    {
        connect(mManager, &ModuleManager::moduleStateChanged, this , &SessionDBusAdaptor::moduleStateChanged);
    }

Q_SIGNALS:
    void moduleStateChanged(QString moduleName, bool state);

public slots:
    bool canLogout()
    {
        return true;
    }

    bool canReboot()
    {
        return mPower->canAction(UkuiPower::PowerReboot);
    }

    bool canPowerOff()
    {
        return mPower->canAction(UkuiPower::PowerShutdown);
    }

    Q_NOREPLY void logout()
    {
        mManager->logout(true);
    }

    Q_NOREPLY void reboot()
    {
        mManager->logout(false);
        mPower->doAction(UkuiPower::PowerReboot);
        QCoreApplication::exit(0);
    }

    Q_NOREPLY void powerOff()
    {
        mManager->logout(false);
        mPower->doAction(UkuiPower::PowerShutdown);
        QCoreApplication::exit(0);
    }

//    QDBusVariant listModules()
//    {
//        return QDBusVariant(mManager->listModules());
//    }

    Q_NOREPLY void startModule(const QString& name)
    {
        mManager->startProcess(name, true);
    }

    Q_NOREPLY void stopModule(const QString& name)
    {
        mManager->stopProcess(name);
    }

private:
    ModuleManager *mManager;
    UkuiPower *mPower;
};

#endif // SESSIONDBUSADAPTOR_H
