#ifndef POWERPROVIDER_H
#define POWERPROVIDER_H

#include <QObject>

#include "ukuipower.h"

class PowerProvider : public QObject
{
    Q_OBJECT
public:
    explicit PowerProvider(QObject *parent = nullptr);
    virtual ~PowerProvider();

    virtual bool canAction(UkuiPower::Action action) const = 0;

public slots:
    virtual bool doAction(UkuiPower::Action action) = 0;
};

class SystemdProvider: public PowerProvider
{
    Q_OBJECT
public:
    SystemdProvider(QObject *parent = nullptr);
    ~SystemdProvider();
    bool canAction(UkuiPower::Action action) const;

public slots:
    bool doAction(UkuiPower::Action action);
};

class UKUIProvider: public PowerProvider
{
    Q_OBJECT
public:
    UKUIProvider(QObject *parent = nullptr);
    ~UKUIProvider();
    bool canAction(UkuiPower::Action action) const;

public slots:
    bool doAction(UkuiPower::Action action);
};

#endif // POWERPROVIDER_H
