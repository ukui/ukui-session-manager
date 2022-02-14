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

#include "mypushbutton.h"
#include <QPixmap>
#include <QVBoxLayout>
#include <QDebug>
#include <QEvent>
MyPushButton::MyPushButton(QString iconPath, QString buttonLable, QString objName, bool isRecBtn, int x, int y,
                           int width, int height, int iconWidth, int labelWidth, QWidget *parent)
    : m_isRecBtn(isRecBtn)
    , QWidget(parent)
{
    if (m_isRecBtn) {
        this->setFixedSize(width, width);
    } else {
        this->setFixedSize(labelWidth, height);
    }
    this->setObjectName(objName);
    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    vBoxLayout->setContentsMargins(0, 0, 0, 10);
    m_iconLabel = new MyIconLabel(labelWidth, iconWidth, iconPath, !isRecBtn);
    m_iconLabel->setObjectName(objName + "_button");
    m_iconLabel->installEventFilter(this);

    m_buttonLabel = new QLabel();
    m_buttonLabelFont = QFont("Noto Sans CJK SC");
    m_buttonLabel->setFont(m_buttonLabelFont);
    m_buttonLabel->setText(buttonLable);
    m_buttonLabel->setStyleSheet("color: white; font: 12pt");
    m_buttonLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    if (m_isRecBtn) {
        this->setStyleSheet("QWidget#" + this->objectName() + "{background-color: transparent;border-radius: 6px;}");
        this->setAttribute(Qt::WA_StyledBackground);
    }
    vBoxLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    vBoxLayout->addWidget(m_buttonLabel, 0, Qt::AlignHCenter);
    this->setLayout(vBoxLayout);
    connect(m_iconLabel, &MyIconLabel::mouseEventSignals, this, &MyPushButton::iconLabelMouseEvent);
}

MyPushButton::~MyPushButton()
{
}

void MyPushButton::changeIconBackColor(bool isChoose, bool isEnterKey)
{
    if (!m_isRecBtn) {
        QString str;
        if (isChoose) {
            if (isEnterKey) {
                str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,80);border: 1px solid #296CD9; border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
            } else {
                str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,80);border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
            }
        } else {
            str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,40);border: 0px; border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
        }
        m_iconLabel->setStyleSheet(str);
        m_iconLabel->setAttribute(Qt::WA_StyledBackground);
    } else {
        QString str;
        if (isChoose) {
            if (isEnterKey) {
                str = "QWidget#" + this->objectName() + "{background-color: rgb(255,255,255,80);border: 0px solid #296CD9; border-radius: 6px;}";
            } else {
                str = "QWidget#" + this->objectName() + "{background-color: rgb(255,255,255,80);border-radius: 6px;}";
            }
        } else {
            str = "QWidget#" + this->objectName() + "{background-color: transparent;border: 0px; border-radius: 6px;}";
        }
        this->setStyleSheet(str);
        this->setAttribute(Qt::WA_StyledBackground);
    }
}

MyIconLabel* MyPushButton::getIconLabel()
{
    return m_iconLabel;
}

void MyPushButton::iconLabelMouseEvent(QEvent *event)
{
    emit mouseRelase(event, m_iconLabel->objectName());
}
