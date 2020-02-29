/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 * Copyright: 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "sessionapplication.h"

#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QMediaPlayer>
#include <QSoundEffect>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session/ukui-session.log";
    if (!QFile::exists(logPath)) {
        return;
    }
    QByteArray localMsg = msg.toLocal8Bit();
    QDateTime dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    QString logMsg;
    switch (type) {
    case QtDebugMsg:
        logMsg = QString("%1 Debug: %2 (%3:%4, %5)\n").arg(time.constData()).arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtInfoMsg:
        logMsg = QString("%1 Info: %2 (%3:%4, %5)\n").arg(time.constData()).arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtWarningMsg:
        logMsg = QString("%1 Warning: %2 (%3:%4, %5)\n").arg(time.constData()).arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtCriticalMsg:
        logMsg = QString("%1 Critical: %2 (%3:%4, %5)\n").arg(time.constData()).arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
        break;
    case QtFatalMsg:
        logMsg = QString("%1 Fatal: %2 (%3:%4, %5)\n").arg(time.constData()).arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
        break;
    }

    QFile logFile(logPath);
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&logFile);
    ts << logMsg << endl;
    logFile.flush();
    logFile.close();
}

void playmusic(){
//    //加载开机音乐
//    QSoundEffect *soundplayer = new QSoundEffect();
//    soundplayer->setSource(QUrl("qrc:/startup.wav"));
//    soundplayer->play();
//    //Qtimer timer = new QTimer();
    QMediaPlayer *player = new QMediaPlayer;
        //QString path("qrc:/startup.wav");
        player->setMedia(QUrl("qrc:/startup.wav"));
        //qDebug() << path1 << player->state() << player->mediaStatus();
        player->play();
        QObject::connect(player,&QMediaPlayer::stateChanged,[=](QMediaPlayer::State state){
            qDebug() <<"sssssssssssssssssssssssss";
           //delete player;
           qDebug() << "play state is " << state;
        });

}

int main(int argc, char **argv)
{
    qInstallMessageHandler(myMessageOutput);
    qDebug() << "UKUI session manager start.";
    SessionApplication app(argc, argv);

    //playmusic();

    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
