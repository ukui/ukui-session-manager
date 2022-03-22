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

#include "myiconlabel.h"
#include <QPushButton>
#include <QPixmap>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QPoint>
#include <QMouseEvent>

MyIconLabel::MyIconLabel(int labelWidth, int iconWidth, QString path, QWidget *parent)
{
    this->setFixedSize(labelWidth, labelWidth);
    this->setObjectName("iconlabel");
    m_btnIcon = new PushButton(this);
    m_btnIcon->setIcon(QIcon(path));
    m_btnIcon->setIconSize(QSize(iconWidth, iconWidth));
    m_btnIcon->setObjectName("btn");
    m_btnIcon->setStyleSheet("QPushButton#btn{background-color: transparent;border:none;} \
                             QPushButton:checked  { background-color: transparent;border:none;}\
                             QPushButton:hover  { background-color: transparent;border:none;}");
    m_btnIcon->setGeometry(QRect((width() - iconWidth)/2, (width() - iconWidth)/2, iconWidth, iconWidth));
    m_btnIcon->setCheckable(false);
    m_btnIcon->setChecked(false);
    m_btnIcon->setGeometry(QRect((width() - iconWidth)/2, (width() - iconWidth)/2, iconWidth, iconWidth));
    m_btnIcon->setAttribute(Qt::WA_TransparentForMouseEvents,true);

    //mask不要与控件一样大  锯齿明显  稍微大一点
    this->setMask(QRegion(this->x() - 1, this->y() - 1, this->width() + 2, this->height() + 2,QRegion::Ellipse));
    this->setStyleSheet("QLabel#"+ this->objectName() + "{background-color: rgb(255,255,255,40);border-radius:" + QString::number(this->width()/2) + "px;}");

    //this->setPixmap(m_pixMap);
    this->setAlignment(Qt::AlignCenter);

   //setMouseTracking(true);
}

MyIconLabel::~MyIconLabel()
{
}

bool MyIconLabel::event(QEvent *event)
{
//    qDebug() << m_showBackColor << "event..." << event->type();
//    if (!m_showBackColor) {
//        return QWidget::event(event);
//    }
    if (event->type() == QEvent::MouseButtonPress) {
        QString str = "QLabel{background-color: rgb(255,255,255,100);border-radius: " + QString::number(this->width()/2) + "px;}";
        this->setStyleSheet(str);
        this->setAttribute(Qt::WA_StyledBackground);
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QString str = "QLabel{background-color: rgb(255,255,255,80);border-radius: " + QString::number(this->width()/2) + "px;}";
        this->setStyleSheet(str);
        this->setAttribute(Qt::WA_StyledBackground);
        emit mouseEventSignals(event);
    } else {
        emit mouseEventSignals(event);
    }
    return QWidget::event(event);
}

bool MyIconLabel::containsPoint(QPoint p)
{
    QPainterPath path;
    path.addEllipse(QRect(0, 0, width(), height()));
    //qDebug() << "containsPoint..." << width() << height() << p << QRect(0,0,width(),height());
    return path.contains(p);
}
