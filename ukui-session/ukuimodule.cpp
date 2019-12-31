#include "ukuimodule.h"

#include <QFileInfo>
#include <QDebug>

UkuiModule::UkuiModule(const XdgDesktopFile& file, QObject* parent) :
    QProcess(parent),
    file(file),
    fileName(QFileInfo(file.fileName()).fileName()),
    mIsTerminating(false)
{
    restartNum = 0;
    QProcess::setProcessChannelMode(QProcess::ForwardedChannels);
    connect(this, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(updateState(QProcess::ProcessState)));
}

void UkuiModule::start()
{
    mIsTerminating = false;
    QStringList args = file.expandExecString();
    QString command = args.takeFirst();
    qDebug() << "Start ukui module: " << command << "args: " << args;
    QProcess::start(command, args);
}

void UkuiModule::terminate()
{
    mIsTerminating = true;
    QProcess::terminate();
}

bool UkuiModule::isTerminating()
{
    return mIsTerminating;
}

void UkuiModule::updateState(QProcess::ProcessState newState)
{
    if (newState != QProcess::Starting)
        emit moduleStateChanged(fileName, (newState == QProcess::Running));
}
