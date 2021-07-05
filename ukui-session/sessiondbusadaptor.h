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
#include "usminhibit.h"

class SessionDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.gnome.SessionManager")

public:
    SessionDBusAdaptor(ModuleManager *manager)
        : QDBusAbstractAdaptor(manager),
          mManager(manager),
          mPower(new UkuiPower()),
          minhibit(new usminhibit())
    {
        connect(mManager, &ModuleManager::moduleStateChanged, this , &SessionDBusAdaptor::moduleStateChanged);
        connect(mManager, &ModuleManager::finished,this,&SessionDBusAdaptor::emitPrepareForPhase2);
    }

Q_SIGNALS:
    void moduleStateChanged(QString moduleName, bool state);
    void inhibitadded(quint32 flags);
    void inhibitremove(quint32 flags);
    void StartLogout();
    void PrepareForSwitchuser();
    void PrepareForPhase2();

public slots:
    void startupfinished(const QString& appName ,const QString& string)
    {
        return mManager->startupfinished(appName,string);
    }

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
        if(mPower->canAction(UkuiPower::PowerReboot)){
            mManager->logout(false);
            mPower->doAction(UkuiPower::PowerReboot);
        }
        //QCoreApplication::exit(0);
    }

    Q_NOREPLY void powerOff()
    {
        if(mPower->canAction(UkuiPower::PowerShutdown)){
            mManager->logout(false);
            mPower->doAction(UkuiPower::PowerShutdown);
        }
        //QCoreApplication::exit(0);
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

    uint Inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags)
    {
        uint result = minhibit->addinhibit(app_id,toplevel_xid,reason,flags);
        if(result < 0){
            return 0;
        }
        emit inhibitadded(flags);
        return result;
    }

    Q_NOREPLY void Uninhibit(uint cookie){
        uint result = minhibit->uninhibit(cookie);
        if(result > 0){
            emit inhibitremove(result);
        }
    }

    QStringList GetInhibitors(){
        return minhibit->getinhibitor();
    }

    bool IsSessionRunning(){
        QString xdg_session_desktop = qgetenv("XDG_SESSION_DESKTOP").toLower();
        if(xdg_session_desktop == "ukui")
            return true;
        return false;
    }

    QString GetSessionName(){
        QString xdg_session_desktop = qgetenv("XDG_SESSION_DESKTOP").toLower();
        if(xdg_session_desktop == "ukui")
            return "ukui-session";
        return "error";
    }

    bool IsInhibited(quint32 flags){
        return minhibit->IsInhibited(flags);
    }

    Q_NOREPLY void emitStartLogout(){
        qDebug()<<"emit  StartLogout";
        emit StartLogout();
    }

    Q_NOREPLY void emitPrepareForSwitchuser(){
        qDebug()<<"emit  PrepareForSwitchuser";
        emit PrepareForSwitchuser();
    }

    Q_NOREPLY void emitPrepareForPhase2(){
        qDebug()<<"emit  PrepareForPhase2";
        emit PrepareForPhase2();
    }

private:
    ModuleManager *mManager;
    UkuiPower *mPower;
    usminhibit *minhibit;
};

#endif // SESSIONDBUSADAPTOR_H
