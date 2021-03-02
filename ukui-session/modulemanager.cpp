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
#include <QGSettings/QGSettings>
#include <QThread>
#include <QSoundEffect>
#include <QDBusInterface>
#include <QDir>
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#define SESSION_REQUIRED_COMPONENTS "org.ukui.session.required-components"
#define SESSION_REQUIRED_COMPONENTS_PATH "/org/ukui/desktop/session/required-components/"

class WorkerThread : public QThread
{
    Q_OBJECT
    void run() override {
        QSoundEffect *player = new QSoundEffect();
        player->setSource(QUrl("qrc:/startup.wav"));
        player->play();
        QTimer *a = new QTimer();
        a->start(8*1000);
        connect(a,&QTimer::timeout,this,&WorkerThread::quit);
        exec();
    }
};

void ModuleManager::playBootMusic(){
    //set default value of whether boot-music is opened
    bool play_music = true;
    if (QGSettings::isSchemaInstalled("org.ukui.session")){
        QGSettings *gset = new QGSettings("org.ukui.session","/org/ukui/desktop/session/",this);
        play_music = gset->get("boot-music").toBool();
    }
    if (play_music) {
        WorkerThread *workerThread = new WorkerThread();
        connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
        workerThread->start();
    }
}

ModuleManager::ModuleManager( QObject* parent)
    : QObject(parent)
{
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForSleep"), this, SLOT(weakup(bool)));
    constructStartupList();
}

void ModuleManager::weakup(bool arg){
    if(arg)
        qDebug()<<"准备好睡眠休眠了";
    else
        qDebug()<<"从睡眠休眠醒来了";
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
    if (QGSettings::isSchemaInstalled(id)) {
        const QGSettings* gs = new QGSettings(SESSION_REQUIRED_COMPONENTS,SESSION_REQUIRED_COMPONENTS_PATH,this);
        window_manager = gs->get("windowmanager").toString() + ".desktop";
        panel = gs->get("panel").toString() + ".desktop";
        file_manager = gs->get("filemanager").toString() + ".desktop";
        wm_notfound = gs->get("windowmanager").toString();
    } else {
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

    //const auto files = XdgAutoStart::desktopFileList(desktop_paths, false);
    for (const QString &dirName : const_cast<const QStringList&>(desktop_paths)) {
        QDir dir(dirName);
        if (!dir.exists())
            continue;
        const QFileInfoList files = dir.entryInfoList(QStringList(QLatin1String("*.desktop")), QDir::Files | QDir::Readable);
        for (const QFileInfo &fi : files) {
            if (fi.fileName() == panel) {
                mPanel.load(fi.absoluteFilePath());
                panel_found = true;
                qDebug() << "panel has been found";
            }
            if (fi.fileName() == file_manager) {
                mFileManager.load(fi.absoluteFilePath());
                fm_found = true;
                qDebug() << "filemanager has been found";
            }
            if (fi.fileName() == window_manager) {
                mWindowManager.load(fi.absoluteFilePath());
                wm_found = true;
                qDebug() << "windowmanager has been found";
            }

            if (fm_found && panel_found && wm_found)
                break;
        }
    }

    if (wm_found == false) {
        QFileInfo check_ukwm("/usr/share/applications/ukwm.desktop");
        QFileInfo check_ukuikwin("/usr/share/applications/ukui-kwin.desktop");
        if(check_ukwm.exists()) {
            window_manager = "ukwm.desktop";
            mWindowManager.load("/usr/share/applications/ukwm.desktop");
        }else if(check_ukuikwin.exists()) {
            window_manager = "ukui-kwin.desktop";
            mWindowManager.load("/usr/share/applications/ukui-kwin.desktop");
        }
        wm_found = true;
    }

    //配置文件所给的窗口管理器找不到.desktop文件时，将所给QString设为可执行命令，创建一个desktop文件赋给mWindowManager
//    if (wm_found == false) {
//        mWindowManager = XdgDesktopFile(XdgDesktopFile::ApplicationType,"window-manager", wm_notfound);
//        qDebug() << "windowmanager has been created";
//    }

    QString desktop_phase = "X-UKUI-Autostart-Phase";
    QString desktop_type = "Type";
    //设置excludeHidden为true，判断所有desktop文件的Hidden值，若为true，则将其从自启列表中去掉
    const XdgDesktopFileList all_file_list = XdgAutoStart::desktopFileList(true);
    for (XdgDesktopFileList::const_iterator i = all_file_list.constBegin(); i != all_file_list.constEnd(); ++i)
    {
        QString filename = QFileInfo(i->fileName()).fileName();
        if(filename == panel || filename == file_manager || filename == window_manager){
            continue;
        }
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

bool ModuleManager::startWmTimer(int i){
    qDebug() << "startprotect";
    twm = new QTimer();
    twm->setSingleShot(true);
    connect(twm,SIGNAL(timeout()),this,SLOT(timeup()));
    twm->start(i*1000);
    return true;
}

bool ModuleManager::startPanelTimer(int i){
    tpanel = new QTimer();
    tpanel->setSingleShot(true);
    connect(tpanel,SIGNAL(timeout()),this,SLOT(timeup()));
    tpanel->start(i*1000);
    return true;
}

bool ModuleManager::startDesktopTimer(int i){
    tdesktop = new QTimer();
    tdesktop->setSingleShot(true);
    connect(tdesktop,SIGNAL(timeout()),this,SLOT(timeup()));
    tdesktop->start(i*1000);
    return true;
}

void ModuleManager::startupfinished(const QString& appName , const QString& string ){
    qDebug() << "moudle :" + appName + "startup finished, and it want to say " + string;
    if(appName == "ukui-kwin"){
        if(runWm == false)
            disconnect(this, &ModuleManager::wmfinished,0,0);
        emit wmfinished();
        return;
    }
    if(appName == "ukui-panel"){
        if(runPanel == false)
            disconnect(this, &ModuleManager::panelfinished,0,0);
        emit panelfinished();
        return;
    }
    if(appName == "peony-qt-desktop"){
        if(runDesktop == false)
            disconnect(this, &ModuleManager::desktopfinished,0,0);
        emit desktopfinished();
        return;
    }
}

void ModuleManager::timeup(){
    QTimer *time_out = qobject_cast<QTimer*>(sender());
    if(time_out == twm){
        qDebug() <<"wm超时";
        emit wmfinished();
        return;
    }
    if(time_out == tpanel){
        qDebug() <<"panel超时";
        emit panelfinished();
        return;
    }
    if(time_out == tdesktop){
        qDebug() <<"peony-qt-desktop超时";
        emit desktopfinished();
        return;
    }
}

void ModuleManager::doStart(){
    qDebug() << "Start panel: " << mPanel.name();
    connect(this, &ModuleManager::panelfinished,this,&ModuleManager::timerUpdate);
    startProcess(mPanel, true);

    startPanelTimer(3);
}

void ModuleManager::startup()
{
    QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
    if(xdg_session_type == "wayland"){
        startProcess("hwaudioservice", true);
    }

    qDebug() << "Start Initialization app: ";
    for (XdgDesktopFileList::const_iterator i = mInitialization.constBegin(); i != mInitialization.constEnd(); ++i) {
        startProcess(*i, true);
    }

    qDebug() << "start peony-qt-desktop----------";
    QTimer::singleShot(2000, this, [&]()
    {
        qDebug() << "Start file manager: " << mFileManager.name();
        connect(this, &ModuleManager::desktopfinished,[&]()
        {
            tdesktop->stop();
            if(runDesktop == false)
                return;
            runDesktop = false;
            dostartwm();
        });
        startProcess(mFileManager, true);
        startDesktopTimer(5);
    });
}

void ModuleManager::dostartwm(){
    QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
    if (xdg_session_type != "wayland"){
        qDebug() <<"============";
        QTimer::singleShot(0, this, [&]()
        {
            qDebug() << "Start window manager: " << mWindowManager.name();
            if(mWindowManager.name() == "UKUI-KWin"){
                connect(this, &ModuleManager::wmfinished, [&]()
                {
                    twm->stop();
                    if(runWm == false)
                        return;
                    runWm = false;
                    doStart();
                });
                startProcess(mWindowManager, true);
                startWmTimer(18);
            }else{
                startProcess(mWindowManager, true);
                QTimer::singleShot(1000, this, [&]()
                {
                    doStart();
                });
            }
        });
    }else{
        doStart();
    }
}

void ModuleManager::timerUpdate(){
    //endprotect();
    tpanel->stop();
    if(runPanel == false)
        return;
    runPanel = false;

    playBootMusic();
    QTimer::singleShot(500, this, [&](){
        emit finished();
    });
    qDebug() << "Start desktop: ";
    for (XdgDesktopFileList::const_iterator i = mDesktop.constBegin(); i != mDesktop.constEnd(); ++i) {
        startProcess(*i, true);
    }

    qDebug() << "Start application: ";
    QFile file_nm("/etc/xdg/autostart/kylin-nm.desktop");
    QFile file_sogou("/usr/bin/sogouImeService");
    for (XdgDesktopFileList::const_iterator i = mApplication.constBegin(); i != mApplication.constEnd(); ++i) {
        qDebug() << i->fileName();
        if(i->fileName()=="/etc/xdg/autostart/nm-applet.desktop" && file_nm.exists()){
            qDebug() << "the kylin-nm exist so the nm-applet will not start";
            continue;
        }
        if(i->fileName()=="/etc/xdg/autostart/fcitx-qimpanel-autostart.desktop" && file_sogou.exists()){
            qDebug() << "the sogouImeService exist so the fcitx-ui-qimpanel will not start";
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
    if (args.isEmpty()) {
        qWarning() << "Wrong desktop file: " << file.fileName();
        return;
    }

    QString name = QFileInfo(file.fileName()).fileName();
    if (!mNameMap.contains(name)) {
        UkuiModule* proc = new UkuiModule(file, this);
        connect(proc, &UkuiModule::moduleStateChanged, this, &ModuleManager::moduleStateChanged);
        proc->start();

        mNameMap[name] = proc;

        if (required || autoRestart(file)) {
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
    for (const XdgDesktopFile& file : files) {
        if (QFileInfo(file.fileName()).fileName() == desktop_name) {
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

    if (nullptr == proc) {
        qWarning() << "Got an invalid (null) module to restart, Ignoring it";
        return;
    }

    if (proc->restartNum > 10) {
        mNameMap.remove(proc->fileName);
        disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), nullptr, nullptr);
        proc->deleteLater();
        return;
    }

    if (!proc->isTerminating()) {
        //通过killall方式杀死退出的程序，有的为NormalExit，有的为CrashExit
        //退出码exitCode的返回值为0时，程序为正常退出，此时exitStatus为NormalExit
        //所以我们不考虑killall杀死的情况，根据程序的返回值来确定是否重启
        QString procName = proc->file.name();
        switch (exitStatus) {
        case QProcess::NormalExit:
            qDebug() << "Process" << procName << "(" << proc << ") exited correctly. "<<"With the exitcode = "<<proc->exitCode();
            break;
        case QProcess::CrashExit:
            qDebug() << "Process" << procName << "(" << proc << ") has to be restarted. "<<"With the exitcode = "<<proc->exitCode();
            proc->start();
            proc->restartNum++;
            return;
        default:
            qWarning() << "Unknown exit status: " << procName << "(" << proc << ")";
        }
    }
    mNameMap.remove(proc->fileName);
    proc->deleteLater();
}

void ModuleManager::logout(bool doExit)
{
    // /org/freedesktop/login1/session/self 和 /org/freedesktop/login1/session/auto
    //有什么区别
    QDBusInterface face("org.freedesktop.login1",\
                             "/org/freedesktop/login1/session/self",\
                             "org.freedesktop.login1.Session",\
                             QDBusConnection::systemBus());
    ModulesMapIterator i(mNameMap);
//    UkuiModule *winman;
    while (i.hasNext()) {
        i.next();
        qDebug() << "Module logout" << i.key();
        UkuiModule *p = i.value();
//        if(p->file.name() == QFileInfo(mWindowManager.name()).fileName()){
//            winman = p;
//            continue;
//        }
        p->terminate();
    }
//    i.toFront();
//    while (i.hasNext()) {
//        i.next();
//        UkuiModule *p = i.value();
//        if(p->file.name() == QFileInfo(mWindowManager.name()).fileName()){
//            continue;
//        }
//        if (p->state() != QProcess::NotRunning && !p->waitForFinished(200)) {
//            qWarning() << "Module " << qPrintable(i.key()) << " won't termiante .. killing.";
//            p->kill();
//        }
//    }
    //winman->terminate();

    if (doExit) {
        face.call("Terminate");
        //QCoreApplication::exit(0);
        exit(0);
    }
}

#include "modulemanager.moc"
