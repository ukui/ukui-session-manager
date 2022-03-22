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
#include "loginedusers.h"
#include "lockchecker.h"
#include <QPushButton>

#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <ukui-log4qt.h>

#ifdef signals
#undef signals
#endif

/*菜单栏调用睡眠且有inhibitor阻塞时调用此函数进行消息提示*/
bool sleepInhibitorCheck(int doaction)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);

    QString message;
    QStringList Inhibitors;
    QStringList Reason;
    LockChecker::getSleepInhibitors(Inhibitors, Reason);

    for (int i = 0; i < Inhibitors.length(); ++i) {
        QString num            = QString("%1").arg(Inhibitors.at(i));
        QString reason         = QString("%1 \n").arg(Reason.at(i));
        QString inhibitMessage = "";
        if(doaction == 1)//主要是为了睡眠、休眠中午翻译上作区分
            inhibitMessage = num + QObject::tr(" is block system") + QObject::tr(" into sleep for reason ") + reason;//休眠
        else
            inhibitMessage = num + QObject::tr(" is block system ") + QObject::tr("into sleep for reason ") + reason;//睡眠

        message.append(std::move(inhibitMessage));
    }
    QString messageStr = "";
    if(doaction == 1)
        messageStr = QObject::tr("Are you sure") + QObject::tr(" you want to get system into sleep?");//休眠
    else
        messageStr = QObject::tr("Are you sure you want to get system into sleep?");//睡眠
    message.append(std::move(messageStr));
    msgBox.setText(message);
    QPushButton *cancelButton = msgBox.addButton(QObject::tr("cancel"), QMessageBox::ActionRole);
    QPushButton *confirmButton = msgBox.addButton(QObject::tr("confirm"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == cancelButton) {
        return false;
    } else if (msgBox.clickedButton() == confirmButton) {
        return true;
    } else {
        return false;
    }
}

/*菜单栏调用重启或关机且有inhibitor阻塞时调用此函数进行消息提示*/
bool shutdownInhibitorCheck(int doaction)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);

    QString message;
    QStringList Inhibitors;
    QStringList Reason;
    LockChecker::getShutdownInhibitors(Inhibitors, Reason);

    for(int i = 0; i < Inhibitors.length(); ++i) {
        QString num            = QString("%1").arg(Inhibitors.at(i));
        QString reason         = QString("%1 \n").arg(Reason.at(i));
        QString inhibitMessage = "";
        if(doaction == 5)
            inhibitMessage = num + QObject::tr(" is block system into reboot for reason ") + reason;//重启
        else
            inhibitMessage = num + QObject::tr(" is block system into shutdown for reason ") + reason;//关机

        message.append(std::move(inhibitMessage));
    }
    QString messageStr = "";
    if(doaction == 5)
        messageStr = QObject::tr("Are you sure you want to reboot?");//重启
    else
        messageStr = QObject::tr("Are you sure you want to shutdown?");//关机

    message.append(std::move(messageStr));
    msgBox.setText(message);
    QPushButton *cancelButton = msgBox.addButton(QObject::tr("cancel"), QMessageBox::ActionRole);
    QPushButton *confirmButton = msgBox.addButton(QObject::tr("confirm"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == cancelButton) {
        return false;
    } else if (msgBox.clickedButton() == confirmButton) {
        return true;
    } else {
        return false;
    }
}

bool messageboxCheck()
{
    QMessageBox msgBox;
//    msgBox.setWindowTitle(QObject::tr("conform"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
//    msgBox.setModal(false);
    msgBox.setText(QObject::tr("Multiple users are logged in at the same time.Are you sure you want to close this system?"));
    QPushButton *cancelButton = msgBox.addButton(QObject::tr("cancel"), QMessageBox::ActionRole);
    QPushButton *confirmButton = msgBox.addButton(QObject::tr("confirm"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == cancelButton) {
        qDebug() << "cancel!";
        return false;
    } else if (msgBox.clickedButton() == confirmButton) {
        qDebug() << "confirm!";
        return true;
    } else {
        return false;
    }
}

void messagecheck()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    QString t1 = QObject::tr("System update or package installation in progress,this function is temporarily unavailable.");
    QString t2 = QObject::tr("System backup or restore in progress,this function is temporarily unavailable.");

    QFile file_update("/tmp/lock/kylin-update.lock");
    QFile file_backup("/tmp/lock/kylin-backup.lock");

    if (file_update.exists()) {
        msgBox.setText(t1);
    }
    if (file_backup.exists()) {
        msgBox.setText(t2);
    }

    QPushButton *cancelButton = msgBox.addButton(QObject::tr("OK"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == cancelButton) {
        qDebug() << "OK!";
    }
}

bool playShutdownMusic(UkuiPower &powermanager, int num, int cc, QTimer *up_to_time)
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
//        if (num == 4) {
//            msg = dbus.call("emitStartLogout");
//        }

        if (num == 0) {
            msg = dbus.call("emitPrepareForSwitchuser");
        }

        if (!msg.errorName().isEmpty()) {
            qWarning() << "Dbus error: " << msg;
        }
    }

    if (play_music) {
        // up_to_time and soundplayer can not be define out of this if().
        // otherwise run ukui-session-tools --suspend with segmente error.
        // because they will be delate at the end of the playShutdownMusic().
        QObject::connect(up_to_time, &QTimer::timeout, [&]() {
            if (powermanager.canAction(UkuiPower::Action(action))) {
                powermanager.doAction(UkuiPower::Action(action));
            }
            exit(0);
        });

        QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
        if (num == 5 || num == 6) {
            if (xdg_session_type == "wayland") {
                QProcess::startDetached("paplay --volume=23456 /usr/share/ukui/ukui-session-manager/shutdown.wav");
            } else {
                QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/shutdown.wav");
            }
            up_to_time->start(5000);
        } else if (num == 4) {
            if (xdg_session_type == "wayland") {
                QProcess::startDetached("paplay --volume=23456 /usr/share/ukui/ukui-session-manager/logout.wav");
            } else {
                QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/logout.wav");
            }
            up_to_time->start(2000);
        } else {
            qDebug() << "error num";
            return false;
        }
    } else {
        if (powermanager.canAction(UkuiPower::Action(action))) {
            powermanager.doAction(UkuiPower::Action(action));
        }
        exit(0);
    }
    return false;
}

/**
 * @brief 判断当前是否处于锁屏
 */
bool screensaverIsActive()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.ukui.ScreenSaver",
                                                          "/",
                                                          "org.ukui.ScreenSaver",
                                                          "GetLockState");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        //从返回参数获取返回值
        bool isActive = response.arguments()[0].toBool();
        qDebug() << "screensaver isActive:" << isActive;
        return isActive;
    }
    else
    {
        qDebug() << "QDBusMessage failed!";
        return false;
    }
}


int main(int argc, char* argv[])
{
    initUkuiLog4qt("ukui-session-tools");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))

  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

#endif

    QApplication a(argc, argv);

//    int cc = check_lock();
    int cc = LockChecker::checkLock();
    qDebug() << cc << "   cc";

    // Load ts files
    const QString locale = QLocale::system().name();
    QTranslator   translator;
    qDebug() << "local: " << locale;
    qDebug() << "path: " << QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager");
    if (translator.load(locale, QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager"))) {
        a.installTranslator(&translator);
    } else {
        qDebug() << "Load translations file failed!";
    }

    UkuiPower powermanager(&a);
    bool      flag = true;

    // define in the main() avoid scope error.
    QTimer *up_to_time = new QTimer();
    up_to_time->setSingleShot(true);

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

    parser.process(a);

    if (parser.isSet(switchuserOption)) {
        flag = playShutdownMusic(powermanager, 0, cc, up_to_time);
    }
    if (parser.isSet(hibernateOption)) {
        if (LockChecker::isSleepBlocked()) {
            if (sleepInhibitorCheck(1)) {
                flag = playShutdownMusic(powermanager, 1, cc, up_to_time);
            } else {
                return 0;
            }
        } else {
            flag = playShutdownMusic(powermanager, 1, cc, up_to_time);
        }
    }
    if (parser.isSet(suspendOption)) {
        if (LockChecker::isSleepBlocked()) {
            if (sleepInhibitorCheck(2)) {
                flag = playShutdownMusic(powermanager, 2, cc, up_to_time);
            } else {
                return 0;
            }
        } else {
            flag = playShutdownMusic(powermanager, 2, cc, up_to_time);
        }
    }
    if (parser.isSet(logoutOption)) {
        flag = playShutdownMusic(powermanager, 4, cc, up_to_time);
    }
    if (parser.isSet(rebootOption)) {
        if (LockChecker::isShutdownBlocked()) {//有inhibitor的情况下
            if (shutdownInhibitorCheck(5)) {//先提醒inhibitor
                if (LockChecker::getLoginedUsers().count() > 1) {//再提醒多个用户登录的情况
                    if (messageboxCheck()) {
                        flag = playShutdownMusic(powermanager, 5, cc, up_to_time);
                    } else {
                        return 0;
                    }
                } else {
                    flag = playShutdownMusic(powermanager, 5, cc, up_to_time);
                }
            } else {
                return 0;
            }

        } else {//没有inhibitor的情况下
            if (LockChecker::getLoginedUsers().count() > 1) {//提醒多个用户登录的情况
                if (messageboxCheck()) {
                    flag = playShutdownMusic(powermanager, 5, cc, up_to_time);
                } else {
                    return 0;
                }
            } else {
                flag = playShutdownMusic(powermanager, 5, cc, up_to_time);
            }
        }

    }
    if (parser.isSet(shutdownOption)) {
        if (LockChecker::isShutdownBlocked()) {
            if (shutdownInhibitorCheck(6)) {
                if (LockChecker::getLoginedUsers().count() > 1) {
                    if (messageboxCheck()) {
                        flag = playShutdownMusic(powermanager, 6, cc, up_to_time);
                    } else {
                        return 0;
                    }
                } else {
                    flag = playShutdownMusic(powermanager, 6, cc, up_to_time);
                }
            } else {
                return 0;
            }
        } else {
            if (LockChecker::getLoginedUsers().count() > 1) {//提醒多个用户登录的情况
                if (messageboxCheck()) {
                    flag = playShutdownMusic(powermanager, 6, cc, up_to_time);
                } else {
                    return 0;
                }
            } else {
                flag = playShutdownMusic(powermanager, 6, cc, up_to_time);
            }
        }

    }

    if (flag && !screensaverIsActive()) {
        // Load qss file
        MainWindow *w = new MainWindow(lock_file, lock_user);
        QFile qss(":/powerwin.qss");
        qss.open(QFile::ReadOnly);
        a.setStyleSheet(qss.readAll());
        qss.close();

        w->showFullScreen();
        QObject::connect(w, &MainWindow::signalTostart, [&]() {
            // 从界面点击 切换用户 按钮，则等待界面隐藏再执行命令
            if (w->defaultnum == 0) {
                w->hide();
                QTimer::singleShot(500, [&]() {
                    playShutdownMusic(powermanager, w->defaultnum, cc, up_to_time);
                });
            } else
                playShutdownMusic(powermanager, w->defaultnum, cc, up_to_time);
        });
    }
    return a.exec();
}
