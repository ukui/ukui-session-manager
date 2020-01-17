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

class ModuleManager;
class IdleWatcher;

class SessionApplication : public QApplication
{
    Q_OBJECT
public:
    SessionApplication(int& argc, char** argv);
    ~SessionApplication();

private Q_SLOTS:
    bool startup();
    void settingsChanged(QString path);
    void registerDBus();

private:
    void InitSettings();

    ModuleManager* modman;
    IdleWatcher* mIdleWatcher;
    QFileSystemWatcher *mSettingsWatcher;
    QSettings *mSettings;
    QWidget *widget;
};

#endif // SESSIONAPPLICATION_H
