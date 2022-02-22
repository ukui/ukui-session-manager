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
#include "powerprovider.h"

#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDebug>

#include <sys/types.h>
#include <unistd.h>

#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QObject>
#include <QDBusReply>
//#include "loginedusers.h"

#define LIGHTDM_SERVICE     "org.freedesktop.DisplayManager"
#define LIGTHDM_INTERFACE   "org.freedesktop.DisplayManager.Seat"

#define SYSTEMD_SERVICE     "org.freedesktop.login1"
#define SYSTEMD_PATH        "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE   "org.freedesktop.login1.Manager"

#define UKUI_SERVICE        "org.gnome.SessionManager"
#define UKUI_PATH           "/org/gnome/SessionManager"
#define UKUI_INTERFACE      "org.gnome.SessionManager"

#define PROPERTIES_INTERFACE    "org.freedesktop.DBus.Properties"


bool messageBoxCheck()
{
    QMessageBox msgBox;
    //    msgBox.setWindowTitle(QObject::tr("conform"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    //    msgBox.setModal(false);
    msgBox.setText(QObject::tr("some applications are running and they don't want you to do this."));
    QPushButton *stillButton =
        msgBox.addButton(QObject::tr("Still to do!"), QMessageBox::ActionRole);
    QPushButton *giveupButton = msgBox.addButton(QObject::tr("give up"), QMessageBox::RejectRole);

    //    QStringList usrlist = getLoginedUsers();
    //    QList<QString>::Iterator it = usrlist.begin(),itend = usrlist.end();
    //    for(;it != itend;it++){
    //        qDebug()<<*it;
    //    }

    msgBox.exec();

    if (msgBox.clickedButton() == stillButton) {
        qDebug() << "Still to do!";
        return true;
    } else if (msgBox.clickedButton() == giveupButton) {
        qDebug() << "give up";
        return false;
    } else {
        return false;
    }
}

static bool dbusCall(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString &method)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path
                   << interface << method;
        return false;
    }

    QDBusMessage msg = dbus.call(method);

    if (!msg.errorName().isEmpty()) {
        qWarning() << "Dbus error: " << msg;
    }

    return msg.arguments().isEmpty() || msg.arguments().constFirst().isNull()
                                     || msg.arguments().constFirst().toBool();
}

static bool dbusCallSystemd(const QString &service,
                            const QString &path,
                            const QString &interface,
                            const QDBusConnection &connection,
                            const QString &method,
                            bool needBoolArg)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path
                   << interface << method;
        return false;
    }

    // QDBusMessage msg = dbus.call(method, needBoolArg ? QVariant(true) : QVariant());
    // 在华为相关的机器上，如果调用的method没有参数，但仍传入一个空值，会调用失败
    QDBusMessage msg;
    if (needBoolArg) {
        msg = dbus.call(method, QVariant(true));
    } else {
        msg = dbus.call(method);
    }

    if (!msg.errorName().isEmpty()) {
        qWarning() << "Debus error: " << msg;
    }

    if (msg.arguments().isEmpty() || msg.arguments().constFirst().isNull()) {
        return false;
    }

    QString response = msg.arguments().constFirst().toString();
    qDebug() << "systemd:" << method << "=" << response;

    // this need to be resolved:
    // while canReboot and canPoweroff return challenge,users click Reboot or shutdown,
    // we call logout ,then Reboot(true) and Poweroff(true) is called,
    // if user click cancel in the polkit messagebox,then Reboot or Poweroff is canceled,
    // but logout has been executed.
    return response == QLatin1String("yes");   //|| response == QLatin1String("challenge");
}

bool dbusGetProperty(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString &property)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusGetProperty: QDBusinterface is invalid" << service << path
                   << interface << property;
        return false;
    }

//    QDBusMessage msg = dbus.call("SwitchToGreeter");//QLatin1String("Get"), dbus.interface(),property

    //    if (!msg.errorName().isEmpty()) {
    //        qWarning() << "Dbus error: " << msg;
    //    }

    //    return !msg.arguments().isEmpty() &&
    //            msg.arguments().constFirst().value<QDBusVariant>().variant().toBool();

    QVariant canswitch = dbus.property("CanSwitch");
    qDebug() << property << "=" << canswitch.toString();
    return canswitch.toBool();
}

PowerProvider::PowerProvider(QObject *parent) : QObject(parent) 
{
}

PowerProvider::~PowerProvider()
{
}

/************************************************
  SystemdProvider

  http://www.freedesktop.org/wiki/Software/systemd/logind
 ************************************************/

SystemdProvider::SystemdProvider(QObject *parent): PowerProvider(parent)
{
}


SystemdProvider::~SystemdProvider()
{
}


bool SystemdProvider::canSwitchUser() const
{
    QString property      = "CanSwitch";
    QString xdg_seat_path = qgetenv("XDG_SEAT_PATH");
    return dbusGetProperty(QLatin1String(LIGHTDM_SERVICE),
                           xdg_seat_path,
                           QLatin1String(LIGTHDM_INTERFACE),
                           QDBusConnection::systemBus(),
                           property);
}

bool SystemdProvider::canAction(UkuiPower::Action action) const
{
    QString command;

    switch (action) {
    case UkuiPower::PowerSwitchUser:
        return canSwitchUser();
    case UkuiPower::PowerReboot:
        command = QLatin1String("CanReboot");
        break;

    case UkuiPower::PowerShutdown:
        command = QLatin1String("CanPowerOff");
        break;

    case UkuiPower::PowerSuspend:
        command = QLatin1String("CanSuspend");
        break;

    case UkuiPower::PowerHibernate:
        command = QLatin1String("CanHibernate");
        break;

    default:
        return false;
    }

    // canAction should be always silent because it can freeze
    // g_main_context_iteration Qt event loop in QMessageBox
    // on panel startup if there is no DBUS running.
    return dbusCallSystemd(QLatin1String(SYSTEMD_SERVICE),
                           QLatin1String(SYSTEMD_PATH),
                           QLatin1String(SYSTEMD_INTERFACE),
                           QDBusConnection::systemBus(),
                           command,
                           false);
}


bool SystemdProvider::doSwitchUser()
{
    bool isinhibited =false;
    QDBusInterface *interface = new QDBusInterface(
                "org.gnome.SessionManager",
                "/org/gnome/SessionManager",
                "org.gnome.SessionManager",
                QDBusConnection::sessionBus());
    quint32 inhibit_switchuser = 2;
    QDBusReply<bool> reply = interface->call("IsInhibited",inhibit_switchuser);
    if (reply.isValid()) {
        // use the returned value
        qDebug() << "Is inhibit by someone: " << reply.value();
        isinhibited = reply.value();
    } else {
        qDebug() << reply.value();
    }

    if (isinhibited == true) {
        isinhibited = !messageBoxCheck();
    }

    if (isinhibited == false) {

//        QDBusInterface dbus("org.ukui.KWin", "/Compositor", "org.ukui.kwin.Compositing", QDBusConnection::sessionBus());

//        if (!dbus.isValid()) {
//            qWarning() << "dbusCall: QDBusInterface is invalid";
//            return false;
//        }

//        dbus.call("suspend");

        QString command       = "SwitchToGreeter";
        QString xdg_seat_path = qgetenv("XDG_SEAT_PATH");
        return dbusCall(QLatin1String(LIGHTDM_SERVICE),
                        xdg_seat_path,
                        QLatin1String(LIGTHDM_INTERFACE),
                        QDBusConnection::systemBus(),
                        command);
    }
    return false;
}

bool SystemdProvider::doAction(UkuiPower::Action action)
{
    QString command;

    switch (action) {
    case UkuiPower::PowerSwitchUser:
        return doSwitchUser();
    case UkuiPower::PowerReboot:
        command = QLatin1String("Reboot");
        break;
    case UkuiPower::PowerShutdown:
        command = QLatin1String("PowerOff");
        break;
    case UkuiPower::PowerSuspend:
        command = QLatin1String("Suspend");
        break;
    case UkuiPower::PowerHibernate:
        command = QLatin1String("Hibernate");
        break;
    default:
        return false;
    }

    return dbusCallSystemd(QLatin1String(SYSTEMD_SERVICE),
                           QLatin1String(SYSTEMD_PATH),
                           QLatin1String(SYSTEMD_INTERFACE),
                           QDBusConnection::systemBus(),
                           command,
                           true);
}


UKUIProvider::UKUIProvider(QObject *parent): PowerProvider (parent)
{
}

UKUIProvider::~UKUIProvider()
{
}

bool UKUIProvider::canAction(UkuiPower::Action action) const
{
    //这里是调用ukui-session注册的d-bus检测是否有inhibitor存在
        bool isinhibited = false;
        QDBusInterface *interface = new QDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                                       "org.gnome.SessionManager", QDBusConnection::sessionBus());
        quint32 inhibit_logout = 1;
        QDBusReply<bool> reply = interface->call("IsInhibited", inhibit_logout);
        if (reply.isValid()) {
            // use the returned value
            qDebug() << "Is inhibit by someone: " << reply.value();
            isinhibited = reply.value();
        } else {
            qDebug() << reply.value();
        }

        if (isinhibited == true) {
            isinhibited = !messageBoxCheck();
        }

        return !isinhibited;

}

bool UKUIProvider::doAction(UkuiPower::Action action)
{
    QString command;
    switch (action) {
    case UkuiPower::PowerLogout:
        command = QLatin1String("logout");
        break;
    case UkuiPower::PowerReboot:
        command = QLatin1String("reboot");
        break;
    case UkuiPower::PowerShutdown:
        command = QLatin1String("powerOff");
        break;
    default:
        return false;
    }

    qDebug() << "ukuipowerprovider call D-Bus session";
    return dbusCall(QLatin1String(UKUI_SERVICE), QLatin1String(UKUI_PATH),
                    QLatin1String(UKUI_INTERFACE), QDBusConnection::sessionBus(), command);
}


