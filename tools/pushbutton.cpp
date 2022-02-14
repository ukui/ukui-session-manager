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

#include "pushbutton.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>
PushButton::PushButton(QWidget *parent)
    :QPushButton(parent)
{
}

PushButton::~PushButton()
{
}

void PushButton::keyPressEvent(QKeyEvent *keyEvent)
{
    qDebug() << "keyPressEvent..." << keyEvent->key();
    return;
}

bool PushButton::event(QEvent *e)
{
    if (e->type() == QEvent::FocusIn) {
        e->accept();
        //qDebug() << "keyrelease" << static_cast<QKeyEvent*>(e)->key();
        return true;
    }

    if (e->type() == QEvent::KeyRelease || e->type() == QEvent::KeyPress) {
        e->accept();
        return true;
    }

    return QPushButton::event(e);
}
