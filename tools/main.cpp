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
#include <QPushButton>

#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
//#include <ukui-log4qt.h>

#ifdef signals
#undef signals
#endif

QString getName(QFile *a)
{
    QString user = getenv("USER");
    if (a->exists()) {
        a->open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream fileStream(a);
        int         k = 0;
        while (!fileStream.atEnd()) {
            QString line = fileStream.readLine();
            if (k == 0) {
                QString a = line;
                qDebug() << "uid=" << a;
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

bool messageboxCheck()
{
    QMessageBox msgBox;
    //    msgBox.setWindowTitle(QObject::tr("conform"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    //    msgBox.setModal(false);
    msgBox.setText(QObject::tr("Multiple users are logged in at the same time.Are you sure you "
                               "want to close this system?"));
    QPushButton *cancelButton  = msgBox.addButton(QObject::tr("cancel"), QMessageBox::ActionRole);
    QPushButton *confirmButton = msgBox.addButton(QObject::tr("confirm"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == cancelButton) {
        qDebug() << "cancel!";
        return false;
    } else if (msgBox.clickedButton() == confirmButton) {
        qDebug() << "confirm!";
        return true;
    } else
        return false;
}

void messagecheck()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(QObject::tr("notice"));
    QString t1 = QObject::tr("System update or package installation in progress,this function is "
                             "temporarily unavailable.");
    QString t2 = QObject::tr(
        "System backup or restore in progress,this function is temporarily unavailable.");

    QFile file_update("/tmp/lock/kylin-update.lock");
    QFile file_backup("/tmp/lock/kylin-backup.lock");

    if (file_update.exists()) {
        msgBox.setText(t1);
    }
    if (file_backup.exists()) {
        msgBox.setText(t2);
    }

    msgBox.exec();
}

int check_lock()
{
    bool lockfile = false;
    bool lockuser = false;

    QFile file_backup("/tmp/lock/kylin-backup.lock");
    QFile file_update("/tmp/lock/kylin-update.lock");
    if (file_backup.exists()) {
        int fd_backup = open(QString("/tmp/lock/kylin-backup.lock").toUtf8().data(), O_RDONLY);
        int b         = flock(fd_backup, LOCK_EX | LOCK_NB);
        qDebug() << "b" << b;
        if (b < 0) {
            lockfile          = true;
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
        int c         = flock(fd_update, LOCK_EX | LOCK_NB);
        qDebug() << "c" << c;
        if (c < 0) {
            lockfile          = true;
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
        if (lockuser)
            return 2;
        return 1;
    }
    return 0;
}

bool playShutdownMusic(UkuiPower &powermanager, int num, int cc, QTimer *up_to_time,
                       QSoundEffect *soundplayer)
{
    if (cc == 1) {
        if (num == 1 || num == 5 || num == 6) {
            messagecheck();
            exit(0);
        }
    } else if (cc == 2) {
        if (num == 1 || num == 4 || num == 5 || num == 6) {
            messagecheck();
            exit(0);
        }
    }

    bool        play_music = false;
    QGSettings *gs         = new QGSettings("org.ukui.session", "/org/ukui/desktop/session/");
    if (num == 4) {
        play_music = gs->get("logout-music").toBool();
    }
    if (num == 5 || num == 6) {
        play_music = gs->get("poweroff-music").toBool();
    }
    if (num == 0 || num == 1 || num == 2) {
        play_music = false;
    }

    gs->set("win-key-release", false);
    static int action = num;

    if (num == 4 || num == 0) {
        QDBusInterface dbus("org.gnome.SessionManager", "/org/gnome/SessionManager",
                            "org.gnome.SessionManager", QDBusConnection::sessionBus());
        if (!dbus.isValid()) {
            qWarning() << "dbusCall: QDBusInterface is invalid";
            return false;
        }

        QDBusMessage msg;
        if (num == 4)
            msg = dbus.call("emitStartLogout");

        if (num == 0)
            msg = dbus.call("emitPrepareForSwitchuser");

        if (!msg.errorName().isEmpty()) {
            qWarning() << "Dbus error: " << msg;
        }
    }

    if (play_music) {
        // up_to_time and soundplayer can not be define out of this if().
        // otherwise run ukui-session-tools --suspend with segmente error.
        // because they will be delate at the end of the playShutdownMusic().
        if (num == 5 || num == 6) {
            soundplayer->setSource(QUrl::fromLocalFile("/usr/share/ukui/ukui-session-manager/shutdown.wav"));
        } else if (num == 4) {
            soundplayer->setSource(QUrl::fromLocalFile("/usr/share/ukui/ukui-session-manager/logout.wav"));
        } else {
            qDebug() << "error num";
            return false;
        }
        // timeout set.
        QObject::connect(up_to_time, &QTimer::timeout, [&]() {
            powermanager.doAction(UkuiPower::Action(action));
            exit(0);
        });
        soundplayer->play();
        up_to_time->start(1200);
    } else {
        powermanager.doAction(UkuiPower::Action(action));
        exit(0);
    }
    return false;
}

bool require_dbus_session()
{
    QString env_dbus = qgetenv("DBUS_SESSION_BUS_ADDRESS");
    if (!env_dbus.isEmpty())
        return true;

    qDebug() << "Fatal DBus error, DBUS_SESSION_BUS_ADDRESS is empty.";
    QProcess a;
    a.setProcessChannelMode(QProcess::ForwardedChannels);
    a.start("dbus-launch", QStringList() << "--exit-with-session" << "ukui-session");
    a.waitForFinished(-1);
    if (a.exitCode()) {
        qWarning() << "dbus-launch exited with code" << a.exitCode();
    }
    return true;
}

int main(int argc, char *argv[])
{
 //   initUkuiLog4qt("ukui-session-tools");

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    require_dbus_session();

    QApplication a(argc, argv);

    int cc = check_lock();
    qDebug() << cc << "   cc";

    // Load ts files
    const QString locale = QLocale::system().name();
    QTranslator   translator;
    qDebug() << "local: " << locale;
    qDebug() << "path: " << QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager");
    if (translator.load(locale,
                        QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager"))) {
        a.installTranslator(&translator);
    } else {
        qDebug() << "Load translations file failed!";
    }

    UkuiPower powermanager(&a);
    bool      flag = true;

    // define in the main() avoid scope error.
    QTimer *up_to_time = new QTimer();
    up_to_time->setSingleShot(true);
    QSoundEffect *soundplayer = new QSoundEffect();
    soundplayer->setVolume(0.4);

    QGSettings *gs = new QGSettings("org.ukui.session", "/org/ukui/desktop/session/");
    gs->set("win-key-release", true);

    bool lock_file = false;
    bool lock_user = false;
    if (cc == 1) {
        lock_file = true;
    }
    if (cc == 2) {
        lock_file = true;
        lock_user = true;
    }
    MainWindow *w = new MainWindow(lock_file, lock_user);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QApplication::tr("UKUI session tools, show the shutdown dialog without any arguments."));
    const QString VERINFO = QStringLiteral("2.0");
    a.setApplicationVersion(VERINFO);

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption switchuserOption(QStringLiteral("switchuser"),
                                        QApplication::tr("Switch the user of this computer."));
    parser.addOption(switchuserOption);
    QCommandLineOption hibernateOption(QStringLiteral("hibernate"),
                                       QApplication::tr("Hibernate this computer."));
    parser.addOption(hibernateOption);
    QCommandLineOption suspendOption(QStringLiteral("suspend"),
                                     QApplication::tr("Suspend this computer."));
    parser.addOption(suspendOption);
    QCommandLineOption logoutOption(QStringLiteral("logout"),
                                    QApplication::tr("Logout this computer."));
    parser.addOption(logoutOption);
    QCommandLineOption rebootOption(QStringLiteral("reboot"),
                                    QApplication::tr("Restart this computer."));
    parser.addOption(rebootOption);
    QCommandLineOption shutdownOption(QStringLiteral("shutdown"),
                                      QApplication::tr("Shutdown this computer."));
    parser.addOption(shutdownOption);
    QCommandLineOption windowOption(QStringLiteral("window"),
                                    QApplication::tr("A window above the desktop."));
    parser.addOption(windowOption);

    parser.process(a);

    if (parser.isSet(switchuserOption)) {
        flag = playShutdownMusic(powermanager, 0, cc, up_to_time, soundplayer);
    }
    if (parser.isSet(hibernateOption)) {
        flag = playShutdownMusic(powermanager, 1, cc, up_to_time, soundplayer);
    }
    if (parser.isSet(suspendOption)) {
        flag = playShutdownMusic(powermanager, 2, cc, up_to_time, soundplayer);
    }
    if (parser.isSet(logoutOption)) {
        flag = playShutdownMusic(powermanager, 4, cc, up_to_time, soundplayer);
    }
    if (parser.isSet(rebootOption)) {
        if (w->getLoginedUsers().count() > 1) {
            if (messageboxCheck())
                flag = playShutdownMusic(powermanager, 5, cc, up_to_time, soundplayer);
            else {
                return 0;
            }
        } else
            flag = playShutdownMusic(powermanager, 5, cc, up_to_time, soundplayer);
    }
    if (parser.isSet(shutdownOption)) {
        if (w->getLoginedUsers().count() > 1) {
            if (messageboxCheck())
                flag = playShutdownMusic(powermanager, 6, cc, up_to_time, soundplayer);
            else {
                return 0;
            }
        } else
            flag = playShutdownMusic(powermanager, 6, cc, up_to_time, soundplayer);
    }
    if (parser.isSet(windowOption)) {
        flag        = false;
        window *win = new window();
        win->showFullScreen();
    }
    if (flag) {
        // Load qss file
        QFile qss(":/powerwin.qss");
        qss.open(QFile::ReadOnly);
        a.setStyleSheet(qss.readAll());
        qss.close();

        w->showFullScreen();
        w->repaint();
        QObject::connect(w, &MainWindow::signalTostart, [&]() {
            playShutdownMusic(powermanager, w->defaultnum, cc, up_to_time, soundplayer);
        });
    }

    return a.exec();
}
