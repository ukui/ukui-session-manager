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
#include <QTranslator>
#include <QCommandLineParser>
#include <QString>
#include <QSoundEffect>

#include "ukuipower.h"
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

//    //加载关机音乐
//    QSoundEffect *soundplayer = new QSoundEffect();
//    soundplayer->setSource(QUrl("qrc:/start.wav"));
//    //soundplayer->play();

    UkuiPower powermanager(&a);

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("UKUI session tools, show the shutdown dialog without any arguments."));
    const QString VERINFO = QStringLiteral("2.0");
    a.setApplicationVersion(VERINFO);

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption logoutOption(QStringLiteral("logout"), QApplication::tr("Logout this computer."));
    parser.addOption(logoutOption);
    QCommandLineOption shutdownOption(QStringLiteral("shutdown"), QApplication::tr("Shutdown this computer."));
    parser.addOption(shutdownOption);
    QCommandLineOption switchuserOption(QStringLiteral("switchuser"), QApplication::tr("Switch the user of this computer."));
    parser.addOption(switchuserOption);
    QCommandLineOption rebootOption(QStringLiteral("reboot"), QApplication::tr("Restart this computer."));
    parser.addOption(rebootOption);

    parser.process(a);

    if (parser.isSet(logoutOption)) {
        //soundplayer->play();
        powermanager.doAction(UkuiPower::Action(0));
        return 0;
    }
    if (parser.isSet(shutdownOption)) {
        //soundplayer->play();
        powermanager.doAction(UkuiPower::Action(4));
        return 0;
    }
    if (parser.isSet(switchuserOption)) {
        powermanager.doAction(UkuiPower::Action(1));
        return 0;
    }
    if (parser.isSet(rebootOption)) {
        //soundplayer->play();
        powermanager.doAction(UkuiPower::Action(3));
        return 0;
    }

    //加载翻译文件
    const QString locale = QLocale::system().name();
    QTranslator translator;
    qDebug() << "local: " << locale;
    qDebug() << "path: " << QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager");
    if (translator.load(locale, QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager"))) {
       a.installTranslator(&translator);
    } else {
       qDebug() << "Load translations file failed!";
    }

    MainWindow w ;

    //加载qss文件
    QFile qss(":/powerwin.qss");
    qss.open(QFile::ReadOnly);
    a.setStyleSheet(qss.readAll());
    qss.close();

    w.showFullScreen();

    return a.exec();
}
