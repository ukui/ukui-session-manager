#include "mainwindow.h"
#include <QScreen>
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QGSettings/QGSettings>

#define BACKGROUND_SETTINGS "org.mate.background"

mainwindow::mainwindow()
{
    const QByteArray id(BACKGROUND_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *gset = new QGSettings(BACKGROUND_SETTINGS,"",this);
        QString fullstr = gset->get("picture-filename").toString();
        pix.load(fullstr);
    }else{
        pix.load(":/images/background-ukui.png");
    }
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setFixedSize(QApplication::primaryScreen()->virtualSize());
}

void mainwindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    for(QScreen *screen : QApplication::screens()) {
        //draw picture to every screen
        QRect rect = screen->geometry();
        painter.drawPixmap(rect,pix);
        painter.drawRect(rect);
    }
    QWidget::paintEvent(e);
}
