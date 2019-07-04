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

private:
    void startWm();

    void startAutostartApps();

    void startProcess(const XdgDesktopFile &file);

    QString mWindowManager;

    QProcess* mWmProcess;

    ModulesMap mNameMap;

    bool mWmStarted;
    bool mTrayStarted;
    QEventLoop* mWaitLoop;
};

#endif // MODULEMANAGER_H
