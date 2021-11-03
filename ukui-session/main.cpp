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
#include "ukuismserver.h"
#include "ukuisessiondebug.h"

#include <signal.h>

#include <QStandardPaths>
#include <QDBusInterface>
#include <QFile>
#include <QDir>
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
#include <ukui-log4qt.h>
extern "C" {
#include <X11/Xatom.h>
#include <X11/Xlib.h>
}

#define XSETTINGS_SCHEMA    "org.ukui.SettingsDaemon.plugins.xsettings"
#define MOUSE_SCHEMA        "org.ukui.peripherals-mouse"
#define SCALING_KEY         "scaling-factor"
#define CURSOR_SIZE         "cursor-size"
#define CURSOR_THEME        "cursor-theme"

extern UKUISMServer *theServer;

void IoErrorHandler(IceConn iceConn)
{
    theServer->ioError(iceConn);
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QDir dir;
    QString logFilePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session";
    //先创建目录
    if (!dir.mkpath(logFilePath)) {
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

    //clear file content when it is too large
    logFilePath = logFilePath + "/ukui-session-xsmp.log";
    QFile file(logFilePath);
    qint64 fileSize = file.size();
    if (fileSize >= 1024 * 1024 * 10) {
        file.open(QFile::WriteOnly | QFile::Truncate);
        file.flush();
        file.close();
    }

    QFile logFile(logFilePath);
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&logFile);
    ts << logMsg << endl;
    logFile.flush();
    logFile.close();
}

void screenScaleJudgement(QGSettings *settings)
{
    qreal        scaling = qApp->devicePixelRatio();
    double       scale;
    scale = settings->get(SCALING_KEY).toDouble();
    if (scale > 1.25) {
        bool state = false;
        for (QScreen *screen : QGuiApplication::screens()) {
            if (screen->geometry().width() * scaling < 1920 &&  screen->geometry().height() * scaling < 1080) {
                state = true;
            } else if (screen->geometry().width() * scaling == 1920 &&  screen->geometry().height() * scaling == 1080 && scale > 1.5) {
                state = true;
            }
        }
        if (state) {
            QGSettings *mGsettings;
            mGsettings = new QGSettings(MOUSE_SCHEMA);
            mGsettings->set(CURSOR_SIZE, 24);
            settings->set(SCALING_KEY, 1.0);
            delete mGsettings;
        }
    }
}

void setXresources(int dpi)
{
    Display    *dpy;
    QGSettings *mouse_settings = new QGSettings(MOUSE_SCHEMA);
    QString str = QString("Xft.dpi:\t%1\nXcursor.size:\t%2\nXcursor.theme:\t%3\n")
                         .arg(dpi)
                         .arg(mouse_settings->get(CURSOR_SIZE).toInt())
                         .arg(mouse_settings->get(CURSOR_THEME).toString());

    dpy = XOpenDisplay(NULL);
    XChangeProperty(dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER, XA_STRING, 8,
                    PropModeReplace, (unsigned char *) str.toLatin1().data(), str.length());

    delete mouse_settings;
    XCloseDisplay(dpy);
}

/* 判断文件是否存在 */
bool isFileExist(QString XresourcesFile)
{
    QFileInfo fileInfo(XresourcesFile);
    if (fileInfo.isFile()) {
        qCDebug(UKUI_SESSION) << "file is true";
        return true;
    }
    qCDebug(UKUI_SESSION) << "file is false";
    return false;
}
/* 写配置文件并设置DPI和鼠标大小*/
void writeXresourcesFile(QString XresourcesFile, QGSettings *settings)
{
    QFile file(XresourcesFile);
    QString content = "Xft.dpi:192\nXcursor.size:48";
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    QByteArray str = content.toLatin1().data();
    file.write(str);
    file.close();
    QGSettings *gs = new QGSettings("org.ukui.font-rendering");
    QGSettings *mouse_settings = new QGSettings(MOUSE_SCHEMA);
    gs->set("dpi", 96.0);
    settings->set(SCALING_KEY, 2.0);
    mouse_settings->set(CURSOR_SIZE, 48);
    delete gs;
    delete mouse_settings;
}

/* 配置新装系统、新建用户第一次登陆时，4K缩放功能*/
void set4KScreenScale()
{
    QGSettings *settings;
    int ScreenNum = QApplication::screens().length();
    settings = new QGSettings(XSETTINGS_SCHEMA);

    /* 过滤单双屏下小分辨率大缩放值 */
    screenScaleJudgement(settings);
    double dpi = settings->get(SCALING_KEY).toDouble() * 96;
    if (ScreenNum > 1) {
        setXresources(dpi);
        delete settings;
        return;
    }
    QScreen *screen = QApplication::screens().at(0);
    int height = screen->size().height() * qApp->devicePixelRatio();
    int width = screen->size().width() * qApp->devicePixelRatio();
    if (height > 1500 && width > 2560) {
        bool res;
        QString homePath = getenv("HOME");
        QString XresourcesFile = homePath+"/.config/xresources";
        res = isFileExist(XresourcesFile);
        if (!res) {
            writeXresourcesFile(XresourcesFile, settings);
        }
    }
    double Dpi = settings->get(SCALING_KEY).toDouble() * 96.0;
    setXresources(Dpi);
    delete settings;
}

void signalHandler(int sig)
{
    QDBusInterface face("org.freedesktop.login1",
                        "/org/freedesktop/login1/user/self",
                        "org.freedesktop.login1.User",
                        QDBusConnection::systemBus());

    face.call("Kill", 9);
}

void openDubug()
{
//#ifdef QT_NO_DEBUG
//    UKUI_SESSION().setFilterRules(QLatin1Literal("org.ukui.ukuisession=false"));
//#else
//    UKUI_SESSION().setFilterRules(QLatin1Literal("org.ukui.ukuisession=true"));
//#endif
    UKUI_SESSION().setFilterRules(QLatin1Literal("org.ukui.ukuisession=true"));
    qInstallMessageHandler(myMessageOutput);
    qCDebug(UKUI_SESSION) << "===================================================    UKUI session manager start.    ===================================================";
}

int main(int argc, char **argv)
{
//    initUkuiLog4qt("ukui-session");
    //加上这个处理会在终端一直输出信息，原因不明
//    signal(SIGTERM, signalHandler);

    openDubug();

    SessionApplication app(argc, argv);

    set4KScreenScale();

    app.setQuitOnLastWindowClosed(false);
    UKUISMServer *server = new UKUISMServer;
    IceSetIOErrorHandler(IoErrorHandler);
    server->restoreSession(QStringLiteral("saved at previous logout"));//恢复会话启动的窗管包含命令行参数
//    server->startDefaultSession();//默认方式启动的窗管不含任何命令行参数
    
    return app.exec();
}
