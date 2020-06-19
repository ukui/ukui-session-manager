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

#define SESSION_DEFAULT_SETTINGS "org.ukui.session"
#define SESSION_DEFAULT_SETTINGS_PATH "/org/ukui/desktop/session/"
#define QT5_UKUI_STYLE "org.ukui.style"
#define PERIPHERALS_MOUSE "org.ukui.peripherals-mouse"
#define PERIPHERALS_MOUSE_PATH "/org/ukui/desktop/peripherals/mouse/"

QByteArray typeConver(int i){
    QString str = QString::number(i);
    QByteArray byte;
    byte.append(str);
    return byte;
}

void SessionApplication::InitialEnvironment()
{
//high-dpi settings has been setted by settings-daemon
//    QByteArray gdk_scale_QB;
//    QByteArray qt_scale_factor_QB;
//    int size = 1;//放大倍率
//    QDesktopWidget *desktop = QApplication::desktop();
//    qDebug() << "Screen-height is" << desktop->height() << ",Screnn-width is" << desktop->width();
//    if (desktop->height() >= 2000)
//        size = 2;
//    desktop->deleteLater();
//    if (gsettings_usable) {
//        int gdk_scale;
//        int qt_scale_factor;
//        bool Hidpi = gs->get("hidpi").toBool();
//        qDebug() << "Hidpi is "<< Hidpi;
//        if (!Hidpi) {
//            //gsettings值跟着改变,同步控制面板的值
//            gs->set("gdk-scale",typeConver(size));
//            gs->set("qt-scale-factor",typeConver(size));
//        }
//        gdk_scale = gs->get("gdk-scale").toInt();
//        qt_scale_factor = gs->get("qt-scale-factor").toInt();
//        qt_scale_factor_QB = typeConver(qt_scale_factor);
//        gdk_scale_QB = typeConver(gdk_scale);

//        //鼠标大小也根据分辨率来变
//        const QByteArray id(PERIPHERALS_MOUSE);
//        if (QGSettings::isSchemaInstalled(id)) {
//            QGSettings *gs_mouse = new QGSettings(PERIPHERALS_MOUSE,PERIPHERALS_MOUSE_PATH,this);
//            QByteArray mouseSize = "24";
//            if (!gs->get("mouse-size-changed").toBool()) {
//                if(size > 1)
//                    mouseSize = "48";
//                gs_mouse->set("cursor-size",mouseSize);
//            }
//            delete gs_mouse;
//        }
//    } else {
//        //设为默认值
//        gdk_scale_QB = "1";
//        qt_scale_factor_QB = "1";
//    }

    //检查qt主题是否安装
    const QByteArray qt_style(QT5_UKUI_STYLE);
    QByteArray QT_QPA_PLATFORMTHEME;
    if (QGSettings::isSchemaInstalled(qt_style)) {
        QT_QPA_PLATFORMTHEME = "ukui";
    } else {
        QT_QPA_PLATFORMTHEME = "gtk2";
    }

    //qDebug() << "gdk_scale" << gdk_scale_QB << "qt_scale_factor" << qt_scale_factor_QB;
    qputenv("XDG_CURRENT_DESKTOP","UKUI");
    //high-dpi settings has been setted by settings-daemon 
    //qputenv("GDK_SCALE",gdk_scale_QB);
    //qputenv("QT_SCALE_FACTOR",qt_scale_factor_QB);
    //qputenv("QT_AUTO_SCREEN_SET_FACTOR","0");
    //qputenv("QT_AUTO_SCREEN_SCALE_FACTOR","0");
    qputenv("QT_QPA_PLATFORMTHEME",QT_QPA_PLATFORMTHEME);
}

void SessionApplication::updateIdleDelay(){
    if (gsettings_usable) {
        const int time = gs->get("idle-delay").toInt() * 60;
        mIdleWatcher->reset(time);
    }
}

void SessionApplication::registerDBus()
{
    new SessionDBusAdaptor(modman);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (!dbus.registerService(QStringLiteral("org.gnome.SessionManager"))) {
        qCritical() << "Can't register org.gnome.SessionManager, there is already a session manager!";
    }
    if (!dbus.registerObject(("/org/gnome/SessionManager"), modman)) {
        qCritical() << "Can't register object, there is already an object registered at "
                    << "/org/gnome/SessionManager";
    }

    int timeout;
    if (gsettings_usable) {
        timeout = gs->get("idle-delay").toInt() * 60;
        connect(gs, &QGSettings::changed, this, &SessionApplication::updateIdleDelay);
    } else {
        timeout = 5 * 60;
    }
    mIdleWatcher = new IdleWatcher(timeout);
    new IdleDBusAdaptor(mIdleWatcher);
    if (!dbus.registerObject("/org/gnome/SessionManager/Presence", mIdleWatcher)) {
        qCritical() << "Cant' register object, there is already an object registered at "
                    << "org/gnome/SessionManager/Presence";
    }
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

    playBootMusic();
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
	
    modman->startup();

    return true;
}

void SessionApplication::playBootMusic(){
    //set default value of whether boot-music is opened
    bool play_music = true;
    if (gsettings_usable)
        play_music = gs->get("boot-music").toBool();
    if (play_music) {
        QMediaPlayer *player = new QMediaPlayer;
        player->setMedia(QUrl("qrc:/startup.wav"));
        player->play();
        QObject::connect(player,&QMediaPlayer::stateChanged,[=](QMediaPlayer::State state) {
            player->stop();
            player->deleteLater();
            //delete player;
            qDebug() << "play state is " << state;
        });
    }
}
