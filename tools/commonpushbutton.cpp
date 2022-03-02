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

#include "commonpushbutton.h"
#include <QPixmap>
#include <QVBoxLayout>
#include <QDebug>
#include <QEvent>
CommonPushButton::CommonPushButton(QString buttonText, QString objName, int width, int height, QWidget *parent)
{
    m_label = new QLabel(this);
    m_label->setText(buttonText);
    this->setFixedSize(width, height);
    this->setObjectName(objName);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(0, 0, width, height);
    m_label->setStyleSheet("font:12pt;color:white");
    this->setStyleSheet("QWidget#" + this->objectName() + "{background-color: rgb(255,255,255,40);border-radius: " + QString::number(this->height()/2) + "px;}");
    this->setAttribute(Qt::WA_StyledBackground);
}

CommonPushButton::~CommonPushButton()
{
}

void CommonPushButton::setText(QString str)
{
    m_label->setText(str);
}

bool CommonPushButton::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QString str = "QWidget#" + this->objectName() +"{background-color: rgb(255,255,255,100);border-radius: " + QString::number(this->height()/2) + "px;}";
        this->setStyleSheet(str);
        this->setAttribute(Qt::WA_StyledBackground);
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QString str = "QWidget#" + this->objectName() + "{background-color: rgb(255,255,255,80);border-radius: " + QString::number(this->height()/2) + "px;}";
        this->setStyleSheet(str);
        this->setAttribute(Qt::WA_StyledBackground);
        emit clicked();
    }

    return QWidget::event(event);
}

void CommonPushButton::enterEvent(QEvent *event)
{
    QString str = "QWidget#" + this->objectName() + "{background-color: rgb(255,255,255,80);border-radius: " + QString::number(this->height()/2) + "px;}";

    this->setStyleSheet(str);
    this->setAttribute(Qt::WA_StyledBackground);
}

void CommonPushButton::leaveEvent(QEvent *event)
{
    QString str = "QWidget#" + this->objectName() + "{background-color: rgb(255,255,255,40);border-radius: " + QString::number(this->height()/2) + "px;}";
    this->setStyleSheet(str);
    this->setAttribute(Qt::WA_StyledBackground);
}
