/*Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "powerprovider.h"
#include <QPainter>
#include <QPixmap>
#include <QException>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QDateTime>
#include <QScreen>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QX11Info>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <sys/types.h>
#include <X11/keysym.h>
#include "grab-x11.h"
#include "xeventmonitor.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_power(new UkuiPower(this)),
    timer(new QTimer()),
    xEventMonitor(new XEventMonitor(this))
{
    ui->setupUi(this);
    ui->suspend->installEventFilter(this);
    ui->switchuser->installEventFilter(this);
    ui->logout->installEventFilter(this);
    ui->reboot->installEventFilter(this);
    ui->shutdown->installEventFilter(this);

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy-MM-dd ddd");
    QString current_time =current_date_time.toString("hh:mm");
    ui->time_lable->setText(current_time);
    ui->date_label->setText(current_date);

    //根据屏幕分辨率与鼠标位置重设界面
    m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    setFixedSize(QApplication::primaryScreen()->virtualSize());
    move(0,0);//设置初始位置的值
    ResizeEvent();

    //设置窗体无边框，不可拖动拖拽拉伸;为顶层窗口，无法被切屏;不使用窗口管理器
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    //setAttribute(Qt::WA_TranslucentBackground, true);//设定该窗口透明显示

    /*捕获键盘，如果捕获失败，那么模拟一次esc按键来退出菜单，如果仍捕获失败，则放弃捕获*/
    if (establishGrab()) {
        qDebug()<<"establishGrab : true";
    } else {
        qDebug()<<"establishGrab : false";
        XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_Escape), True, 1);
        XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_Escape), False, 1);
        XFlush(QX11Info::display());
        sleep(1);
        if (!establishGrab()) {
            qDebug()<<"establishGrab : false again!";
            //exit(1);
        }
    }
    connect(xEventMonitor, SIGNAL(keyRelease(const QString &)),this, SLOT(onGlobalKeyRelease(const QString &)));
    xEventMonitor->start();

    this->show();

    //qApp->installNativeEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete m_power;
    delete xEventMonitor;
    delete ui;
}

void MainWindow::ResizeEvent(){
    int xx = m_screen.x();
    int yy = m_screen.y();//取得当前鼠标所在屏幕的最左，上坐标
    if (m_screen.width()<928) {
        ui->suspend->move(0,(m_screen.height()-168)/2);
        ui->switchuser->move(168,(m_screen.height()-168)/2);
        ui->logout->move(168*2,(m_screen.height()-168)/2);
        ui->reboot->move(168*3,(m_screen.height()-168)/2);
        ui->shutdown->move(168*4,(m_screen.height()-168)/2);
        ui->widget->move((m_screen.width()-130)/2,m_screen.height()/4);
    }
    int spaceW = (m_screen.width() - 928) / 2;
    int spaceH = (m_screen.height() - 168) / 2 -20;
    ui->suspend->move(xx + spaceW + 0,yy + spaceH);
    ui->switchuser->move(xx+spaceW + 190,yy+spaceH);
    ui->logout->move(xx+spaceW + 190*2,yy+spaceH);
    ui->reboot->move(xx+spaceW + 190*3,yy+spaceH);
    ui->shutdown->move(xx+spaceW + 190*4,yy+spaceH);
    ui->widget->move(xx+(m_screen.width()-130)/2,yy+40);
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    //painter.setBrush(QColor("#0a4989"));
    QPixmap pix;
    pix.load(":/images/background-ukui.png");
    for(QScreen *screen : QApplication::screens()) {
        //draw picture to every screen
        QRect rect = screen->geometry();
        painter.drawPixmap(rect,pix);
        painter.drawRect(rect);
    }
    QWidget::paintEvent(e);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == "suspend") {
        doevent(event,"suspend",5);
//    } else if (obj == m_hibernate) {
//        doevent(event,"hibernate",2);
    } else if (obj->objectName() == "reboot") {
        doevent(event,"reboot",3);
    } else if(obj->objectName() == "shutdown") {
        doevent(event,"shutdown",4);
    } else if (obj->objectName() == "logout") {
        doevent(event,"logout",0);
    } else if (obj->objectName() == "switchuser") {
        doevent(event,"switchuser",1);
    }
    return QWidget::eventFilter(obj, event);
}

void MainWindow::doevent(QEvent *event, QString test, int i){
    if (event->type() == QEvent::MouseButtonRelease) {
        try {
//            close();
//            m_power->doAction(UkuiPower::Action(i));
            defaultnum = i;
            qDebug()<<"Start do action"<<test<<defaultnum;
            this->hide();
            emit signalTostart();
            //timer->start(1000);
        } catch (QException &e) {
            qWarning() << e.what();
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    if (!ui->suspend->geometry().contains(event->pos()) &&
            !ui->switchuser->geometry().contains(event->pos()) &&
            !ui->logout->geometry().contains(event->pos()) &&
            !ui->reboot->geometry().contains(event->pos()) &&
            !ui->shutdown->geometry().contains(event->pos())) {
        close();
        exit(0);
    }
}

void MainWindow::onGlobalKeyRelease(const QString &key)
{
    if (key == "Escape") {
        close();
        exit(0);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"MainWindow:: CloseEvent";
    if (closeGrab()) {
        qDebug()<<"success to close Grab";
    } else {
        qDebug()<<"failure to close Grab";
    }
    return QWidget::closeEvent(event);
}

/*
bool MainWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
        return false;
    }
    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
    const uint8_t responseType = event->response_type & ~0x80;
    if (responseType == XCB_CONFIGURE_NOTIFY) {
        xcb_configure_notify_event_t *xc = reinterpret_cast<xcb_configure_notify_event_t*>(event);
        if (xc->event == QX11Info::appRootWindow())
        {
            XRaiseWindow(QX11Info::display(), this->winId());
            XFlush(QX11Info::display());
            //raise();
        }
        return false;
    }
    else if(responseType == XCB_PROPERTY_NOTIFY)
    {
        //raise();
        XRaiseWindow(QX11Info::display(), this->winId());
        XFlush(QX11Info::display());
    }
    return false;
}
*/
