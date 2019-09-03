#ifndef UKUIPOWER_H
#define UKUIPOWER_H

#include <QObject>
#include <QList>

class PowerProvider;

class UkuiPower : public QObject
{
    Q_OBJECT
public:
    enum Action {
        PowerLogout,
        PowerSwitchUser,
        PowerHibernate,
        PowerReboot,
        PowerShutdown,
        PowerSuspend,
        PowerMonitorOff
    };

    explicit UkuiPower(QObject *parent = nullptr);
    virtual ~UkuiPower();

    bool canAction(Action action) const;

public slots:
    bool doAction(Action action);

private:
    QList<PowerProvider*> mProviders;
};

#endif // UKUIPOWER_H
