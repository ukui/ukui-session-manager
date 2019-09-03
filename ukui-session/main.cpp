#include "sessionapplication.h"

#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
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
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ukui-session/ukui-session.log";
    QFile logFile(logPath);
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&logFile);
    ts << logMsg << endl;
}

int main(int argc, char **argv)
{
    qInstallMessageHandler(myMessageOutput);
    SessionApplication app(argc, argv);

    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
