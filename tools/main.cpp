#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QDesktopWidget>
#include <QTranslator>
#include <QCommandLineParser>
#include <QString>

#include "ukuipower.h"
#include "powerwin.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

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
        powermanager.doAction(UkuiPower::Action(0));
        return 0;
    }
    if (parser.isSet(shutdownOption)) {
        powermanager.doAction(UkuiPower::Action(4));
        return 0;
    }
    if (parser.isSet(switchuserOption)) {
        powermanager.doAction(UkuiPower::Action(1));
        return 0;
    }
    if (parser.isSet(rebootOption)) {
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

    PowerWin *pw;
    pw = new PowerWin(false);

    //加载qss文件
    QFile qss(":/powerwin.qss");
    qss.open(QFile::ReadOnly);
    a.setStyleSheet(qss.readAll());
    qss.close();

    QRect screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    int xx = screen.x();
    int yy = screen.y();//取得当前鼠标所在屏幕的最左，上坐标。

    pw->setWindowFlag(Qt::WindowStaysOnTopHint);//设置为顶层窗口，无法被切屏
    pw->showFullScreen();
    pw->move(xx+(screen.width() - pw->width())/2,yy+(screen.height()- pw->height())/2);

    return a.exec();
}
