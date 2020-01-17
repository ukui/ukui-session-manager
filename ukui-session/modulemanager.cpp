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

ModuleManager::ModuleManager(QSettings* settings, QObject* parent)
    : QObject(parent),
      mSettings(settings)
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
    QString window_manager;
    QString panel;
    QString file_manager;

    if (mSettings->contains(QLatin1String("WindowManager")))
        window_manager = mSettings->value(QLatin1String("WindowManager")).toString() + ".desktop";

    if (mSettings->contains(QLatin1String("Panel")))
        panel = mSettings->value(QLatin1String("Panel")).toString() + ".desktop";

    if (mSettings->contains(QLatin1String("FileManager")))
        file_manager = mSettings->value(QLatin1String("FileManager")).toString() + ".desktop";

    QStringList desktop_paths;
    desktop_paths << "/usr/share/applications";
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
        }
        if (QFileInfo(file.fileName()).fileName() == file_manager)
        {
            mFileManager = file;
            fm_found = true;
        }
        if (QFileInfo(file.fileName()).fileName() == window_manager)
        {
            mWindowManager = file;
            wm_found = true;
        }

        if(fm_found && panel_found && wm_found)
            break;
    }

    QString desktop_phase = "X-UKUI-Autostart-Phase";
    QString desktop_type = "Type";
    const XdgDesktopFileList all_file_list = XdgAutoStart::desktopFileList();
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

    qDebug() << "wait for ukui-settings-daemon start-up";
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    timer->start(1000);
}

void ModuleManager::timerUpdate(){
    timer->stop();

    qDebug() << "Start panel: " << mPanel.name();
    startProcess(mPanel, true);

    qDebug() << "Start file manager: " << mFileManager.name();
    startProcess(mFileManager, true);

    qDebug() << "Start desktop: ";
    for (XdgDesktopFileList::const_iterator i = mDesktop.constBegin(); i != mDesktop.constEnd(); ++i){
        startProcess(*i, true);
    }

    qDebug() << "Start application: ";
    QFile file("/etc/xdg/autostart/kylin-nm.desktop");
    for (XdgDesktopFileList::const_iterator i = mApplication.constBegin(); i != mApplication.constEnd(); ++i){
        if(i->fileName()=="/etc/xdg/autostart/nm-applet.desktop" && file.exists()){
            qDebug() << "the kylin-nm exist so the nm-applet will not start";
            continue;
        }
        startProcess(*i, false);
    }

    qDebug() << "Start force application: ";
    for (XdgDesktopFileList::const_iterator i = mForceApplication.constBegin(); i != mForceApplication.constEnd(); ++i){
        startProcess(*i, false);
    }
}

void ModuleManager::startProcess(const XdgDesktopFile& file, bool required)
{
//    if (!required && !file.value(QL1S("OnlyShowIn")).toString().toUpper().contains("UKUI"))
//    {
//        if (file.value((QL1S("NotShowIn"))).toString().toUpper().contains("UKUI") || file.contains("OnlyShowIn"))
//        {
//            qDebug() << "Do not launch " << file.fileName();
//            return;
//        }

//        qDebug() << "Start detached: " << file.fileName();
//        file.startDetached();
//        return;
//    }

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
        connect(proc, SIGNAL(moduleStateChanged(QString, bool)), this, SIGNAL(moduleStateChanged(QString, bool)));
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
