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
#include "idlewatcher.h"

#include <KIdleTime>
#include <QDebug>
#include <QDBusReply>

#define SYSTEMD_SERVICE   "org.freedesktop.login1"
#define SYSTEMD_PATH      "/org/freedesktop/login1/session/auto"
#define SYSTEMD_INTERFACE "org.freedesktop.login1.Session"
#define PROPERTY          "Active"

IdleWatcher::IdleWatcher(int idle, QObject *parent) : QObject(parent)
{
    if (idle > 0) {
        mSecsidle = idle;
    }

    connect(KIdleTime::instance(), &KIdleTime::resumingFromIdle,
            this, &IdleWatcher::resumingFromIdle);

    connect(KIdleTime::instance(), SIGNAL(timeoutReached(int, int)),
            this, SLOT(timeoutReached(int, int)));

    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForSleep"), this, SLOT(weakupFromSleep(bool)));

    setup();

    interface = new QDBusInterface("org.gnome.SessionManager",
                                   "/org/gnome/SessionManager",
                                   "org.gnome.SessionManager",
                                   QDBusConnection::sessionBus());

    args.append(QVariant(SYSTEMD_INTERFACE));
    args.append(QVariant(PROPERTY));
}

IdleWatcher::~IdleWatcher()
{
    KIdleTime::instance()->removeAllIdleTimeouts();
}

void IdleWatcher::setup()
{
    if (mSecsidle > 0) {
        KIdleTime::instance()->addIdleTimeout(1000 * mSecsidle);
    }
}

void IdleWatcher::weakupFromSleep(bool a)
{
    if (!a) {
        qDebug() << "模拟用户操作";
        KIdleTime::instance()->simulateUserActivity();
    }
}

void IdleWatcher::timeoutReached(int identifier, int timeout)
{
    quint32 inhibit_idle = 8;
    bool isinhibited = false;
    QDBusReply<bool> reply = interface->call("IsInhibited", inhibit_idle);

    if (reply.isValid()) {
        // use the returned value
        qDebug() << "Is inhibit by someone: " << reply.value();
        isinhibited = reply.value();
    } else {
        qDebug() << reply.value();
    }

    if (isinhibited == true) {
        qDebug() << "some applications inhibit idle.";
        return;
    }

    if (isinhibited == false) {
        //判断腾讯会议是否在进行
        if (isWemeetappRunning()) {
            return;
        }

        //Inactive user do not send StatusChanged signal.
        QDBusMessage mesg = QDBusMessage::createMethodCall(SYSTEMD_SERVICE,SYSTEMD_PATH, "org.freedesktop.DBus.Properties", "Get");
        mesg.setArguments(args);
        QDBusMessage ret = QDBusConnection::systemBus().call(mesg);

        if (ret.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Error getting property value.";
            return;
        }

        bool value = ret.arguments()[0].value<QDBusVariant>().variant().value<bool>();
        if (!value) return;

        KIdleTime::instance()->catchNextResumeEvent();
        if (timeout == 1000 * mSecsidle) {
            qDebug() << "idle Timeout Reached, emit StatusChanged 3 signal!";
            emit StatusChanged(3);
        }
    }
}

void IdleWatcher::resumingFromIdle()
{
    qDebug() << "Somethings happened, emit StatusChanged 0 signal!";
    emit StatusChanged(0);
}

void IdleWatcher::reset(int idle)
{
    KIdleTime::instance()->removeAllIdleTimeouts();
    if (idle > 0) {
        qDebug() << "Idle timeout reset to " << idle;
        mSecsidle = idle;
        setup();
    }
}

bool IdleWatcher::isWemeetappRunning()
{
    bool result = false;
    FILE *fp;
    const char *command;
    char *output = new char[4096];
    char c;

    //这条命令的含义是查询腾讯会议打开的端口，经过实验发现，腾讯会议在进行会议的时候，会存在一个特殊的UDP连接，
    //lsof命令会打印出该连接的详细信息，输出信息中用来标识的字符是"UDP *:"，通过这个特殊标识符来过滤我们需要的信息
    //进而用来判断腾讯会议是否在会议中
    //反斜杠注意前面还要加一个反斜杠转义,grep用\s\来表示空格，而\在字符串中是转义符号，所以为了能够在字符串中正确使用\，需要对\使用转义符\。
    command = "lsof -i -P -n | grep wemeetapp|grep 'UDP\\s\\*:'";
    //执行command并通过fp获取输出
    fp = popen(command, "r");

    //将command的输出存入outpur数组
    int i = 0;
    while (1) {
        c = fgetc(fp);
        output[i] = c;
        ++i;

        if (c == '\n' || c == EOF) {
            break;
        }
    }

    QString str(output);
    if (output[0] == '\n' || c == EOF) {
        //此处我们根据lsof命令的执行结果来判断，如果输出为空，那么读到的第一个字符就是换行符或者结束符，那就代表腾讯会议没有在进行会议
        result = false;
    } else {
        //str有值也不代表一定是我们需要的输出，需要根据str的内容来判断，但现在无法得知会存在什么样的错误输出，只能以后出现bug再做判断
//        qDebug() << str;
        qDebug() << "wemeetapp is running";
        result = true;
    }

    delete[] output;
    fclose(fp);

    return result;
}

