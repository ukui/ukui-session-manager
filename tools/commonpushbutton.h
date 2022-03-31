/*
* Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301, USA.
**/

#ifndef COMMONPUSHBUTTON_H
#define COMMONPUSHBUTTON_H

#include <QMainWindow>
#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QFont>
#include <QAbstractNativeEventFilter>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>

class CommonPushButton : public QWidget
{
    Q_OBJECT

public:
    CommonPushButton(QString buttonText, QString objName, int width = 120, int height = 48, int radius = 24, QWidget *parent = nullptr);
    ~CommonPushButton();

    void setText(QString str);


protected:
    bool event(QEvent *);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);


signals:
    void clicked();
public slots:

private:
    QLabel *m_label = nullptr;
    int m_radius;
};
#endif // COMMONPUSHBUTTON_H
