/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *               2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef SESSIONAPPLICATION_H
#define SESSIONAPPLICATION_H

#include <QApplication>
#include <QFileSystemWatcher>
#include <QSettings>
#include <QGSettings/QGSettings>
#include "mainwindow.h"
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/

#ifdef signals
#undef signals
#endif

class ModuleManager;
class IdleWatcher;

class SessionApplication : public QApplication
{
    Q_OBJECT
public:
    SessionApplication(int& argc, char** argv);
    mainwindow *window;
    ~SessionApplication();

private Q_SLOTS:
    bool startup();
    void registerDBus();
    void updateIdleDelay();

private:
    void InitialEnvironment();

    bool gsettings_usable;
    QGSettings * gs;
    ModuleManager* modman;
    IdleWatcher* mIdleWatcher;
    //QWidget *widget;
    void playBootMusic();
};

#endif // SESSIONAPPLICATION_H
