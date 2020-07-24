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
#include <QGSettings/QGSettings>
#include <QFileInfo>

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE
#define BLUR_RADIUS 300
#define BACKGROUND_SETTINGS "org.mate.background"

void getUserNum(){
    QStringList args;
    QString cmd = "/bin/sh -c \"cat file | grep string\"";
    //args<<"-l";
//    QString command = "cat /etc/passwd | grep string";
//    qDebug() << "Start ukui module: " << command << "args: " << args;
    QProcess *process;
    process->start(cmd,args);
    process->waitForFinished();
    process->waitForReadyRead();
    QString s = process->readAll();
    //process->close();
    qDebug()<<s;
    //return s.toInt();
}

QPixmap blurPixmap(QPixmap pixmap)
{
    QPainter painter(&pixmap);
    QImage srcImg = pixmap.toImage();
    qt_blurImage(&painter, srcImg, BLUR_RADIUS, false, false);
    painter.end();
    return pixmap;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_power(new UkuiPower(this)),
    timer(new QTimer()),
    xEventMonitor(new XEventMonitor(this))
{
    const QByteArray id(BACKGROUND_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *gs = new QGSettings(BACKGROUND_SETTINGS,"",this);
        QString fullstr = gs->get("picture-filename").toString();
        QFileInfo fileInfo(fullstr);
        if(fileInfo.isFile()){
            pix.load(fullstr);
            pix = blurPixmap(pix);
            gs->deleteLater();
        }else
            pix.load(":/images/background-ukui.png");
    }else
        pix.load(":/images/background-ukui.png");

    ui->setupUi(this);
    ui->switchuser->installEventFilter(this);
    ui->hibernate->installEventFilter(this);
    ui->suspend->installEventFilter(this);
    ui->lockscreen->installEventFilter(this);
    ui->logout->installEventFilter(this);
    ui->reboot->installEventFilter(this);
    ui->shutdown->installEventFilter(this);

    //Make a hash-map to store tableNum-to-lastWidget
    map.insert(0,ui->switchuser);
    map.insert(1,ui->hibernate);
    map.insert(2,ui->suspend);
    map.insert(3,ui->lockscreen);
    map.insert(4,ui->logout);
    map.insert(5,ui->reboot);
    map.insert(6,ui->shutdown);

    if(m_power->canAction(UkuiPower::PowerHibernate)){
        isHibernateHide = false;
    }

    if(true){
        isSwitchuserHide = false;
    }

    //Set the default value
    lastWidget = ui->lockscreen;
    tableNum = 3;
    ui->lockscreen->setStyleSheet("QWidget#lockscreen{background-color: rgb(255,255,255,100);}");

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
    //KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events has been redirected
    connect(xEventMonitor, SIGNAL(keyRelease(const QString &)),this, SLOT(onGlobalKeyPress(const QString &)));

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

    int hideNum = 0;
    if(isHibernateHide){
        hideNum = hideNum + 1;
    }
    if(isSwitchuserHide){
        hideNum = hideNum + 1;
    }

    //Move the widget to the direction where they should be
    int spaceW,spaceH;
    int sum = 0;
    int maxXX = 1000*1000;
    int k = 0;
    if(m_screen.width()>1088){
        spaceW = (m_screen.width() - 158*(7-hideNum)+18) / 2;
        spaceH = (m_screen.height() - 140) / 2 -20;
        for(int i=0;i<=6;i++){
            if(isHibernateHide && i == 1){
                map[i]->move(maxXX , maxXX);
                continue;
            }
            if(isSwitchuserHide && i == 0){
                map[i]->move(maxXX , maxXX);
                continue;
            }
            map[i]->move(xx+spaceW + 158*sum,yy+spaceH);
            sum = sum+1;
        }
        ui->widget->move(xx+(m_screen.width()-130)/2,yy+40);
    }else{
        int spaceWw = (m_screen.width() - 158*(3-hideNum)+18) / 2;
        spaceW = (m_screen.width() - 158*4+18) / 2;
        spaceH = (m_screen.height() - 80) / 2;
        for(int i=0;i<=6;i++){
            if(i < 3){
                if(isHibernateHide && i == 1){
                    map[i]->move(maxXX , maxXX);
                    continue;
                }
                if(isSwitchuserHide && i == 0){
                    map[i]->move(maxXX , maxXX);
                    continue;
                }
                map[i]->move(xx+spaceWw + 158*sum,yy+spaceH-120);
                sum = sum+1;
            }else{
                map[i]->move(xx+spaceW + 158*k,yy+spaceH+38);
                k++;
            }
        }
        ui->widget->move(xx+(m_screen.width()-130)/2,yy+40);
    }
}

//Paint the background picture
void MainWindow::paintEvent(QPaintEvent *e)
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

//lock screen
void doLockscreen(){
    QString arg = "-l";
    QStringList args;
    args.append(arg);
    QString command = "ukui-screensaver-command";
    qDebug() << "Start ukui module: " << command << "args: " << args;
    QProcess::execute(command, args);
}

//handle mouse-clicked event
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == "switchuser") {
        changePoint(ui->switchuser,event,0);
        if (event->type() == QEvent::MouseButtonRelease) {
            doevent("switchuser",0);
        }
    }else if (obj->objectName() == "hibernate") {
        changePoint(ui->hibernate,event,1);
        if (event->type() == QEvent::MouseButtonRelease) {
            doevent("hibernate",1);
        }
    }else if (obj->objectName() == "suspend") {
        changePoint(ui->suspend,event,2);
        if (event->type() == QEvent::MouseButtonRelease) {
            doevent("suspend",2);
        }
    } else if (obj->objectName() == "lockscreen") {
        changePoint(ui->lockscreen,event,3);
        if (event->type() == QEvent::MouseButtonRelease) {
            doLockscreen();
        }
    }else if (obj->objectName() == "logout") {
        changePoint(ui->logout,event,4);
        if (event->type() == QEvent::MouseButtonRelease) {
            doevent("logout",4);
        }
    }else if (obj->objectName() == "reboot") {
        changePoint(ui->reboot,event,5);
        if (event->type() == QEvent::MouseButtonRelease) {
            doevent("reboot",5);
        }
    } else if(obj->objectName() == "shutdown") {
        changePoint(ui->shutdown,event,6);
        if (event->type() == QEvent::MouseButtonRelease) {
            doevent("shutdown",6);
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MainWindow::changePoint(QWidget *widget, QEvent *event, int i){
    if(event->type() == QEvent::Enter){
        tableNum = i;
        flag = true;
        refreshBlur(lastWidget,widget);
    }
    if(event->type() == QEvent::Leave){
        flag = false;
        lastWidget = widget;
    }
}

void MainWindow::doevent(QString test, int i){
    try {
        //close();
        //m_power->doAction(UkuiPower::Action(i));
        defaultnum = i;
        qDebug()<<"Start do action"<<test<<defaultnum;
        this->hide();
        emit signalTostart();
        //timer->start(1000);
    } catch (QException &e) {
        qWarning() << e.what();
    }
}

//handle the blank-area mousePressEvent
void MainWindow::mousePressEvent(QMouseEvent *event){
    if (!ui->suspend->geometry().contains(event->pos()) &&
            !ui->hibernate->geometry().contains(event->pos()) &&
            !ui->lockscreen->geometry().contains(event->pos()) &&
            !ui->switchuser->geometry().contains(event->pos()) &&
            !ui->logout->geometry().contains(event->pos()) &&
            !ui->reboot->geometry().contains(event->pos()) &&
            !ui->shutdown->geometry().contains(event->pos())) {
        close();
        exit(0);
    }
}

//handle "Esc","Left","Right","Enter" keyPress event
void MainWindow::onGlobalKeyPress(const QString &key)
{
    if (key == "Escape") {
        close();
        exit(0);
    }
    if (key == "Left"){
        if (flag == false){
            if(tableNum == 0){
                tableNum = 6;
            }else{
                if(isHibernateHide && tableNum == 2){
                    if(isSwitchuserHide){
                        tableNum = 6;
                    }else
                        tableNum = 0;
                }else{
                    if(isSwitchuserHide && tableNum == 1){
                        tableNum = 6;
                    }else
                        tableNum = tableNum-1;
                }
            }
        }
        refreshBlur(lastWidget,map[tableNum]);
        lastWidget = map[tableNum];
    }
    if (key == "Right"){
        if(flag == false){
            if(!isSwitchuserHide && tableNum == 6){
                tableNum = 0;
            }else if(isSwitchuserHide && tableNum == 6){
                if(isHibernateHide)
                    tableNum = 2;
                else
                    tableNum = 1;
            }else{
                if(isHibernateHide && tableNum == 0)
                    tableNum = 2;
                else
                    tableNum = tableNum+1;
            }
        }
        refreshBlur(lastWidget,map[tableNum]);
        lastWidget = map[tableNum];
    }
    if (key == "Return"){//space,KP_Enter
        qDebug()<<map[tableNum]->objectName()<<"";
        switch (tableNum) {
        case 0:
            doevent("switchuser",0);
            break;
        case 1:
            doevent("hibernate",1);
            break;
        case 2:
            doevent("suspend",2);
            break;
        case 3:
            doLockscreen();
            break;
        case 4:
            doevent("logout",4);
            break;
        case 5:
            doevent("reboot",5);
            break;
        case 6:
            doevent("shutdown",6);
            break;
        }
        this->hide();
    }
}

void MainWindow::refreshBlur(QWidget *last, QWidget *now){
    QString pastName = last->objectName();
    QString name = now->objectName();
    QString strlast = "QWidget#" + pastName + "{background-color: rgb(0,0,0,0)}";
    QString str = "QWidget#" + name + "{background-color: rgb(255,255,255,100);border-radius: 6px;}";
    last->setStyleSheet(strlast);
    now->setStyleSheet(str);
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
