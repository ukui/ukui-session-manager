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
#include <QMediaPlayer>
#include <QDBusInterface>
#include <map>

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

    void dostartwm();

    void startupfinished(const QString& appName ,const QString& string);

    // Qt5 users native event filter
    bool nativeEventFilter(const QByteArray &eventType, void* message, long* result) override;

    static void insertStartupList(QString &&str);

    static bool isProgramStarted(QString &&str);
private:
    bool startModuleTimer(QTimer *timer,int i);
    void playBootMusic(bool arg);
    void startProcess(const XdgDesktopFile &file, bool required);
    void constructStartupList();
    bool autoRestart(const XdgDesktopFile &file);

public slots:
    void startCompsite();
    void logout(bool doExit);
    void timerUpdate();
    void timeup();
    void weakup(bool arg);
    //void stateChanged(QMediaPlayer::State state);

private slots:
    void restartModules(int exitCode, QProcess::ExitStatus exitStatus);

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

    bool isPanelStarted    = false;
    bool isDesktopStarted  = false;
    bool isWMStarted       = false;
    bool isCompsiteStarted = false;

    bool isWayland         = false;
    bool wmFound           = false;

    //QMediaPlayer *player;
    bool isDirectInstall = false;
    bool mWmStarted;
    bool mTrayStarted;

    static std::map<QString, int> m_startupMap;
    ModulesMap mNameMap;
    XdgDesktopFileList mInitialization;
    XdgDesktopFile mWindowManager;
    XdgDesktopFile mPanel;
    XdgDesktopFile mFileManager;
    XdgDesktopFileList mDesktop;
    XdgDesktopFileList mApplication;
    XdgDesktopFileList mForceApplication;
};

#endif // MODULEMANAGER_H
