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

void SessionApplication::InitialEnvironment()
{
    int gdk_scale;
    int qt_scale_factor;
    QDesktopWidget *desktop = QApplication::desktop();
    qDebug()<< "Screen-height is"<<desktop->height()<<",Screnn-width is"<<desktop->width();
    bool Hidpi = gs->get("hidpi").toBool();
    qDebug()<< "Hidpi is "<<Hidpi;
    if(Hidpi){
        gdk_scale = gs->get("gdk-scale").toInt();
        qt_scale_factor = gs->get("qt-scale-factor").toInt();
    }else{
        int i = 1;
        if(desktop->height() >= 2000)
            i = 2;
        gdk_scale = i;
        qt_scale_factor = i;
    }
    QString qt1 = QString::number(qt_scale_factor);
    QByteArray qt2;
    qt2.append(qt1);
    QString gdk1 = QString::number(gdk_scale);
    QByteArray gdk2;
    gdk2.append(gdk1);

    qDebug()<< "gdk_scale"<<gdk2<<"qt_scale_factor"<<qt2;
    qputenv("XDG_CURRENT_DESKTOP","UKUI");
    qputenv("GDK_SCALE",gdk2);
    qputenv("QT_SCALE_FACTOR",qt2);
    qputenv("QT_AUTO_SCRENN_SET_FACTOR","0");
}

void SessionApplication::updatevalue(){
    const int time = gs->get("idle-delay").toInt() * 60;
    mIdleWatcher->reset(time);
}

void SessionApplication::registerDBus()
{
    new SessionDBusAdaptor(modman);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (!dbus.registerService(QStringLiteral("org.gnome.SessionManager")))
    {
        qCritical() << "Can't register org.gnome.SessionManager, there is already a session manager!";
    }
    if (!dbus.registerObject(("/org/gnome/SessionManager"), modman))
    {
        qCritical() << "Can't register object, there is already an object registered at "
                    << "/org/gnome/SessionManager";
    }

    const int timeout = gs->get("idle-delay").toInt() * 60;
    connect(gs,&QGSettings::changed,this,&SessionApplication::updatevalue);
    mIdleWatcher = new IdleWatcher(timeout);
    new IdleDBusAdaptor(mIdleWatcher);
    if (!dbus.registerObject("/org/gnome/SessionManager/Presence", mIdleWatcher))
    {
        qCritical() << "Cant' register object, there is already an object registered at "
                    << "org/gnome/SessionManager/Presence";
    }
}

SessionApplication::SessionApplication(int& argc, char** argv) :
    QApplication(argc, argv)
{
    gs = new QGSettings("org.ukui.session","/org/ukui/desktop/session/",this);

    InitialEnvironment();

    modman = new ModuleManager();

    // Wait until the event loop starts
    QTimer::singleShot(0, this, SLOT(startup()));

    playmusic();
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

void SessionApplication::playmusic(){
    QMediaPlayer *player = new QMediaPlayer;
    player->setMedia(QUrl("qrc:/startup.wav"));
    player->play();
    QObject::connect(player,&QMediaPlayer::stateChanged,[=](QMediaPlayer::State state){
        player->stop();
        player->deleteLater();
        //delete player;
        qDebug() << "play state is " << state;
    });

}
