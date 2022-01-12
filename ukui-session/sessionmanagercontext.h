/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *               2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef SESSIONINHIBITCONTEXT_H
#define SESSIONINHIBITCONTEXT_H


#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDebug>
#include "../tools/ukuipower.h"
#include "../tools/powerprovider.h"
#include "modulemanager.h"
#include "usminhibit.h"
#include "ukuismserver.h"
#include <KIdleTime>

#ifdef signals
#undef signals
#endif

class SessionDBusAdaptor;

class SessionManagerDBusContext : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    SessionManagerDBusContext(ModuleManager *manager,QObject *object = nullptr);
    ~SessionManagerDBusContext() override;

Q_SIGNALS:
    void moduleStateChanged(QString moduleName, bool state);
    void inhibitadded(quint32 flags);
    void inhibitremove(quint32 flags);
    void StartLogout();
    void PrepareForSwitchuser();
    void PrepareForPhase2();
public Q_SLOTS:

    Q_NOREPLY void startupfinished(const QString& appName ,const QString& string);

    bool canLogout();

    bool canSwitch();

    bool canHibernate();

    bool canSuspend();

    bool canReboot();

    bool canPowerOff();

    Q_NOREPLY void switchUser();

    Q_NOREPLY void hibernate();

    Q_NOREPLY void suspend();

    Q_NOREPLY void logout();

    Q_NOREPLY void reboot();

    Q_NOREPLY void powerOff();

    Q_NOREPLY void startModule(const QString& name);

    Q_NOREPLY void stopModule(const QString& name);

    uint Inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags);

    Q_NOREPLY void Uninhibit(uint cookie);

    QStringList GetInhibitors();

    bool IsSessionRunning();

    QString GetSessionName();

    bool IsInhibited(quint32 flags);

    Q_NOREPLY void emitStartLogout();

    Q_NOREPLY void emitPrepareForSwitchuser();

    Q_NOREPLY void emitPrepareForPhase2();

    Q_NOREPLY void simulateUserActivity();

    void on_serviceUnregistered(const QString &serviceName);

private:
    ModuleManager *mManager;
    usminhibit *minhibit;
    PowerProvider *m_systemdProvider;
    PowerProvider *m_ukuiProvider;

    QHash<QString, QList<quint32>> m_hashInhibitionServices;
    QDBusServiceWatcher *m_serviceWatcher = nullptr;
    SessionDBusAdaptor *m_sessionDBusAdaptor = nullptr;

    QTimer m_systemdLogoutTimer;
    QTimer m_systemdShutdownTimer;
    QTimer m_systemdRebootTimer;
};


#endif // SESSIONINHIBITCONTEXT_H
