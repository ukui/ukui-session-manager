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
