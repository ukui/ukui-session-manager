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
