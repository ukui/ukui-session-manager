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

#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QMainWindow>
#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QFont>
#include <QAbstractNativeEventFilter>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include "myiconlabel.h"
class MyPushButton : public QWidget
{
    Q_OBJECT

public:
    MyPushButton(QString iconPath, QString buttonLable, QString objName, QWidget *parent = nullptr, bool isRecBtn=true, int x = 0, int y = 0,
                 int width = 140, int height = 180, int iconWidth = 48,
                 int labelWidth = 128);
    ~MyPushButton();

    void changeIconBackColor(bool isChoose = true, bool isKeySelect = false);

    MyIconLabel* getIconLabel();

    bool getIsKeySelect();

    bool getIsMouseSelect();

    void setIsKeySelect(bool isKeySelect);

    void setIsMouseSelect(bool isMouseSelect);


protected:

private:
    QLabel *m_buttonLabel = nullptr;
    QFont m_buttonLabelFont;
    QPixmap m_pixMap;
    MyIconLabel *m_iconLabel = nullptr;
    bool m_isRecBtn=true;//true 背景为整体  false表示背景只在ICON区域
    bool m_isKeySelect = false;//是否通过键盘选中
    bool m_isMouseSelect = false;//是否鼠标选中
protected:

signals:
    void mouseRelase(QEvent *event, QString iconName);

public slots:
    void iconLabelMouseEvent(QEvent *event);

};
#endif // MYPUSHBUTTON_H
