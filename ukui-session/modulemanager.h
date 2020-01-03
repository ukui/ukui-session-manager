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

class XdgDesktopFile;
class IdleWatcher;

typedef QMap<QString, UkuiModule*> ModulesMap;
typedef QMapIterator<QString, UkuiModule*> ModulesMapIterator;

class ModuleManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    ModuleManager(QSettings* settings, QObject* parent = nullptr);
    ~ModuleManager() override;

    void startProcess(const QString& name, bool detach);

    void stopProcess(const QString& name);

    void startup();

    // Qt5 users native event filter
    bool nativeEventFilter(const QByteArray &eventType, void* message, long* result) override;

public slots:
    void logout(bool doExit);

signals:
    void moduleStateChanged(QString moduleName, bool state);

private:

    void startProcess(const XdgDesktopFile &file, bool required);

    void constructStartupList();

    bool autoRestart(const XdgDesktopFile &file);

    ModulesMap mNameMap;

    QList<QString> mAllAppList;

    QSettings* mSettings;

    bool mWmStarted;
    bool mTrayStarted;
    QEventLoop* mWaitLoop;

    XdgDesktopFileList mInitialization;
    XdgDesktopFile mWindowManager;
    XdgDesktopFile mPanel;
    XdgDesktopFile mFileManager;
    XdgDesktopFileList mDesktop;
    XdgDesktopFileList mApplication;
    XdgDesktopFileList mForceApplication;

private slots:
    void restartModules(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif // MODULEMANAGER_H
