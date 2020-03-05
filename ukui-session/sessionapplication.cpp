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

void InitialEnvironment()
{
    qputenv("XDG_CURRENT_DESKTOP","UKUI");
}

void SessionApplication::InitSettings()
{
    QString config_file = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session/ukui-session.ini";
    bool config_exists;
    if (QFile::exists(config_file))
        config_exists = true;
    else
        config_exists = false;

    mSettings = new QSettings(config_file, QSettings::IniFormat);

    if (!config_exists)
    {
        mSettings->setValue("WindowManager", "ukwm");
        mSettings->setValue("Panel", "ukui-panel");
        mSettings->setValue("FileManager", "peony-desktop");
        mSettings->setValue("Desktop", "");
        mSettings->setValue("ForceApplication", "");
        mSettings->setValue("InhibitApplication", "nm-applet");
        mSettings->setValue("IdleTimeSecs", 600);
        mSettings->sync();
    }

    QStringList config_list;
    config_list << config_file;
    mSettingsWatcher = new QFileSystemWatcher(config_list);
    connect(mSettingsWatcher, SIGNAL(fileChanged(QString)), this, SLOT(settingsChanged(QString)));
}

void SessionApplication::settingsChanged(QString path)
{
    qDebug() << "session manager settings changed!";
    mSettings->sync();
    int timeout = mSettings->value(QLatin1String("IdleTimeSecs")).toInt();
    //TODO
    mIdleWatcher->reset(timeout);
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

    int timeout = mSettings->value(QLatin1String("IdleTimeSecs")).toInt();
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
    InitialEnvironment();

    InitSettings();

    modman = new ModuleManager(mSettings);

    // Wait until the event loop starts
    QTimer::singleShot(0, this, SLOT(startup()));

    playmusic();
}

SessionApplication::~SessionApplication()
{
    delete modman;
    delete mIdleWatcher;
    //delete mSettings;
    delete mSettingsWatcher;
}

bool SessionApplication::startup()
{
    modman->startup();

    QTimer::singleShot(5 * 1000, this, SLOT(registerDBus()));

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
