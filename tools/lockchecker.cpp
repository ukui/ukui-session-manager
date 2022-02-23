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

#include "lockchecker.h"
#include "loginedusers.h"
#include <sys/file.h>
#include <pwd.h>
#include <QString>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusInterface>

#define SYSTEMD_SERVICE     "org.freedesktop.login1"
#define SYSTEMD_PATH        "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE   "org.freedesktop.login1.Manager"

LockChecker::LockChecker()
{
}

LockChecker::~LockChecker()
{
}

int LockChecker::checkLock()
{
    return 2;
    bool lockfile = false;
    bool lockuser = false;

    QFile file_backup("/tmp/lock/kylin-backup.lock");
    QFile file_update("/tmp/lock/kylin-update.lock");
    if (file_backup.exists()) {
        int fd_backup = open(QString("/tmp/lock/kylin-backup.lock").toUtf8().data(), O_RDONLY);
        int b = flock(fd_backup, LOCK_EX|LOCK_NB);
        qDebug() << "b" << b;
        if (b < 0) {
            lockfile = true;
            QString file_user = getName(&file_backup);
            if (file_user == qgetenv("USER")) {
                lockuser = true;
            }
        }
        file_backup.close();
        if (flock(fd_backup, LOCK_UN) == 0) {
            qDebug() << "unlock sucess.";
        } else {
            qDebug() << "unlock fail.";
        }
    }
    if (file_update.exists()) {
        int fd_update = open(QString("/tmp/lock/kylin-update.lock").toUtf8().data(), O_RDONLY);
        int c = flock(fd_update, LOCK_EX|LOCK_NB);
        qDebug() << "c" << c;
        if (c < 0) {
            lockfile = true;
            QString file_user = getName(&file_update);
            if (file_user == qgetenv("USER")) {
                lockuser = true;
            }
        }
        file_backup.close();
        if (flock(fd_update, LOCK_UN) == 0) {
            qDebug() << "unlock sucess.";
        } else {
            qDebug() << "unlock fail.";
        }
    }
    if (lockfile) {
        if(lockuser)
            return 2;
        return 1;
    }
    return 0;
}

QStringList LockChecker::getLoginedUsers()
{
    QStringList loginedUser;
    qRegisterMetaType<LoginedUsers>("LoginedUsers");
    qDBusRegisterMetaType<LoginedUsers>();
    QDBusInterface loginInterface(SYSTEMD_SERVICE,
                                  SYSTEMD_PATH,
                                  SYSTEMD_INTERFACE,
                                  QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage result = loginInterface.call("ListUsers");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusArgument dbvFirst = first.value<QDBusArgument>();
    QVariant vFirst = dbvFirst.asVariant();
    const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

    QVector<LoginedUsers> loginedUsers;

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        LoginedUsers user;
        dbusArgs >> user;
        loginedUsers.push_back(user);
    }
    dbusArgs.endArray();

    for (LoginedUsers user : loginedUsers) {

        QDBusInterface userPertyInterface("org.freedesktop.login1",
                                          user.objpath.path(),
                                          "org.freedesktop.DBus.Properties",
                                          QDBusConnection::systemBus());

        QDBusReply<QVariant> reply = userPertyInterface.call("Get", "org.freedesktop.login1.User", "State");
        if (reply.isValid()) {
            QString status = reply.value().toString();
            if ("closing" != status) {
                loginedUser.append(user.userName);
            }
        }
    }
    return loginedUser;
}

/*只获取mode为block的inhibitors*/
QVector<Inhibitor> LockChecker::getInhibitors()
{
    QVector<Inhibitor> inhibitors;
    qRegisterMetaType<Inhibitor>("Inhibitorss");
    qDBusRegisterMetaType<Inhibitor>();
    QDBusInterface loginInterface(SYSTEMD_SERVICE,
                                  SYSTEMD_PATH,
                                  SYSTEMD_INTERFACE,
                                  QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage result = loginInterface.call("ListInhibitors");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusArgument dbvFirst = first.value<QDBusArgument>();
    QVariant vFirst = dbvFirst.asVariant();
    const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        Inhibitor inhibtor;
        dbusArgs >> inhibtor;

        if (inhibtor.action == QString("shutdown") || inhibtor.action == QString("sleep")) {
            if (inhibtor.mode == QString("block")) {
                inhibitors.push_back(inhibtor);
            }
        }
    }
    dbusArgs.endArray();

    return inhibitors;
}

bool LockChecker::isSleepBlocked()
{
    QVector<Inhibitor> inhibitors = getInhibitors();
    for (auto iter = inhibitors.begin(); iter != inhibitors.end(); ++iter) {
        if (iter->action == QString("sleep")) {
            return true;
        }
    }

    return false;
}

bool LockChecker::isShutdownBlocked()
{
    QVector<Inhibitor> inhibitors = getInhibitors();
    for (auto iter = inhibitors.begin(); iter != inhibitors.end(); ++iter) {
        if (iter->action == QString("shutdown")) {
            return true;
        }
    }

    return false;
}

void LockChecker::getSleepInhibitors(QStringList &sleepInhibitors, QStringList &sleepInhibitorsReason)
{
    QVector<Inhibitor> inhibitors = getInhibitors();
    if (inhibitors.size() > 0) {
        for (auto iter = inhibitors.begin(); iter != inhibitors.end(); ++iter) {
            if (iter->action == QString("sleep")) {//获取阻止睡眠的应用名和原因
                QString name = iter->name;
                sleepInhibitors.append(std::move(name));
                QString reason = iter->reason;
                sleepInhibitorsReason.append(std::move(reason));
            }
        }
    }
}

void LockChecker::getShutdownInhibitors(QStringList &shutdownInhibitors, QStringList &shutdownInhibitorsReason)
{
    QVector<Inhibitor> inhibitors = getInhibitors();
    if (inhibitors.size() > 0) {
        for (auto iter = inhibitors.begin(); iter != inhibitors.end(); ++iter) {
            if (iter->action == QString("shutdown")) {//获取阻止关机和重启的应用名和原因
                QString name = iter->name;
                shutdownInhibitors.append(std::move(name));
                QString reason = iter->reason;
                shutdownInhibitorsReason.append(std::move(reason));
            }
        }
    }
}

QString LockChecker::getName(QFile *a)
{
    QString user = getenv("USER");
    if (a->exists()) {
        a->open(QIODevice::ReadOnly|QIODevice::Text);
        QTextStream fileStream(a);
        int k = 0;
        while (!fileStream.atEnd()) {
            QString line = fileStream.readLine();
            if (k == 0) {
                QString a = line;
                qDebug() << "uid="<<a;
                struct passwd *user1;
                user1 = getpwuid(a.toInt());
                qDebug() << "name=" << user1->pw_name << ",uid=" << user1->pw_uid;
                if (user1->pw_name == NULL) {
                    return user;
                }
                user = user1->pw_name;
            }
            k++;
        }
    }
    return user;
}

int LockChecker::getCachedUsers()
{
    QDBusInterface loginInterface("org.freedesktop.Accounts",
                                  "/org/freedesktop/Accounts",
                                  "org.freedesktop.Accounts",
                                  QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage ret = loginInterface.call("ListCachedUsers");
    QList<QVariant> outArgs = ret.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
    QDBusObjectPath path;
    dbusArgs.beginArray();
    int userNum = 0;
    while (!dbusArgs.atEnd()) {
        dbusArgs >> path;
        userNum++;
    }
    dbusArgs.endArray();
    qDebug() << userNum;

    return userNum;
}

QDBusArgument &operator<<(QDBusArgument &argument, const Inhibitor &mystruct)
{
    argument.beginStructure();
    argument << mystruct.action << mystruct.name << mystruct.reason << mystruct.mode << mystruct.uid << mystruct.pid;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Inhibitor &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.action >> mystruct.name >> mystruct.reason >> mystruct.mode >> mystruct.uid >> mystruct.pid;
    argument.endStructure();
    return argument;
}
