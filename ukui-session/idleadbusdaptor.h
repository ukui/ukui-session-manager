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
#ifndef IDLEDBUSADAPTOR_H
#define IDLEDBUSADAPTOR_H

#include <QtDBus>
#include "idlewatcher.h"

class IdleDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.gnome.SessionManager.Presence")

public:
    IdleDBusAdaptor(IdleWatcher *mIdleWatch)
        : QDBusAbstractAdaptor(mIdleWatch)
    {
        connect(mIdleWatch, SIGNAL(StatusChanged(uint)), SIGNAL(StatusChanged(uint)));
    }

signals:
    void StatusChanged(uint status);

private:
    IdleWatcher *mIdleWatch;
};

#endif // IDLEDBUSADAPTOR_H
