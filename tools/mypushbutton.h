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
    MyPushButton(QString iconPath, QString buttonLable, QString objName, bool isRecBtn=true, int x = 0, int y = 0,
                 int width = 140, int height = 180, int iconWidth = 48,
                 int labelWidth = 128, QWidget *parent = nullptr);
    ~MyPushButton();

    void changeIconBackColor(bool isChoose = true, bool isEnterKey = false);

    MyIconLabel* getIconLabel();
protected:

private:
    QLabel *m_buttonLabel = nullptr;
    QFont m_buttonLabelFont;
    QPixmap m_pixMap;
    MyIconLabel *m_iconLabel = nullptr;
    bool m_isRecBtn=true;//true 背景为整体  false表示背景只在ICON区域
protected:

signals:
    void mouseRelase(QEvent *event, QString iconName);

public slots:
    void iconLabelMouseEvent(QEvent *event);

};
#endif // MYPUSHBUTTON_H
