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

ModuleManager::ModuleManager(QObject* parent)
    : QObject(parent)
{
    qputenv("XDG_CURRENT_DESKTOP","UKUI");

    QString config_file = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session/ukui-session.ini";
    bool config_exists;
    if (QFile::exists(config_file))
        config_exists = true;
    else
        config_exists = false;

    mSettings = new QSettings(config_file, QSettings::IniFormat);

    if (!config_exists)
    {
//        mSettings->beginGroup("PHASE");
        mSettings->setValue("Windows_manager", "ukwm");
        mSettings->setValue("Panel", "ukui-panel");
        mSettings->setValue("Desktop", "peony");
//        mSettings->endGroup();

//        mSettings->beginGroup("IDLE");
        mSettings->setValue("idle_time_secs", 30);
//        mSettings->endGroup();

        mSettings->sync();
    }

    QString keyInit = "X-UKUI-Autostart-Phase";
    QString keyType = "Type";
    QStringList desktop_paths;
    desktop_paths << "/usr/share/applications";
    QString Win_desktop_name = mSettings->value(QLatin1String("Windows_manager")).toString() + ".desktop";
    QString Panel_desktop_name = mSettings->value(QLatin1String("Panel")).toString() + ".desktop";
    QString Desktop_desktop_name = mSettings->value(QLatin1String("Desktop")).toString() + ".desktop";

    const auto files = XdgAutoStart::desktopFileList(desktop_paths, false);
    bool findpanel = false;
    bool finddesktop = false;
    bool findwin = false;
    for (const XdgDesktopFile& file : files)
    {
        if (QFileInfo(file.fileName()).fileName() == Panel_desktop_name)
        {
            Panel << file;
            findpanel = true;
        }
        if (QFileInfo(file.fileName()).fileName() == Desktop_desktop_name)
        {
            Desktop << file;
            finddesktop = true;
        }
        if (QFileInfo(file.fileName()).fileName() == Win_desktop_name)
        {
            Windowmanager << file;
            findwin = true;
        }
        if(finddesktop && findpanel && findwin)
            break;
    }

    const XdgDesktopFileList AllfileList = XdgAutoStart::desktopFileList();
    //qDebug()<<"总数："<<AllfileList.count();
    for (XdgDesktopFileList::const_iterator i = AllfileList.constBegin(); i != AllfileList.constEnd(); ++i){
        XdgDesktopFile desktop = *i;
        if(i->contains(keyInit)){
            QStringList s1 =desktop.value(keyInit).toString().split(QLatin1Char(';'));
            if(s1.contains("Initialization")){
                Initialization << desktop;
            }
            else if(s1.contains("Application")){
                Applications << desktop;
            }
        }else if(i->contains(keyType)){
            QStringList s2 = desktop.value(keyType).toString().split(QLatin1Char(';'));
            if(s2.contains("Application")){
                Applications << desktop;
            }
        }
    }
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
    delete mSettings;
}

void ModuleManager::startApps()
{
    for(XdgDesktopFileList::const_iterator i = Desktop.constBegin(); i != Desktop.constEnd(); ++i){
        qDebug() << "Start Desktop app: " << i->fileName();
        startProcess(*i, true);
    }
}

void ModuleManager::startup(){
    for(XdgDesktopFileList::const_iterator i = Initialization.constBegin(); i != Initialization.constEnd(); ++i){
        qDebug() << "Start Initialization app: " << i->fileName();
        startProcess(*i, true);
    }
    QTimer::singleShot(2000, this, SLOT(startApps()));

    for(XdgDesktopFileList::const_iterator i = Windowmanager.constBegin(); i != Windowmanager.constEnd(); ++i){
        qDebug() << "Start Windowmanager app: " << i->fileName();
        startProcess(*i, true);
    }

    for(XdgDesktopFileList::const_iterator i = Panel.constBegin(); i != Panel.constEnd(); ++i){
        qDebug() << "Start Panel app: " << i->fileName();
        startProcess(*i, true);
    }

    QFile file("/etc/xdg/autostart/kylin-nm.desktop");
    for(XdgDesktopFileList::const_iterator i = Applications.constBegin(); i != Applications.constEnd(); ++i){
        if(i->fileName()=="/etc/xdg/autostart/nm-applet.desktop" && file.exists()){
            qDebug() << "the kylin-nm exist so the nm-applet will not start";
            continue;
        }
        qDebug() << "Start Initialization app: " << i->fileName();
        startProcess(*i, false);
    }
}

void ModuleManager::startProcess(const XdgDesktopFile& file, bool required)
{
    if (!required && !file.value(QL1S("OnlyShowIn")).toString().toUpper().contains("UKUI"))
    {
        if (file.value((QL1S("NotShowIn"))).toString().toUpper().contains("UKUI") || file.contains("OnlyShowIn"))
        {
            qDebug() << "Do not launch " << file.fileName();
            return;
        }

        qDebug() << "Start detached: " << file.fileName();
        file.startDetached();
        return;
    }

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

        connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(restartModules(int, QProcess::ExitStatus)));
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
