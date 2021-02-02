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


#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef signals
#undef signals
#endif

bool messagecheck(){
    QMessageBox msgBox;
    msgBox.setWindowTitle(QObject::tr("notice"));
    msgBox.setText(QObject::tr("System update or package installation in progress,some functions are temporarily unavailable."));;

    msgBox.exec();
}

int check_lock(){
    bool lockfile = false;
    bool lockuser = false;
    QString user;
    QFile file_backup("/tmp/lock/kylin-backup.lock");
    QFile file_update("/tmp/lock/kylin-update.lock");
    if(file_backup.exists()){
        int fd_backup = open(QString("/tmp/lock/kylin-backup.lock").toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        int b = lockf(fd_backup, F_TLOCK, 0);
        qDebug()<<"b"<<b;
        if(b<0){
            lockfile = true;
            file_backup.open(QIODevice::ReadOnly | QIODevice::Text);
            QTextStream backup(&file_backup);
            int k = 0;
            while (!backup.atEnd()) {
                QString line = backup.readLine();
                if(k == 0){
                    QStringList list = line.split("(");
                    user = list[0];
                    if(user == qgetenv("USER")){
                        lockuser = true;
                    }
                }
                k++;
            }
        }
        file_backup.close();
    }
    if(file_update.exists()){
        int fd_update = open(QString("/tmp/lock/kylin-update.lock").toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        int c = lockf(fd_update, F_TLOCK, 0);
        qDebug()<<"c"<<c;
        if(c<0){
            lockfile = true;
            file_update.open(QIODevice::ReadOnly | QIODevice::Text);
            QTextStream update(&file_update);
            int j = 0;
            while (!update.atEnd()) {
                QString line = update.readLine();
                if(j == 0){
                    QStringList list = line.split("(");
                    user = list[0];
                    if(user == qgetenv("USER")){
                        lockuser = true;
                    }
                }
                j++;
            }
        }
        file_backup.close();
    }
    if(lockfile){
        if(lockuser)
            return 2;
        return 1;
    }
    return 0;
}

bool playShutdownMusic(UkuiPower &powermanager, int num ,int cc)
{
    if(cc == 1){
        if(num == 1 || num == 5 || num == 6){
            messagecheck();
            exit(0);
        }
    }else if(cc == 2 && num == 4){
        messagecheck();
        exit(0);
    }

    bool play_music = true;
    QGSettings *gs = new QGSettings("org.ukui.session","/org/ukui/desktop/session/");
    play_music = gs->get("boot-music").toBool();
    gs->set("win-key-release",false);
    if(num != 5 || num != 6){
        play_music = false;
    }
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

    int cc = check_lock();
    qDebug()<<cc<<"   cc";

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
    QCommandLineOption hibernateOption(QStringLiteral("hibernate"), QApplication::tr("Hibernate this computer."));
    parser.addOption(hibernateOption);
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
        flag = playShutdownMusic(powermanager, 0, cc);
    }
    if (parser.isSet(hibernateOption)) {
        flag = playShutdownMusic(powermanager, 1, cc);
    }
    if (parser.isSet(suspendOption)) {
        flag = playShutdownMusic(powermanager, 2, cc);
    }
    if (parser.isSet(logoutOption)) {
        flag = playShutdownMusic(powermanager, 4, cc);
    }
    if (parser.isSet(rebootOption)) {
        flag = playShutdownMusic(powermanager, 5, cc);
    }
    if (parser.isSet(shutdownOption)) {
        flag = playShutdownMusic(powermanager, 6, cc);
    }
    if (parser.isSet(windowOption)) {
        flag = false;
        window *win = new window();
        win->showFullScreen();
    }
    if (flag) {
        QGSettings *gs = new QGSettings("org.ukui.session","/org/ukui/desktop/session/");
        gs->set("win-key-release",true);

        bool lock_file = false;
        bool lock_user = false;
        if(cc == 1){
            lock_file = true;
        }
        if(cc == 2){
            lock_file = true;
            lock_user = true;
        }
        MainWindow *w = new MainWindow(lock_file,lock_user);

        // Load qss file
        QFile qss(":/powerwin.qss");
        qss.open(QFile::ReadOnly);
        a.setStyleSheet(qss.readAll());
        qss.close();

        w->showFullScreen();
        QObject::connect(w, &MainWindow::signalTostart, [&]()
        {
            playShutdownMusic(powermanager, w->defaultnum, cc);
        });
        //return a.exec();
    }
    return a.exec();
}
