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
#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "ukuimodule.h"

#include <QString>
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QEventLoop>
#include <QProcess>
#include <QVector>
#include <QMap>
#include <QTimer>

class XdgDesktopFile;
class IdleWatcher;

typedef QMap<QString, UkuiModule*> ModulesMap;
typedef QMapIterator<QString, UkuiModule*> ModulesMapIterator;

class ModuleManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    ModuleManager(QObject* parent = nullptr);
    ~ModuleManager() override;

    void startProcess(const QString& name, bool detach);

    void stopProcess(const QString& name);

    void startup();

    void doStart();

    void dostartwm();

    void startupfinished(const QString& appName ,const QString& string);

    // Qt5 users native event filter
    bool nativeEventFilter(const QByteArray &eventType, void* message, long* result) override;

public slots:
    void logout(bool doExit);
    void timerUpdate();
    void timeup();
    void weakup(bool arg);

Q_SIGNALS:
    void moduleStateChanged(QString moduleName, bool state);
    void finished();
    void usdfinished();
    void wmfinished();
    void panelfinished();
    void desktopfinished();

private:
    QTimer *tusd = new QTimer();
    QTimer *twm = new QTimer();
    QTimer *tpanel = new QTimer();
    QTimer *tdesktop = new QTimer();
    bool start_module_Timer(QTimer *timer,int i);

    bool runUsd = true;
    bool runWm = true;
    bool runPanel = true;
    bool runDesktop = true;
    void playBootMusic(bool arg);
    void startProcess(const XdgDesktopFile &file, bool required);

    void constructStartupList();

    bool autoRestart(const XdgDesktopFile &file);

    ModulesMap mNameMap;

    QList<QString> mAllAppList;

    bool mWmStarted;
    bool mTrayStarted;
    QEventLoop* mWaitLoop;

    XdgDesktopFileList mInitialization;
    XdgDesktopFile mWindowManager;
    XdgDesktopFile mPanel;
    XdgDesktopFile mFileManager;
    XdgDesktopFileList mDesktop;
    XdgDesktopFileList mApplication;
    XdgDesktopFileList mForceApplication;

private slots:
    void restartModules(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif // MODULEMANAGER_H
