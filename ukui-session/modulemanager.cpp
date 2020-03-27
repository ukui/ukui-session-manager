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
#include "modulemanager.h"
#include "ukuimodule.h"
#include "idlewatcher.h"

#include <QCoreApplication>
#include "xdgautostart.h"
#include "xdgdesktopfile.h"
#include "xdgdirs.h"
#include <QFileInfo>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QTimer>
#include <QGSettings/QGSettings>
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#define SESSION_REQUIRED_COMPONENTS "org.ukui.session.required-components"
#define SESSION_REQUIRED_COMPONENTS_PATH "/org/ukui/desktop/session/required-components/"

ModuleManager::ModuleManager( QObject* parent)
    : QObject(parent)
{
    constructStartupList();
}

ModuleManager::~ModuleManager()
{
    ModulesMapIterator i(mNameMap);
    while (i.hasNext())
    {
        i.next();

        auto p = i.value();
        disconnect(p, SIGNAL(finished(int, QProcess::ExitStatus)), nullptr, nullptr);

        delete p;
        mNameMap[i.key()] = nullptr;
    }
}

void ModuleManager::constructStartupList()
{
    const QByteArray id(SESSION_REQUIRED_COMPONENTS);
    QString window_manager;
    QString panel;
    QString file_manager;
    QString wm_notfound;
    if(QGSettings::isSchemaInstalled(id)) {
        const QGSettings* gs = new QGSettings(SESSION_REQUIRED_COMPONENTS,SESSION_REQUIRED_COMPONENTS_PATH,this);
        window_manager = gs->get("windowmanager").toString() + ".desktop";
        panel = gs->get("panel").toString() + ".desktop";
        file_manager = gs->get("filemanager").toString() + ".desktop";
        wm_notfound = gs->get("windowmanager").toString();
    }else{
        //gsetting安装失败，或无法获取，设置默认值
        qDebug() << "从gsettings 中或取值失败，设置默认值";
        window_manager = "ukwm.desktop";
        panel = "ukui-panel.desktop";
        file_manager = "peony-qt-desktop.desktop";
    }

    QStringList desktop_paths;
    desktop_paths << "/usr/share/applications";
    desktop_paths << "/etc/xdg/autostart";
    bool panel_found = false;
    bool fm_found = false;
    bool wm_found = false;

    const auto files = XdgAutoStart::desktopFileList(desktop_paths, false);
    for (const XdgDesktopFile& file : files)
    {
        if (QFileInfo(file.fileName()).fileName() == panel)
        {
            mPanel = file;
            panel_found = true;
            qDebug() << "panel has been found";
        }
        if (QFileInfo(file.fileName()).fileName() == file_manager)
        {
            mFileManager = file;
            fm_found = true;
            qDebug() << "filemanager has been found";
        }
        if (QFileInfo(file.fileName()).fileName() == window_manager)
        {
            mWindowManager = file;
            wm_found = true;
            qDebug() << "windowmanager has been found";
        }

        if(fm_found && panel_found && wm_found)
            break;
    }

    //配置文件所给的窗口管理器找不到.desktop文件时，将所给QString设为可执行命令，创建一个desktop文件赋给mWindowManager
    if(wm_found == false){
        mWindowManager = XdgDesktopFile(XdgDesktopFile::ApplicationType,"window-manager", wm_notfound);
        qDebug() << "windowmanager has been created";
    }

    QString desktop_phase = "X-UKUI-Autostart-Phase";
    QString desktop_type = "Type";
    //设置excludeHidden为true，判断所有desktop文件的Hidden值，若为true，则将其从自启列表中去掉
    const XdgDesktopFileList all_file_list = XdgAutoStart::desktopFileList(true);
    for (XdgDesktopFileList::const_iterator i = all_file_list.constBegin(); i != all_file_list.constEnd(); ++i)
    {
        const XdgDesktopFile file = *i;
        if (i->contains(desktop_phase)) {
            QStringList s1 =file.value(desktop_phase).toString().split(QLatin1Char(';'));
            if (s1.contains("Initialization")) {
                mInitialization << file;
            } else if (s1.contains("Desktop")) {
                mDesktop << file;
            } else if (s1.contains("Application")) {
                mApplication << file;
            }
        } else if (i->contains(desktop_type)) {
            QStringList s2 = file.value(desktop_type).toString().split(QLatin1Char(';'));
            if (s2.contains("Application")) {
                mApplication << file;
            }
        }
    }

    QStringList force_app_paths;
    force_app_paths << "/usr/share/ukui/applications";
    const XdgDesktopFileList force_file_list = XdgAutoStart::desktopFileList(force_app_paths, true);
    for (XdgDesktopFileList::const_iterator i = force_file_list.constBegin(); i != force_file_list.constEnd(); ++i)
    {
        qDebug() << (*i).fileName();
        mForceApplication << *i;
    }
}

/* Startup Phare:
 *  Initialization
 *  WindowManager
 *  Panel
 *  FileManager
 *  Desktop
 *  Application
 *
 */
void ModuleManager::startup()
{
    qDebug() << "Start Initialization app: ";
    for (XdgDesktopFileList::const_iterator i = mInitialization.constBegin(); i != mInitialization.constEnd(); ++i){
        startProcess(*i, true);
    }

    qDebug() << "Start window manager: " << mWindowManager.name();
    startProcess(mWindowManager, true);

    qDebug() << "Start panel: " << mPanel.name();
    startProcess(mPanel, true);

    qDebug() << "wait for ukui-settings-daemon start-up";
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    timer->start(1000);
}

void ModuleManager::timerUpdate(){
    timer->stop();
    delete timer;

    qDebug() << "Start file manager: " << mFileManager.name();
    startProcess(mFileManager, true);

    qDebug() << "Start desktop: ";
    for (XdgDesktopFileList::const_iterator i = mDesktop.constBegin(); i != mDesktop.constEnd(); ++i){
        startProcess(*i, true);
    }

    qDebug() << "Start application: ";
    QFile file("/etc/xdg/autostart/kylin-nm.desktop");
    for (XdgDesktopFileList::const_iterator i = mApplication.constBegin(); i != mApplication.constEnd(); ++i){
        qDebug() << i->fileName();
        if(i->fileName()=="/etc/xdg/autostart/nm-applet.desktop" && file.exists()){
            qDebug() << "the kylin-nm exist so the nm-applet will not start";
            continue;
        }
        startProcess(*i, false);
    }

    qDebug() << "Start force application: ";
    const QString ws = "ukui-window-switch";
    XdgDesktopFile ukui_ws= XdgDesktopFile(XdgDesktopFile::ApplicationType,"ukui-window-switch", ws);
    startProcess(ukui_ws,true);
    for (XdgDesktopFileList::const_iterator i = mForceApplication.constBegin(); i != mForceApplication.constEnd(); ++i){
        startProcess(*i, true);
    }
}

void ModuleManager::startProcess(const XdgDesktopFile& file, bool required)
{
    QStringList args = file.expandExecString();
    if (args.isEmpty())
    {
        qWarning() << "Wrong desktop file: " << file.fileName();
        return;
    }

    QString name = QFileInfo(file.fileName()).fileName();
    if (!mNameMap.contains(name))
    {
        UkuiModule* proc = new UkuiModule(file, this);
        connect(proc, &UkuiModule::moduleStateChanged, this, &ModuleManager::moduleStateChanged);
        proc->start();

        mNameMap[name] = proc;

        if (required || autoRestart(file))
        {
            connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(restartModules(int, QProcess::ExitStatus)));
        }
    }
}

void ModuleManager::startProcess(const QString& name, bool required)
{
    QString desktop_name = name + ".desktop";
    QStringList desktop_paths;
    desktop_paths << "/usr/share/applications";

    const auto files = XdgAutoStart::desktopFileList(desktop_paths, false);
    for (const XdgDesktopFile& file : files)
    {
        if (QFileInfo(file.fileName()).fileName() == desktop_name)
        {
            startProcess(file, required);
            return;
        }
    }
}

void ModuleManager::stopProcess(const QString& name)
{
     if (mNameMap.contains(name))
         mNameMap[name]->terminate();
}

bool ModuleManager::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (eventType != "xcb_generic_event_t") // We only want to handle XCB events
        return false;

    return false;
}

bool ModuleManager::autoRestart(const XdgDesktopFile &file)
{
    QString auto_restart = "X-UKUI-AutoRestart";
    return file.value(auto_restart).toBool();
}

void ModuleManager::restartModules(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
    UkuiModule* proc = qobject_cast<UkuiModule*>(sender());
    if(proc->restartNum > 10){
        mNameMap.remove(proc->fileName);
        disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), nullptr, nullptr);
        proc->deleteLater();
        return;
    }
    if (nullptr == proc) {
        qWarning() << "Got an invalid (null) module to restart, Ignoring it";
        return;
    }

    if (!proc->isTerminating())
    {
        QString procName = proc->file.name();
        switch (exitStatus)
        {
            case QProcess::NormalExit:
                qDebug() << "Process" << procName << "(" << proc << ") exited correctly.";
                break;
            case QProcess::CrashExit:
            {
                qDebug() << "Process" << procName << "(" << proc << ") has to be restarted";
                proc->start();
                proc->restartNum++;
                return;
//                time_t now = time(NULL);
            }
        }
    }
    mNameMap.remove(proc->fileName);
    proc->deleteLater();
}

void ModuleManager::logout(bool doExit)
{
    ModulesMapIterator i(mNameMap);
    while (i.hasNext()) {
        i.next();
        qDebug() << "Module logout" << i.key();
        UkuiModule *p = i.value();
        p->terminate();
    }
    i.toFront();
    while (i.hasNext()) {
        i.next();
        UkuiModule *p = i.value();
        if (p->state() != QProcess::NotRunning && !p->waitForFinished(2000)) {
            qWarning() << "Module " << qPrintable(i.key()) << " won't termiante .. killing.";
            p->kill();
        }
    }
    if (doExit)
        QCoreApplication::exit(0);
}

