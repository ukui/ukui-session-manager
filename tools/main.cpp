/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*               2010-2016 LXQt team
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301, USA.
**/
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QTranslator>
#include <QCommandLineParser>
#include <QString>
#include <QSoundEffect>
#include <QTimer>
#include <QGSettings/QGSettings>
#include <X11/Xlib.h>

#include <QMessageBox>
#include <QDBusInterface>
#include <QDBusReply>

#include "ukuipower.h"
#include "mainwindow.h"
#include "window.h"

#ifdef signals
#undef signals
#endif

bool playShutdownMusic(UkuiPower &powermanager, int num)
{
    bool play_music = true;
    QGSettings *gs = new QGSettings("org.ukui.session","/org/ukui/desktop/session/");
    play_music = gs->get("boot-music").toBool();
    gs->set("win-key-release",false);
    static int action = num;
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);

    if(num == 4){
        QDBusInterface dbus("org.gnome.SessionManager", "/org/gnome/SessionManager", "org.gnome.SessionManager", QDBusConnection::sessionBus());
        if (!dbus.isValid()) {
            qWarning() << "dbusCall: QDBusInterface is invalid";
            return false;
        }

        QDBusMessage msg = dbus.call("emitStartLogout");

        if (!msg.errorName().isEmpty()) {
            qWarning() << "Dbus error: " << msg;
        }
    }

    if (play_music) {
        QSoundEffect *soundplayer = new QSoundEffect();
        soundplayer->setSource(QUrl("qrc:/shutdown.wav"));
        soundplayer->play();

        QObject::connect(timer, &QTimer::timeout, [&]()
        {
            powermanager.doAction(UkuiPower::Action(action));
            exit(0);
        });
        timer->start(1000);
    } else {
        powermanager.doAction(UkuiPower::Action(action));
        exit(0);
    }
    return false;
}

int main(int argc, char* argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    // Load ts files
    const QString locale = QLocale::system().name();
    QTranslator translator;
    qDebug() << "local: " << locale;
    qDebug() << "path: " << QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager");
    if (translator.load(locale, QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager"))) {
       a.installTranslator(&translator);
    } else {
       qDebug() << "Load translations file failed!";
    }

    UkuiPower powermanager(&a);
    bool flag = true;

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("UKUI session tools, show the shutdown dialog without any arguments."));
    const QString VERINFO = QStringLiteral("2.0");
    a.setApplicationVersion(VERINFO);

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption switchuserOption(QStringLiteral("switchuser"), QApplication::tr("Switch the user of this computer."));
    parser.addOption(switchuserOption);
//    QCommandLineOption hibernateOption(QStringLiteral("hibernate"), QApplication::tr("Hibernate this computer."));
//    parser.addOption(hibernateOption);
    QCommandLineOption suspendOption(QStringLiteral("suspend"), QApplication::tr("Suspend this computer."));
    parser.addOption(suspendOption);
    QCommandLineOption logoutOption(QStringLiteral("logout"), QApplication::tr("Logout this computer."));
    parser.addOption(logoutOption);
    QCommandLineOption rebootOption(QStringLiteral("reboot"), QApplication::tr("Restart this computer."));
    parser.addOption(rebootOption);
    QCommandLineOption shutdownOption(QStringLiteral("shutdown"), QApplication::tr("Shutdown this computer."));
    parser.addOption(shutdownOption);
    QCommandLineOption windowOption(QStringLiteral("window"), QApplication::tr("A window above the desktop."));
    parser.addOption(windowOption);

    parser.process(a);

    if (parser.isSet(switchuserOption)) {
        flag = playShutdownMusic(powermanager, 0);
    }
//    if (parser.isSet(hibernateOption)) {
//        flag = playShutdownMusic(powermanager, 1);
//    }
    if (parser.isSet(suspendOption)) {
        flag = playShutdownMusic(powermanager, 2);
    }
    if (parser.isSet(logoutOption)) {
        flag = playShutdownMusic(powermanager, 4);
    }
    if (parser.isSet(rebootOption)) {
        flag = playShutdownMusic(powermanager, 5);
    }
    if (parser.isSet(shutdownOption)) {
        flag = playShutdownMusic(powermanager, 6);
    }
    if (parser.isSet(windowOption)) {
        flag = false;
        window *win = new window();
        win->showFullScreen();
    }
    if (flag) {
        QGSettings *gs = new QGSettings("org.ukui.session","/org/ukui/desktop/session/");
        gs->set("win-key-release",true);

        MainWindow *w = new MainWindow();

        // Load qss file
        QFile qss(":/powerwin.qss");
        qss.open(QFile::ReadOnly);
        a.setStyleSheet(qss.readAll());
        qss.close();

        w->showFullScreen();
        QObject::connect(w, &MainWindow::signalTostart, [&]()
        {
            playShutdownMusic(powermanager, w->defaultnum);
        });
        //return a.exec();
    }
    return a.exec();
}
