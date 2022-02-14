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

#ifndef MYICONLABEL_H
#define MYICONLABEL_H

#include <QMainWindow>
#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QFont>
#include <QAbstractNativeEventFilter>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QPushButton>
#include "pushbutton.h"
class MyIconLabel : public QLabel
{
    Q_OBJECT

public:
    MyIconLabel(int labelWidth, int iconWidth, QString path, bool showBackClor, QWidget *parent = nullptr);
    ~MyIconLabel();

    bool containsPoint(QPoint p);
protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    bool event(QEvent *);
    //void paintEvent(QPaintEvent *event);
    //void mouseMoveEvent(QMouseEvent *event);
    //void mousePressEvent(QMouseEvent* event);
private:
    PushButton *m_btnIcon = nullptr;
    bool m_showBackColor = false;
    QPoint m_point;
//    bool m_isNormal = true;
//    bool m_isHover = false;
//    bool m_isClicked = false;
protected:

public:

signals:
    void mouseEventSignals(QEvent *event);
};
#endif // MYICONLABEL_H
