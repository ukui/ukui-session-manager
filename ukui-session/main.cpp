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
#include <QFileInfo>
#include <QScreen>
#include <QProcess>
#include <QGSettings/QGSettings>


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

/* 判断文件是否存在 */
bool isFileExist(QString XresourcesFile)
{
    QFileInfo fileInfo(XresourcesFile);
    if(fileInfo.isFile()){
        qDebug()<<"file is true";
        return true;
    }
    qDebug()<<"file is false";
    return false;
}
/* 写配置文件并设置DPI和鼠标大小*/
void WriteXresourcesFile(QString XresourcesFile)
{
    QFile file(XresourcesFile);
    QString content = "Xft.dpi:192\nXcursor.size:48";
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    QByteArray str = content.toLatin1().data();
    file.write(str);
    file.close();
    QGSettings *gs = new QGSettings("org.ukui.font-rendering");
    QGSettings *settings = new QGSettings("org.ukui.SettingsDaemon.plugins.xsettings");
    gs->set("dpi",96.0);
    settings->set("scaling-factor",2);
    delete settings;
    delete gs;
}
/* 配置新装系统、新建用户第一次登陆时，4K缩放功能*/
void Set4KScreenScale()
{
    int ScreenNum = QApplication::screens().length();
    for (int i=0;i<ScreenNum;i++)
    {
        QScreen *screen = QApplication::screens().at(i);
        int height = screen->size().height();
        if(height > 2000){
                bool res;
                QString homePath = getenv("HOME");
                QString XresourcesFile = homePath+"/.Xresources";
                res = isFileExist(XresourcesFile);
                if(!res)
                    WriteXresourcesFile(XresourcesFile);
        }
    }
}

int main(int argc, char **argv)
{
    qInstallMessageHandler(myMessageOutput);
    qDebug() << "UKUI session manager start.";
    SessionApplication app(argc, argv);

    Set4KScreenScale();

    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
