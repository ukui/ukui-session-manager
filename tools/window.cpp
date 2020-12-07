#include "window.h"
#include <QScreen>
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QGSettings/QGSettings>

#define BACKGROUND_SETTINGS "org.mate.background"

window::window()
{
    //QString path = "/tmp/greeter-background.conf";
    QString fullstr;
    //QFile file(path);
    const QByteArray id(BACKGROUND_SETTINGS);
//    if(file.exists()){
//        QSettings qset(path,QSettings::NativeFormat);
//        fullstr = qset.value("Greeter").toString();
//    }
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *gset = new QGSettings(BACKGROUND_SETTINGS,"",this);
        fullstr = gset->get("picture-filename").toString();
    }else
        fullstr = "/usr/share/backgrounds/default.jpg";
    qDebug()<<"picture file is "<<fullstr;
    pix.load(fullstr);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setFixedSize(QApplication::primaryScreen()->virtualSize());
}

void window::paintEvent(QPaintEvent *e)
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
