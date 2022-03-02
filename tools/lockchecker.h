/*
 * Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef LOCKCHECKER_H
#define LOCKCHECKER_H

#include <QFile>
#include <QDBusArgument>

struct Inhibitor {
    QString action;
    QString name;
    QString reason;
    QString mode;
    int uid;
    int pid;
};

QDBusArgument &operator<<(QDBusArgument &argument, const Inhibitor &mystruct);

const QDBusArgument &operator>>(const QDBusArgument &argument, Inhibitor &mystruct);

Q_DECLARE_METATYPE(Inhibitor)

class LockChecker
{
public:
    LockChecker();
    ~LockChecker();

public:
    static int checkLock();

    static QStringList getLoginedUsers();

    static QVector<Inhibitor> getInhibitors();

    static bool isSleepBlocked();

    static bool isShutdownBlocked();

    static void getSleepInhibitors(QStringList &sleepInhibitors, QStringList &sleepInhibitorsReason);

    static void getShutdownInhibitors(QStringList &shutdownInhibitors, QStringList &shutdownInhibitorsReason);

    static int getCachedUsers();

private:
    static QString getName(QFile *a);
};

#endif // LOCKCHECKER_H
