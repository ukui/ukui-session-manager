#include "pushbutton.h"
#include <QPushButton>
#include <QMouseEvent>
PushButton::PushButton(QWidget *parent)
    :QPushButton(parent)
{
}

PushButton::~PushButton()
{

}

void PushButton::keyPressEvent(QKeyEvent *keyEvent)
{
    return;
//    if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return ||  keyEvent->key() == Qt::Key_Escape ||
//            keyEvent->key() == Qt::Key_Left ||keyEvent->key() == Qt::Key_Right)
//    return;
//    return PushButton::keyPressEvent(keyEvent);
}

//void PushButton::enterEvent(QEvent *event)
//{

//}

//void PushButton::leaveEvent(QEvent *event)
//{

//}
