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

    void startProcess(const QString& name);

    void stopProcess(const QString& name);

    void startup();

    // Qt5 users native event filter
    bool nativeEventFilter(const QByteArray &eventType, void* message, long* result) override;

public slots:
    void logout(bool doExit);

signals:
    void moduleStateChanged(QString moduleName, bool state);

private:
    void startWm();

    void startAutostartApps();

    void startRequiredApps();

    void startProcess(const XdgDesktopFile &file);

    QString mWindowManager;

    QProcess* mWmProcess;

    ModulesMap mNameMap;

    QSettings* mSettings;

    bool mWmStarted;
    bool mTrayStarted;
    QEventLoop* mWaitLoop;

private slots:
    void restartModules(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif // MODULEMANAGER_H
