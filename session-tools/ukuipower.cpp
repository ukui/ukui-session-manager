#include "ukuipower.h"
#include "powerprovider.h"

UkuiPower::UkuiPower(QObject *parent) : QObject(parent)
{
    mProviders.append(new SystemdProvider(this));
    mProviders.append(new UPowerProvider(this));
    mProviders.append(new ConsoleKitProvider(this));
    mProviders.append(new UKUIProvider(this));
}

UkuiPower::~UkuiPower()
{
}

bool UkuiPower::canAction(UkuiPower::Action action) const
{
    for (const PowerProvider *provider : qAsConst(mProviders)) {
        if (provider->canAction(action))
            return true;
    }

    return false;
}

bool UkuiPower::doAction(UkuiPower::Action action)
{
    for (PowerProvider *provider : qAsConst(mProviders)) {
        if (provider->canAction(action) && provider->doAction(action))
            return true;
    }

    return false;
}
