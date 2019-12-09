#include "modulemanager.h"
#include "ukuimodule.h"

#include <QCoreApplication>
#include <XdgAutoStart>
#include <XdgDirs>
#include <QFileInfo>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QTimer>

ModuleManager::ModuleManager(QObject* parent)
    : QObject(parent),
      mWmProcess(new QProcess(this))
{
    QString config_file = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session/ukui-session.ini";
    mSettings = new QSettings(config_file, QSettings::IniFormat);
//    mSettings->beginGroup("RequiredApps");
    QStringList apps;
    apps.append("peony");
    apps.append("ukui-panel");
    mSettings->setValue("required_apps", apps);
    mSettings->setValue("windows_manager", "ukwm");
    mSettings->setValue("apps","ukui-settings-daemon");
//    mSettings->endGroup();
    mSettings->sync();
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

    delete mWmProcess;
    delete mSettings;
}

void ModuleManager::startApps()
{
    startWm();

    startRequiredApps();

    startAutostartApps();
}

void ModuleManager::startup(){
    qputenv("XDG_CURRENT_DESKTOP","ukui");
    QString ukui_settings_daemon = mSettings->value(QLatin1String("apps")).toString();
    qDebug() << "Start apps: " << ukui_settings_daemon;
    QString desktop_name = ukui_settings_daemon + ".desktop";
    QStringList desktop_paths;
    desktop_paths << "/etc/xdg/autostart";
    const auto files = XdgAutoStart::desktopFileList(desktop_paths, false);
    for (const XdgDesktopFile& file : files)
    {
        if (QFileInfo(file.fileName()).fileName() == desktop_name)
        {
            startProcess(file, true);
        }
    }
    QTimer::singleShot(3000, this, SLOT(startApps()));
}

void ModuleManager::startAutostartApps()
{
    const XdgDesktopFileList fileList = XdgAutoStart::desktopFileList();
    QFile file("/etc/xdg/autostart/kylin-nm.desktop");
    for (XdgDesktopFileList::const_iterator i = fileList.constBegin(); i != fileList.constEnd(); ++i)
    {
        if(i->fileName()=="/etc/xdg/autostart/nm-applet.desktop" && file.exists()){
            qDebug() << "the kylin-nm exist so the nm-applet will not start";
            continue;
        }
        qDebug() << "Start autostart app: " << i->fileName();
        startProcess(*i, false);
    }
}

void ModuleManager::startWm()
{
    mWindowManager = mSettings->value(QLatin1String("windows_manager")).toString();
    qDebug() << "Start window manager: " << mWindowManager;
    mWmProcess->start(mWindowManager);
}

void ModuleManager::startRequiredApps()
{
    QStringList appList = mSettings->value(QLatin1String("required_apps")).toStringList();
    for (QStringList::iterator it = appList.begin(); it != appList.end(); ++it) {
        qDebug() << "Start required app: " << *it;
        startProcess(*it, true);
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
