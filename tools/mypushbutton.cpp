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
MyPushButton::MyPushButton(QString iconPath, QString buttonLable, QString objName, QWidget *parent, bool isRecBtn, int x, int y,
                           int width, int height, int iconWidth, int labelWidth)
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

void MyPushButton::changeIconBackColor(bool isChoose, bool isKeySelect)
{
    if(m_isMouseSelect && m_isKeySelect){
        if((isChoose && isKeySelect) || (isChoose && !isKeySelect)){
            m_isKeySelect = true;
            m_isMouseSelect = true;
        }
        else if(!isChoose && !isKeySelect){
            m_isKeySelect = true;
            m_isMouseSelect = false;
        }
        else if(!isChoose && isKeySelect){
            m_isKeySelect = false;
            m_isMouseSelect = true;
        }
    }
    else if(m_isMouseSelect && !m_isKeySelect){
        if((isChoose && isKeySelect) || (!isChoose && !isKeySelect)){
            m_isKeySelect = isKeySelect;
            m_isMouseSelect = isChoose;
        }
        else if((isChoose && !isKeySelect) || (!isChoose && isKeySelect)){
            m_isKeySelect = false;
            m_isMouseSelect = true;
        }
    }
    else if(!m_isMouseSelect && !m_isKeySelect){
        if(isChoose && isKeySelect){
            m_isKeySelect = true;
            m_isMouseSelect = false;
        }
        else if(isChoose && !isKeySelect){
            m_isKeySelect = false;
            m_isMouseSelect = true;
        }
        else if((!isChoose && !isKeySelect) || (!isChoose && isKeySelect)){
            m_isKeySelect = false;
            m_isMouseSelect = false;
        }
    }
    else if(!m_isMouseSelect && m_isKeySelect){
        if((isChoose && isKeySelect) || (!isChoose && !isKeySelect)){
            m_isKeySelect = true;
            m_isMouseSelect = false;
        }
        else if((isChoose && !isKeySelect) || (!isChoose && isKeySelect)){
            m_isKeySelect = !isKeySelect;
            m_isMouseSelect = isChoose;
        }
    }
//    qDebug() << "isKeySelect..." << objectName() << m_isKeySelect << m_isMouseSelect;

    QString str;
    if((m_isKeySelect && m_isMouseSelect) || (m_isKeySelect && !m_isMouseSelect)){
        str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,80);border: 1px solid #296CD9; border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
    }
    else if(!m_isKeySelect && m_isMouseSelect){
        str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,80);border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
    }
    else if(!m_isKeySelect && !m_isMouseSelect){
        str = "QLabel#" + m_iconLabel->objectName() + "{background-color: rgb(255,255,255,40);border: 0px; border-radius: " + QString::number(m_iconLabel->width()/2) + "px;}";
    }

    m_iconLabel->setStyleSheet(str);
    m_iconLabel->setAttribute(Qt::WA_StyledBackground);

}

MyIconLabel* MyPushButton::getIconLabel()
{
    return m_iconLabel;
}

void MyPushButton::iconLabelMouseEvent(QEvent *event)
{
    emit mouseRelase(event, m_iconLabel->objectName());
}

bool MyPushButton::getIsKeySelect()
{
    return m_isKeySelect;
}

bool MyPushButton::getIsMouseSelect()
{
    return m_isMouseSelect;
}

void MyPushButton::setIsKeySelect(bool isKeySelect)
{
    if(m_isKeySelect != isKeySelect)
        m_isKeySelect = isKeySelect;
}

void MyPushButton::setIsMouseSelect(bool isMouseSelect)
{
    if(m_isMouseSelect != isMouseSelect)
        m_isMouseSelect = isMouseSelect;
}
