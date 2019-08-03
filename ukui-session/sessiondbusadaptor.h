#ifndef SESSIONDBUSADAPTOR_H
#define SESSIONDBUSADAPTOR_H

#include <QtDBus>
#include "../session-tools/ukuipower.h"
#include "modulemanager.h"

class SessionDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Session")

public:
    SessionDBusAdaptor(ModuleManager *manager)
        : QDBusAbstractAdaptor(manager),
          mManager(manager),
          mPower(new UkuiPower())
    {
        connect(mManager, SIGNAL(moduleStateChanged(QString, bool)), SIGNAL(moduleStateChanged(QString, bool)));
    }

signals:
    void moduleStateChanged(QString moduleName, bool state);

public slots:
    bool canLogout()
    {
        return true;
    }

    bool canReboot()
    {
        return mPower->canAction(UkuiPower::PowerReboot);
    }

    bool canPowerOff()
    {
        return mPower->canAction(UkuiPower::PowerShutdown);
    }

    Q_NOREPLY void logout()
    {
        mManager->logout(true);
    }

    Q_NOREPLY void reboot()
    {
        mManager->logout(false);
        mPower->doAction(UkuiPower::PowerReboot);
        QCoreApplication::exit(0);
    }

    Q_NOREPLY void powerOff()
    {
        mManager->logout(false);
        mPower->doAction(UkuiPower::PowerShutdown);
        QCoreApplication::exit(0);
    }

//    QDBusVariant listModules()
//    {
//        return QDBusVariant(mManager->listModules());
//    }

    Q_NOREPLY void startModule(const QString& name)
    {
        mManager->startProcess(name);
    }

    Q_NOREPLY void stopModule(const QString& name)
    {
        mManager->stopProcess(name);
    }

private:
    ModuleManager *mManager;
    UkuiPower *mPower;
};

#endif // SESSIONDBUSADAPTOR_H
