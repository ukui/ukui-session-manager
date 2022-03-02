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
#include "../tools/powerprovider.h"
#include "modulemanager.h"
#include "usminhibit.h"
#include "ukuismserver.h"
#include "sessionmanagercontext.h"
#include <KIdleTime>

class SessionDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.gnome.SessionManager")

public:
    SessionDBusAdaptor(SessionManagerDBusContext *context)
        :QDBusAbstractAdaptor(context)
    {
        connect(parent(), &SessionManagerDBusContext::moduleStateChanged, this, &SessionDBusAdaptor::moduleStateChanged);
        connect(parent(), &SessionManagerDBusContext::inhibitadded, this, &SessionDBusAdaptor::inhibitadded);
        connect(parent(), &SessionManagerDBusContext::inhibitremove, this, &SessionDBusAdaptor::inhibitremove);
        connect(parent(), &SessionManagerDBusContext::StartLogout, this, &SessionDBusAdaptor::StartLogout);
        connect(parent(), &SessionManagerDBusContext::PrepareForSwitchuser, this, &SessionDBusAdaptor::PrepareForSwitchuser);
        connect(parent(), &SessionManagerDBusContext::PrepareForPhase2, this, &SessionDBusAdaptor::PrepareForPhase2);
    }

    virtual ~SessionDBusAdaptor(){}

    inline SessionManagerDBusContext *parent() const
    {
        return static_cast<SessionManagerDBusContext *>(QObject::parent());
    }

Q_SIGNALS:
    void moduleStateChanged(QString moduleName, bool state);
    void inhibitadded(quint32 flags);
    void inhibitremove(quint32 flags);
    void StartLogout();
    void PrepareForSwitchuser();
    void PrepareForPhase2();

public slots:
    void startupfinished(const QString &appName ,const QString &string)
    {
        return parent()->startupfinished(appName, string);
    }

    bool canSwitch()
    {
        return parent()->canSwitch();
    }

    bool canHibernate()
    {
        return parent()->canHibernate();
    }

    bool canSuspend()
    {
        return parent()->canSuspend();
    }

    bool canLogout()
    {
        return parent()->canLogout();
    }

    bool canReboot()
    {
        return parent()->canReboot();
    }

    bool canPowerOff()
    {
        return parent()->canPowerOff();
    }

    Q_NOREPLY void switchUser()
    {
        return parent()->switchUser();
    }

    Q_NOREPLY void hibernate()
    {
        return parent()->hibernate();
    }

    Q_NOREPLY void suspend()
    {
        return parent()->suspend();
    }

    Q_NOREPLY void logout()
    {
        return parent()->logout();
    }

    Q_NOREPLY void reboot()
    {
        return parent()->reboot();
    }

    Q_NOREPLY void powerOff()
    {
        return parent()->powerOff();
    }

    Q_NOREPLY void startModule(const QString& name)
    {
        return parent()->startModule(name);
    }

    Q_NOREPLY void stopModule(const QString& name)
    {
        return parent()->stopModule(name);
    }

    uint Inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags)
    {
        return parent()->Inhibit(app_id,toplevel_xid, reason, flags);
    }

    Q_NOREPLY void Uninhibit(uint cookie)
    {
        return parent()->Uninhibit(cookie);
    }

    QStringList GetInhibitors()
    {
        return parent()->GetInhibitors();
    }

    bool IsSessionRunning(){
        return parent()->IsSessionRunning();
    }

    QString GetSessionName()
    {
        return parent()->GetSessionName();
    }

    bool IsInhibited(quint32 flags)
    {
        return parent()->IsInhibited(flags);
    }

    Q_NOREPLY void emitStartLogout()
    {
        return parent()->emitStartLogout();
    }

    Q_NOREPLY void emitPrepareForSwitchuser()
    {
        return parent()->emitPrepareForSwitchuser();
    }

    Q_NOREPLY void emitPrepareForPhase2()
    {
        return parent()->emitPrepareForPhase2();
    }

    Q_NOREPLY void simulateUserActivity(){
        return parent()->simulateUserActivity();
    }
};

#endif // SESSIONDBUSADAPTOR_H
