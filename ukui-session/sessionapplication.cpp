/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *               2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "sessionapplication.h"
#include "modulemanager.h"
#include "sessiondbusadaptor.h"
#include "idleadbusdaptor.h"
#include "idlewatcher.h"

#include <QDebug>
#include <QMediaPlayer>
#include <QDesktopWidget>
#include "../tools/ukuipower.h"
#include <QProcess>

#define SESSION_DEFAULT_SETTINGS "org.ukui.session"
#define SESSION_DEFAULT_SETTINGS_PATH "/org/ukui/desktop/session/"
#define QT5_UKUI_STYLE "org.ukui.style"
#define PERIPHERALS_MOUSE "org.ukui.peripherals-mouse"
#define PERIPHERALS_MOUSE_PATH "/org/ukui/desktop/peripherals/mouse/"
#define MOUSE_KEY "cursor-size"

#define FONT_RENDERING_SCHEMAS "org.ukui.font-rendering"
#define FONT_REENDERING_PATH "/org/ukui/desktop/font-rendering/"
#define DPI_KEY "dpi"

QByteArray typeConver(int i){
    QString str = QString::number(i);
    QByteArray byte;
    byte.append(str);
    return byte;
}

void SessionApplication::InitialEnvironment()
{
    UkuiPower *upower = new UkuiPower();
    if(gsettings_usable){
        if(upower->canAction(UkuiPower::PowerHibernate))
            gs->set("canhibernate",true);
	else
	    gs->set("canhibernate",false);
    }

    //检查qt主题是否安装
    const QByteArray qt_style(QT5_UKUI_STYLE);
    QByteArray QT_QPA_PLATFORMTHEME;
    if (QGSettings::isSchemaInstalled(qt_style)) {
        QT_QPA_PLATFORMTHEME = "ukui";
    } else {
        QT_QPA_PLATFORMTHEME = "gtk2";
    }

    qputenv("XDG_CURRENT_DESKTOP","UKUI");
    qputenv("QT_QPA_PLATFORMTHEME",QT_QPA_PLATFORMTHEME);
    qputenv("QT_QPA_PLATFORM", "xcb");

    QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
    if (xdg_session_type == "wayland"){
        QProcess::startDetached("dbus-update-activation-environment", QStringList() << "--systemd" << "DISPLAY"<<"QT_QPA_PLATFORM");
    }
    //restart user's gvfs-daemon.service
    QProcess::startDetached("systemctl", QStringList() << "--user" << "restart" << "gvfs-daemon.service");
}

void SessionApplication::updateIdleDelay(){
    if (gsettings_usable) {
        const int idle = gs->get("idle-delay").toInt() * 60;
        const int power = gs->get("power-delay").toInt() * 60;
        mIdleWatcher->reset(idle,power);
    }
}

void SessionApplication::registerDBus()
{
    new SessionDBusAdaptor(modman);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if(!dbus.isConnected()){
        qDebug()<<"Fatal DBus Error";
        QProcess a;
        a.setProcessChannelMode(QProcess::ForwardedChannels);
        a.start("dbus-launch", QStringList() << "--exit-with-session" << "ukui-session");
        a.waitForFinished(-1);
        if (a.exitCode()) {
            qWarning() <<  "exited with code" << a.exitCode();
        }
    }
    if (!dbus.registerService(QStringLiteral("org.gnome.SessionManager"))) {
        qCritical() << "Can't register org.gnome.SessionManager, there is already a session manager!";
    }
    if (!dbus.registerObject(("/org/gnome/SessionManager"), modman)) {
        qCritical() << "Can't register object, there is already an object registered at "
                    << "/org/gnome/SessionManager";
    }

    int idle = 5 * 60;
    int power = 5 * 60;

    if (gsettings_usable) {
        idle = gs->get("idle-delay").toInt() * 60;
        power = gs->get("power-delay").toInt() * 60;
        connect(gs, &QGSettings::changed, this, &SessionApplication::updateIdleDelay);
    }

    mIdleWatcher = new IdleWatcher(idle,power);
    new IdleDBusAdaptor(mIdleWatcher);
    if (!dbus.registerObject("/org/gnome/SessionManager/Presence", mIdleWatcher)) {
        qCritical() << "Cant' register object, there is already an object registered at "
                    << "org/gnome/SessionManager/Presence";
    }

    modman->startup();
}

SessionApplication::SessionApplication(int& argc, char** argv) :
    QApplication(argc, argv)
{
    const QByteArray id(SESSION_DEFAULT_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        gsettings_usable = true;
        gs = new QGSettings(SESSION_DEFAULT_SETTINGS,SESSION_DEFAULT_SETTINGS_PATH,this);

    } else {
        qWarning() << "Failed to get default value from gsettings, set gsettings_usable to false!";
        gsettings_usable = false;
    }

    InitialEnvironment();

    modman = new ModuleManager();


    // Wait until the event loop starts
    QTimer::singleShot(0, this, SLOT(startup()));
}

SessionApplication::~SessionApplication()
{
    delete modman;
    delete mIdleWatcher;
    //delete mSettings;
    delete gs;
}

bool SessionApplication::startup()
{
    QTimer::singleShot(0, this, SLOT(registerDBus()));

    return true;
}
