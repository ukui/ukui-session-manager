/*
 * Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "window.h"
#include <QScreen>
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QGSettings/QGSettings>

#define BACKGROUND_SETTINGS "org.mate.background"

window::window()
{
    // QString path = "/tmp/greeter-background.conf";
    QString fullstr;
    // QFile file(path);
    const QByteArray id(BACKGROUND_SETTINGS);
    //    if(file.exists()){
    //        QSettings qset(path,QSettings::NativeFormat);
    //        fullstr = qset.value("Greeter").toString();
    //    }
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *gset = new QGSettings(BACKGROUND_SETTINGS, "", this);
        fullstr          = gset->get("picture-filename").toString();
    } else
        fullstr = "/usr/share/backgrounds/default.jpg";
    qDebug() << "picture file is " << fullstr;
    pix.load(fullstr);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                   | Qt::X11BypassWindowManagerHint);
    setFixedSize(QApplication::primaryScreen()->virtualSize());
}

void window::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    for (QScreen *screen: QApplication::screens()) {
        // draw picture to every screen
        QRect rect = screen->geometry();
        painter.drawPixmap(rect, pix);
        painter.drawRect(rect);
    }
    QWidget::paintEvent(e);
}
