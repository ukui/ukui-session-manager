#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "ukuimodule.h"

#include <QString>
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QEventLoop>
#include <QProcess>
#include <QMap>

class XdgDesktopFile;

typedef QMap<QString, UkuiModule*> ModulesMap;
typedef QMapIterator<QString, UkuiModule*> ModulesMapIterator;

class ModuleManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    ModuleManager(QObject* parent = nullptr);
    ~ModuleManager() override;

    XdgDesktopFileList Initialization;
    XdgDesktopFileList Windowmanager;
    XdgDesktopFileList Panel;
    XdgDesktopFileList Desktop;
    XdgDesktopFileList Applications;

    void startProcess(const QString& name, bool detach);

    void stopProcess(const QString& name);

    void startup();

    // Qt5 users native event filter
    bool nativeEventFilter(const QByteArray &eventType, void* message, long* result) override;

public slots:
    void logout(bool doExit);

    void startApps();

signals:
    void moduleStateChanged(QString moduleName, bool state);

private:

    void startProcess(const XdgDesktopFile &file, bool detach);

    ModulesMap mNameMap;

    bool mWmStarted;
    bool mTrayStarted;
    QEventLoop* mWaitLoop;

private slots:
    void restartModules(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif // MODULEMANAGER_H
