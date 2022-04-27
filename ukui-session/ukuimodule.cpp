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
#include "ukuimodule.h"
#include "ukuismserver.h"

#include <QFileInfo>
#include <QDebug>
#include <modulemanager.h>

extern UKUISMServer*& getGlobalServer();

UkuiModule::UkuiModule(const XdgDesktopFile& file, QObject* parent) : QProcess(parent)
                                                                    , file(file)
                                                                    , fileName(QFileInfo(file.fileName()).fileName())
                                                                    , mIsTerminating(false)
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

    if (command == "ukui-kwin_x11") {
        getGlobalServer()->restoreWM(QStringLiteral("saved at previous logout"));
//        startWM(command, args);
    } else {
        qDebug() << "Start ukui module: " << command << "args: " << args;
        QProcess::start(command, args);
    }

    ModuleManager::insertStartupList(std::move(command));
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

void UkuiModule::startWM(QString &command, QStringList &args)
{
    QList<QStringList> wmStartCommand = getGlobalServer()->wmStartCommands();

    if (!wmStartCommand.empty()) {
        QStringList fullCommand = wmStartCommand[0];
        QString app = fullCommand[0];
        QStringList argList;
        for (int i = 1; i < fullCommand.count(); i++) {
            argList.append(fullCommand[i]);
        }

        QProcess::start(app, argList);
    } else {
        QProcess::start(command, args);
    }
}

void UkuiModule::updateState(QProcess::ProcessState newState)
{
    if (newState != QProcess::Starting)
        emit moduleStateChanged(fileName, (newState == QProcess::Running));
}
