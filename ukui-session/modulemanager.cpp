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

ModuleManager::ModuleManager(QObject* parent)
    : QObject(parent),
      mWmProcess(new QProcess(this))
{
    QString config_file = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session.ini";
    mSettings = new QSettings(config_file, QSettings::IniFormat);
    qDebug() << config_file;
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

void ModuleManager::startup()
{
    startWm();

    startRequiredApps();

    startAutostartApps();
}

void ModuleManager::startAutostartApps()
{
    const XdgDesktopFileList fileList = XdgAutoStart::desktopFileList();
    for (XdgDesktopFileList::const_iterator i = fileList.constBegin(); i != fileList.constEnd(); ++i)
    {
        startProcess(*i, true);
        qDebug() << "start" << i->fileName();
    }
}

void ModuleManager::startWm()
{
    mWindowManager = mSettings->value(QLatin1String("windowManager")).toString();
    qDebug() << "Start window manager: " << mWindowManager;
    mWmProcess->start(mWindowManager);
}

void ModuleManager::startRequiredApps()
{
    QStringList appList = mSettings->value("requiredApps").toStringList();
    qDebug() << "Required apps: ";
    for (QStringList::iterator it = appList.begin(); it != appList.end(); ++it) {
        qDebug() << *it;
        startProcess(*it, false);
    }
}

void ModuleManager::startProcess(const XdgDesktopFile& file, bool detach)
{
    qDebug() << file.value(QL1S("OnlyShowIn")).toString();
    if (!file.value(QL1S("OnlyShowIn")).toString().contains("UKUI") && detach)
    {
        qDebug() << "startDetachedd: " << file.fileName();
        file.startDetached();
        return;
    }

    QStringList args = file.expandExecString();
    if (args.isEmpty())
    {
        qDebug() << "Wrong desktop file" << file.fileName();
        return;
    }
    UkuiModule* proc = new UkuiModule(file, this);
    connect(proc, SIGNAL(moduleStateChanged(QString, bool)), this, SIGNAL(moduleStateChanged(QString, bool)));
    proc->start();

    QString name = QFileInfo(file.fileName()).fileName();
    mNameMap[name] = proc;

    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(restartModules(int, QProcess::ExitStatus)));
}

void ModuleManager::startProcess(const QString& name, bool detach)
{
    QString desktop_name = name + ".desktop";
    QStringList desktop_paths;
    desktop_paths << "/usr/share/applications";
    if (!mNameMap.contains(desktop_name))
    {
        const auto files = XdgAutoStart::desktopFileList(desktop_paths, false);
        for (const XdgDesktopFile& file : files)
        {
            if (QFileInfo(file.fileName()).fileName() == desktop_name)
            {
                startProcess(file, detach);
                return;
            }
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
