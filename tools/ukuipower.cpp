/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 * Copyright: 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *            2012 Razor team
 * Authors:
 *   Christian Surlykke <christian@surlykke.dk>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */
#include "ukuipower.h"
#include "powerprovider.h"
#include <QDebug>

UkuiPower::UkuiPower(QObject *parent) : QObject(parent)
{
    mProviders.append(new UKUIProvider(this));
    mProviders.append(new SystemdProvider(this));
}

UkuiPower::~UkuiPower()
{
}

bool UkuiPower::canAction(UkuiPower::Action action) const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
    foreach(const PowerProvider *provider, mProviders) {
#else
    for (const PowerProvider *provider : qAsConst(mProviders)) {
#endif
        if (provider->canAction(action))
            return true;
    }

    return false;
}

bool UkuiPower::doAction(UkuiPower::Action action)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
    foreach(PowerProvider *provider, mProviders) {
#else
    for (PowerProvider *provider : qAsConst(mProviders)) {
#endif
        if (provider->canAction(action) && provider->doAction(action))
            return true;
    }

    return false;
}
