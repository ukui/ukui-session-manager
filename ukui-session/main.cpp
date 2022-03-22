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
#include <QCommandLineParser>
#include <QTranslator>
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

extern UKUISMServer*& getGlobalServer();

void IoErrorHandler(IceConn iceConn)
{
    getGlobalServer()->ioError(iceConn);
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + "/ukui-session/ukui-session-xsmp.log";
    //判断log文件是否存在
    if (!QFile::exists(logPath)) {
        QString logDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session";
        //不存在时，创建ukui-session文件夹
        QDir dir(logDir);
        if (!dir.exists(logDir)) {
            if (!dir.mkdir(logDir)) {
                return;
            }
        }
        //创建log文件
        QFile file(logPath);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        file.close();
    }
    if (!QFile::exists(logPath)) {
        return;
    }

    QByteArray localMsg = msg.toLocal8Bit();
    QDateTime  dateTime = QDateTime::currentDateTime();
    QByteArray time = QString("[%1] ").arg(dateTime.toString("MM-dd hh:mm:ss.zzz")).toLocal8Bit();
    QString    logMsg;
    switch (type) {
    case QtDebugMsg:
        logMsg = QString("%1 Debug: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    case QtInfoMsg:
        logMsg = QString("%1 Info: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    case QtWarningMsg:
        logMsg = QString("%1 Warning: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    case QtCriticalMsg:
        logMsg = QString("%1 Critical: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    case QtFatalMsg:
        logMsg = QString("%1 Fatal: %2 (%3:%4, %5)\n")
                     .arg(time.constData())
                     .arg(localMsg.constData())
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function);
        break;
    }

    //clear file content when it is too large
    QFile file(logPath);
    qint64 fileSize = file.size();
    if (fileSize >= 1024 * 1024 * 10) {
        file.open(QFile::WriteOnly | QFile::Truncate);
        file.flush();
        file.close();
    }

    QFile logFile(logPath);
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&logFile);
    ts << logMsg << endl;
    logFile.flush();
    logFile.close();
}

/* 过滤低分辨率高缩放比情况 */
void screenScaleJudgement(QGSettings *settings)
{
    qreal        scaling = qApp->devicePixelRatio();
    double       scale;
    scale = settings->get(SCALING_KEY).toDouble();
    if (scale > 1.25) {
        bool state  = false;
        bool mScale = false;

        for (QScreen *screen : QGuiApplication::screens()) {
            int width  = screen->geometry().width() * scaling;
            int height = screen->geometry().height() * scaling;

            if (width < 1920 && height < 1080) {
                state = true;
            } else if (width == 1920 && height == 1080 && scale > 1.5) {
                state = true;
            } else if (width > 2560 && height > 1440) {
                mScale = true;
            }
        }

        if (state && !mScale) {
            QGSettings *mGsettings = new QGSettings(MOUSE_SCHEMA);
            mGsettings->set(CURSOR_SIZE, 24);
            settings->set(SCALING_KEY, 1.0);
            delete mGsettings;
        }
    }
}

/* 设置DPI环境变量 */
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
    XCloseDisplay(dpy);

    qDebug() << "setXresources：" << str;

    delete mouse_settings;
}

/* 判断文件是否存在 */
bool isFileExist(QString XresourcesFile)
{
    QFileInfo fileInfo(XresourcesFile);
    if (fileInfo.isFile()) {
        qDebug() << "File exists";
        return true;
    }

    qDebug() << "File does not exis";

    return false;
}

/* 编写判断标志文件，更改 鼠标/DPI 配置大小*/
void writeXresourcesFile(QString XresourcesFile, QGSettings *settings, double scaling)
{
    QFile file(XresourcesFile);
    QString content = QString("Xft.dpi:%1\nXcursor.size:%2").arg(96.0 * scaling).arg(24.0 * scaling);
    QByteArray str = content.toLatin1().data();

    file.open(QIODevice::ReadWrite | QIODevice::Text);
    file.write(str);
    file.close();

    QGSettings *Font = new QGSettings("org.ukui.font-rendering");
    QGSettings *mouse_settings = new QGSettings(MOUSE_SCHEMA);

    Font->set("dpi", 96.0);
    settings->set(SCALING_KEY, scaling);
    mouse_settings->set(CURSOR_SIZE, scaling * 24.0);

    qDebug() << " writeXresourcesFile: content = " << content
             << " scalings = " << settings->get(SCALING_KEY).toDouble()
             << "cursor size = " << mouse_settings->get(CURSOR_SIZE).toInt();
    delete Font;
    delete mouse_settings;
}

/* 判断是否为首次登陆 */
bool isTheFirstLogin(QGSettings *settings)
{
    QString homePath       = getenv("HOME");
    QString XresourcesFile = homePath+"/.config/xresources";
    QString Xresources     = homePath+"/.Xresources";
    qreal   scaling        = qApp->devicePixelRatio();
    bool    zoom1 = false, zoom2 = false, zoom3 = false;
    double  mScaling;
    bool xres, Xres;

    Xres = isFileExist(Xresources);
    xres = isFileExist(XresourcesFile); //判断标志文件是否存在

    if (xres && !Xres) {
        return false;
    } else if (xres && Xres) {
        QFile::remove(Xresources);
        return false;
    } else if (Xres && !xres) {
        QFile::rename(Xresources, XresourcesFile);
        return false;
    }

    for (QScreen *screen : QGuiApplication::screens()) {
        int width  = screen->geometry().width() * scaling;
        int height = screen->geometry().height() * scaling;

        if (width <= 1920 && height <= 1080) {
            zoom1 = true;
        } else if (width > 1920 && height > 1080 && width <= 2560 && height <=1500) {
            zoom2 = true;
        } else if (width > 2560 && height > 1440) {
            zoom3 = true;
        }
    }

    if (zoom1) {
        mScaling = 1.0;
    } else if (!zoom1 && zoom2) {
        mScaling = 1.5;
    } else if (!zoom1 && !zoom2 && zoom3) {
        mScaling = 2.0;
    }

    writeXresourcesFile(XresourcesFile, settings, mScaling);

    return true;
}

/* 配置新装系统、新建用户第一次登陆时，4K缩放功能*/
void setHightResolutionScreenZoom()
{
    QGSettings *settings;
    double      dpi;
    int         ScreenNum = QApplication::screens().length();
    if (!QGSettings::isSchemaInstalled(XSETTINGS_SCHEMA) || !QGSettings::isSchemaInstalled("org.ukui.font-rendering") ||
            !QGSettings::isSchemaInstalled(MOUSE_SCHEMA)) {
        qDebug() << "Error: ukui-settings-daemon's Schema  is not installed, will not setting dpi!";
        delete settings;
        return;
    }
    settings = new QGSettings(XSETTINGS_SCHEMA);

    if (isTheFirstLogin(settings)) {
        qDebug() << "Set the default zoom value when logging in for the first time.";
        goto end;
    }
    /* 过滤单双屏下小分辨率大缩放值 */

    if (ScreenNum > 1) {
        goto end;
    }

    screenScaleJudgement(settings);

end:
    dpi = 0.0;
    dpi = settings->get(SCALING_KEY).toDouble() * 96.0;
    setXresources(dpi);
    delete settings;
}

bool require_dbus_session()
{
    QString env_dbus = qgetenv("DBUS_SESSION_BUS_ADDRESS");
    if (!env_dbus.isEmpty()) return true;
    qDebug() << "Fatal DBus Error";
    QProcess *a  = new QProcess;
    a->setProcessChannelMode(QProcess::ForwardedChannels);
    a->start("dbus-launch", QStringList() << "--exit-with-session" << "ukui-session");
    a->waitForFinished(-1);
    if (a->exitCode()) {
        qWarning() <<  "exited with code" << a->exitCode();
    }
    delete a;
    return true;
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

    require_dbus_session();

    qputenv("QT_QPA_PLATFORM", "xcb");

    SessionApplication app(argc, argv);

    setHightResolutionScreenZoom();

    app.setQuitOnLastWindowClosed(false);

    IceSetIOErrorHandler(IoErrorHandler);
    getGlobalServer()->restoreWM(QStringLiteral("saved at previous logout"));//恢复会话启动的窗管包含命令行参数
//    server->startDefaultSession();//默认方式启动的窗管不含任何命令行参数

    // Load ts files
    const QString locale = QLocale::system().name();
    QTranslator   translator;
    qDebug() << "local: " << locale;
    qDebug() << "path: " << QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager");
    if (translator.load(locale, QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager"))) {
        app.installTranslator(&translator);
    } else {
        qDebug() << "Load translations file failed!";
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("UKUI Session Manager"));

    const QString VERINFO = QStringLiteral("2.0.11-7");
    app.setApplicationVersion(VERINFO);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);
    
    return app.exec();
}
