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
#include "lockchecker.h"
#include "./ui_mainwindow.h"
#include "powerprovider.h"
#include "../ukui-session/xdgdesktopfile.h"

#include <QListView> //XTest.h/Xlib.h/XInput.h/X.h中定义了一个None,QStyleOption中也定义了None,会造成冲突，把QListView的头文件放到前面
#include <QDir>
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
#include <QFileInfo>
#include <QDBusInterface>
#include <QTextStream>
#include <QTextBrowser>
#include <QStringListModel>
#include <QMessageBox>
#include <QPushButton>
#include <QDBusReply>
#include "loginedusers.h"
#include <QDBusMetaType>
#include <QStandardItemModel>
#include <QScrollBar>

#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <glib.h>
#include "commonpushbutton.h"

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly,
                         int transposed = 0);
QT_END_NAMESPACE
#define BLUR_RADIUS         300
#define BACKGROUND_SETTINGS "org.mate.background"

QPixmap blurPixmap(QPixmap pixmap)
{
    QPainter painter(&pixmap);
    QImage   srcImg = pixmap.toImage();
    qt_blurImage(&painter, srcImg, BLUR_RADIUS, false, false);
    painter.end();
    return pixmap;
}

QString getUserName(QFile *a)
{
    QString user = getenv("USER");
    if (a->exists()) {
        a->open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream fileStream(a);
        int         k = 0;
        while (!fileStream.atEnd()) {
            QString line = fileStream.readLine();
            if (k == 0) {
                QString a = line;
                qDebug() << "uid=" << a;
                struct passwd *user1;
                user1 = getpwuid(a.toInt());
                qDebug() << "name=" << user1->pw_name << ",uid=" << user1->pw_uid;
                if (user1->pw_name == NULL) {
                    return user;
                }
                user = user1->pw_name;
            }
            k++;
        }
    }
    return user;
}

MainWindow::MainWindow(bool a, bool b, QWidget *parent) : QMainWindow(parent)
                                                        , ui(new Ui::MainWindow)
                                                        , m_power(new UkuiPower(this))
                                                        , timer(new QTimer())
                                                        , xEventMonitor(new XEventMonitor(this))
{
    const QByteArray bid(BACKGROUND_SETTINGS);
    if (QGSettings::isSchemaInstalled(bid)) {
        QGSettings    *gset = new QGSettings(BACKGROUND_SETTINGS, "", this);
        QString     fullstr = gset->get("picture-filename").toString();
        qDebug() << "picture path = " << fullstr;
        QFileInfo fileInfo(fullstr);
        if (fileInfo.isFile()) {
            pix.load(fullstr);
            //增加对pix的判断，有些图片格式qt不支持，无法读取，导致pix为null,引起程序崩溃 #bug75856
            if (pix.isNull()) {
                pix.load(":/images/background-ukui.png");
                pix = blurPixmap(pix);
            } else {
                pix = blurPixmap(pix);
            }

        } else {
            QString   imagefile = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
            QFileInfo fileimage(imagefile);
            if (fileimage.isFile() && fileimage.exists()) {
                pix.load(imagefile);
                pix = blurPixmap(pix);
            }
        }
        gset->deleteLater();
    } else {
        pix.load("/usr/share/ukui/ukui-session-manager/images/background-ukui.png");
        pix = blurPixmap(pix);
    }

    this->setObjectName("widget");
    m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());

    m_toolWidget = new QWidget(this);
    m_toolWidget->setGeometry(m_screen);

    qDebug() << "m_toolWidget width:" << m_toolWidget->width()<<m_toolWidget->height();
    m_vBoxLayout = new QVBoxLayout();
    m_buttonHLayout = new QGridLayout();
    m_dateTimeLayout = new QVBoxLayout();

    m_timeLabel = new QLabel();
    m_dateLabel = new QLabel();
    m_judgeWidgetVLayout = new QVBoxLayout();
    m_judgeBtnHLayout = new QHBoxLayout();
    m_judgeLabel = new QLabel();
    m_messageVLayout = new QVBoxLayout();
    m_messageLabel1 = new QLabel();
    m_messageLabel2 = new QLabel();

    m_scrollArea = new QScrollArea;
    m_scrollArea->setObjectName("scrollArea");

    m_btnWidget = new QWidget();
    m_btnWidget->setObjectName("btnWidget");

    //m_showWarningArea 作为该界面所有组件的父指针，方便排版
    m_showWarningArea = new QWidget(this);
    m_showWarningArea->setObjectName(QString::fromUtf8("area"));
    //initialSystemMonitor();

    initialBtn();

    initialJudgeWidget();

    //获取mode为block的sleep和shutdown inhibitors
    if (LockChecker::isSleepBlocked()) {
        LockChecker::getSleepInhibitors(sleepInhibitors, sleepInhibitorsReason);
        inhibitSleep = true;
    }

    if (LockChecker::isShutdownBlocked()) {
        LockChecker::getShutdownInhibitors(shutdownInhibitors, shutdownInhibitorsReason);
        inhibitShutdown = true;
    }

    user     = getenv("USER");
    lockfile = a;
    lockuser = b;

    initialMessageWidget();

    int hideNum = 7;
    //Make a hash-map to store tableNum-to-lastWidget
    if (m_power->canAction(UkuiPower::PowerHibernate)) {
        isHibernateHide = false;
        hideNum--;
    }

    if (m_power->canAction(UkuiPower::PowerSuspend)) {
        isSuspendHide = false;
        hideNum--;
    }

    if (m_power->canAction(UkuiPower::PowerLogout)) {
        isLogoutHide = false;
        hideNum--;
    }

    if (m_power->canAction(UkuiPower::PowerReboot)) {
        isRebootHide = false;
        hideNum--;
    }

    if (m_power->canAction(UkuiPower::PowerShutdown)) {
        isPowerOffHide = false;
        hideNum--;
    }

    if (LockChecker::getCachedUsers() > 1 && m_power->canAction(UkuiPower::PowerSwitchUser)) {
        isSwitchuserHide = false;
        hideNum--;
    }

    initialBtnCfg();
    map.insert(0, m_switchUserBtn);
    map.insert(1, m_hibernateBtn);
    map.insert(2, m_suspendBtn);
    map.insert(3, m_lockScreenBtn);
    map.insert(4, m_logoutBtn);
    map.insert(5, m_rebootBtn);
    map.insert(6, m_shutDownBtn);

    gs = new QGSettings("org.ukui.session", "/org/ukui/desktop/session/");

    tableNum = -1;
    changeBtnState("empty");

    initialDateTimeWidget();

    ResizeEvent();

    //使按钮控件上下比例均衡
    QVBoxLayout *spaceLayout = new QVBoxLayout();
    QLabel spaceLabel1;
    QLabel spaceLabel2;
    spaceLabel1.setFont(QFont("Noto Sans CJK SC", 28, 50));
    spaceLabel2.setFont(QFont("Noto Sans CJK SC", 12, 50));
    spaceLabel1.setStyleSheet("color: white; font: 28pt");
    spaceLabel2.setStyleSheet("color: white; font: 12pt");
    spaceLabel1.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    spaceLabel2.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    spaceLabel1.setObjectName("date_label");
    spaceLabel2.setObjectName("time_lable");
    spaceLayout->addStretch();
    spaceLayout->addWidget(&spaceLabel1);
    spaceLayout->addSpacing(10);
    spaceLayout->addWidget(&spaceLabel2);
    setLayoutWidgetVisible(spaceLayout, false);

    m_vBoxLayout->addStretch(20);
    m_vBoxLayout->addLayout(m_dateTimeLayout, 80);
    m_vBoxLayout->addLayout(m_judgeWidgetVLayout, 120);
    m_vBoxLayout->addWidget(m_scrollArea, 640);
    m_vBoxLayout->addLayout(m_messageVLayout, 120);
    m_vBoxLayout->addLayout(spaceLayout, 80);
    m_vBoxLayout->addStretch(20);
    m_vBoxLayout->setSpacing(0);

    m_vBoxLayout->setContentsMargins((m_screen.width() - m_scrollArea->width() - 20)/2,0,(m_screen.width() - m_scrollArea->width() - 20)/2,0);

    qDebug() << "width..........." << m_judgeLabel->width() << m_scrollArea->width() << m_messageLabel1->width() << m_messageLabel2->width();
    //根据屏幕分辨率与鼠标位置重设界面
    //m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    //setFixedSize(QApplication::primaryScreen()->virtualSize());
    setGeometry(0, 0, QApplication::primaryScreen()->virtualSize().width(), QApplication::primaryScreen()->virtualSize().height());
    move(0, 0);//设置初始位置的值

    //设置窗体无边框，不可拖动拖拽拉伸;为顶层窗口，无法被切屏;不使用窗口管理器
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

    // setAttribute(Qt::WA_TranslucentBackground, true);//设定该窗口透明显示
    m_toolWidget->setLayout(m_vBoxLayout);
    //setCentralWidget(m_toolWidget);

    qDebug() << "m_toolWidget...." << m_toolWidget->geometry();
    qDebug() << "pos..." << QCursor::pos() << this->geometry();
    qDebug() << "m_screen..." << m_screen;

    rowMap.insert(0, m_switchRow);
    rowMap.insert(1, m_hibernateRow);
    rowMap.insert(2, m_suspendRow);
    rowMap.insert(3, m_lockScreenRow);
    rowMap.insert(4, m_logoutRow);
    rowMap.insert(5, m_rebootRow);
    rowMap.insert(6, m_shutDownRow);

    /*捕获键盘，如果捕获失败，那么模拟一次esc按键来退出菜单，如果仍捕获失败，则放弃捕获*/
    if (establishGrab()) {
        qDebug() << "establishGrab : true";
    } else {
        qDebug() << "establishGrab : false";
        XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Escape),
                          True, 1);
        XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Escape),
                          False, 1);
        XFlush(QX11Info::display());
        sleep(1);
        if (!establishGrab()) {
            qDebug() << "establishGrab : false again!";
            // exit(1);
        }
    }
    // KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events has been redirected
    connect(xEventMonitor, SIGNAL(keyPress(const QString &)), this,
            SLOT(onGlobalKeyPress(const QString &)));
    connect(xEventMonitor, SIGNAL(keyRelease(const QString &)), this,
            SLOT(onGlobalkeyRelease(const QString &)));

    xEventMonitor->start();

    //设置字体跟随主题
    const QByteArray id("org.ukui.style");
    QFont font = QFont("Noto Sans CJK SC");
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *fontSetting = new QGSettings(id, QByteArray(), this);
        font                    = QFont(fontSetting->get("systemFont").toString());
    }
    for (auto widget : qApp->allWidgets()) {
        font.setWordSpacing(2);
        widget->setFont(font);
    }
    //    QFont fontt = font;
    //    qDebug()<<font.wordSpacing()<<"----"<<font.letterSpacing();
    //    fontt.setWordSpacing(13.0);
    //    fontt.setLetterSpacing(QFont::AbsoluteSpacing,100);
    //    ui->time_lable->setFont(fontt);
    //    qDebug()<<fontt.wordSpacing()<<"----"<<font.letterSpacing();

    //    this->show();

    //screencount changed
    QDesktopWidget *desktop = QApplication::desktop();
    connect(desktop, &QDesktopWidget::screenCountChanged, this, &MainWindow::screenCountChanged);
    connect(desktop, &QDesktopWidget::resized, this, &MainWindow::screenCountChanged);
    connect(desktop, &QDesktopWidget::workAreaResized, this, &MainWindow::screenCountChanged);
    connect(desktop, &QDesktopWidget::primaryScreenChanged, this, &MainWindow::screenCountChanged);
    qDebug() << "m_btnWidget FixedHeight000:" << m_btnWidget->width() << m_scrollArea->width();

    qApp->installNativeEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete m_power;
    delete xEventMonitor;
    delete ui;
}

/*
void MainWindow::initialSystemMonitor()
{
    m_systemMonitorHLayout = new QHBoxLayout();
    m_systemMonitorBtn = new QWidget(this);
    m_systemMonitorIconLabel = new QLabel();
    m_systemMonitorLabel = new QLabel(m_systemMonitorBtn);
    m_systemMonitorBtn->setObjectName("systemMonitor");
    QFont font;
    font.setFamily("Noto Sans CJK SC");
    font.setPointSize(12);
    QFontMetrics fm(font);

    m_systemMonitorLabel->setText(QApplication::tr("system-monitor"));
    m_systemMonitorLabel->setFont(font);
    m_systemMonitorLabel->setStyleSheet("color: white; font: 12pt");

    int btnWidth = fm.boundingRect(m_systemMonitorLabel->text()).width() + 40;

    qDebug() << "btnWidth:" << btnWidth << fontMetrics().width(m_systemMonitorLabel->width()) ;
    m_systemMonitorBtn->setFixedSize(btnWidth,48);
    QString str = "QWidget#systemMonitor{background-color: transparent;border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";

    //QString str = "QWidget#systemMonitor{background-color: rgb(255,255,255,40);border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
    m_systemMonitorBtn->setStyleSheet(str);
    m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);
    QRegion region0(m_systemMonitorBtn->x() + m_systemMonitorBtn->height()/2 + 1, m_systemMonitorBtn->y() - 1, m_systemMonitorBtn->width()-m_systemMonitorBtn->height(), m_systemMonitorBtn->height() + 2);
    QRegion region1(m_systemMonitorBtn->x() - 1, m_systemMonitorBtn->y() - 1, m_systemMonitorBtn->height() + 2, m_systemMonitorBtn->height() + 2, QRegion::Ellipse);
    QRegion region2(m_systemMonitorBtn->x() + m_systemMonitorBtn->width() - m_systemMonitorBtn->height(), m_systemMonitorBtn->y() - 1, m_systemMonitorBtn->height() + 2, m_systemMonitorBtn->height() + 2, QRegion::Ellipse);
    QRegion region = region0 + region1 + region2;
    m_systemMonitorBtn->setMask(region);
    m_systemMonitorLabel->setAlignment(Qt::AlignCenter);
    m_systemMonitorHLayout->setAlignment(Qt::AlignCenter);

    m_systemMonitorHLayout->addWidget(m_systemMonitorLabel,fm.boundingRect(m_systemMonitorLabel->text()).width());

    m_systemMonitorBtn->setLayout(m_systemMonitorHLayout);
    m_systemMonitorBtn->installEventFilter(this);
    qDebug() << "m_systemMonitorIcon:" << m_systemMonitorIcon.width() << m_systemMonitorIcon.height() << m_systemMonitorIconLabel->width() << m_systemMonitorIconLabel->height();
    qDebug() << "m_systemMonitorBtn:" << m_systemMonitorBtn->height();

}
*/

void MainWindow::initialBtn()
{
    m_switchUserBtn = new MyPushButton(m_btnImagesPath+"/switchuser.svg", QApplication::tr("Switch User"), "switchuser", m_scrollArea);
    m_hibernateBtn = new MyPushButton(m_btnImagesPath+"/hibernate.svg", QApplication::tr("Hibernate"), "hibernate", m_scrollArea);
    m_suspendBtn = new MyPushButton(m_btnImagesPath+"/suspend.svg", QApplication::tr("Suspend"), "suspend", m_scrollArea);
    m_logoutBtn = new MyPushButton(m_btnImagesPath+"/logout.svg", QApplication::tr("Logout"), "logout", m_scrollArea);
    m_rebootBtn = new MyPushButton(m_btnImagesPath+"/reboot.svg", QApplication::tr("Reboot"), "reboot", m_scrollArea);
    m_shutDownBtn = new MyPushButton(m_btnImagesPath+"/shutdown.svg", QApplication::tr("Shut Down"), "shutdown", m_scrollArea);
    m_lockScreenBtn = new MyPushButton(m_btnImagesPath+"/lockscreen.svg", QApplication::tr("Lock Screen"), "lockscreen", m_scrollArea);

    //ui->setupUi(this);
    m_switchUserBtn->installEventFilter(this);
    m_hibernateBtn->installEventFilter(this);
    m_suspendBtn->installEventFilter(this);
    m_lockScreenBtn->installEventFilter(this);
    m_logoutBtn->installEventFilter(this);
    m_rebootBtn->installEventFilter(this);
    m_shutDownBtn->installEventFilter(this);
    connect(m_switchUserBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_hibernateBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_suspendBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_lockScreenBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_logoutBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_rebootBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
    connect(m_shutDownBtn, &MyPushButton::mouseRelase, this, &MainWindow::mouseReleaseSlots);
}

void MainWindow::initialJudgeWidget()
{
//    QStringList userlist = getLoginedUsers();
    QStringList userlist = LockChecker::getLoginedUsers();
    if (userlist.count() > 1) {
        close_system_needed_to_confirm = true;
    }
    QString tips = QApplication::tr("Multiple users are logged in at the same time.Are you sure "
                                    "you want to close this system?");
    m_judgeLabel->setText(tips);
    m_judgeLabel->setStyleSheet("color:white;font:12pt;");
    m_judgeLabel->setObjectName("label");
    //m_judgeLabel->setGeometry(0,0,m_screen.width(),50);
    //m_judgeLabel->setFixedHeight(60);

    qDebug() << "m_judgeLabel width:" << m_judgeLabel->width() << m_judgeLabel->height();
    m_judgeLabel->setAlignment(Qt::AlignCenter);
    m_judgeLabel->setWordWrap(true);

    m_cancelBtn = new QPushButton(QApplication::tr("cancel"));
    m_confirmBtn = new QPushButton(QApplication::tr("confirm"));
    m_cancelBtn->setObjectName("cancelButton");
    m_confirmBtn->setObjectName("confirmButton");
    m_cancelBtn->setGeometry(QRect(0, 0, 96, 36));
    m_confirmBtn->setGeometry(QRect(0, 0, 96, 36));

    m_judgeBtnHLayout->setAlignment(Qt::AlignHCenter| Qt::AlignBottom);
    m_judgeBtnHLayout->setSpacing(0);
    m_judgeBtnHLayout->addWidget(m_cancelBtn);
    m_judgeBtnHLayout->addSpacing(54);
    m_judgeBtnHLayout->addWidget(m_confirmBtn);

    m_judgeWidgetVLayout->addStretch();
    m_judgeWidgetVLayout->addWidget(m_judgeLabel);
    m_judgeWidgetVLayout->addSpacing(10);
    m_judgeWidgetVLayout->addLayout(m_judgeBtnHLayout);
    m_judgeWidgetVLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::exitt);
    connect(m_confirmBtn, &QPushButton::clicked, [&]() { emit confirmButtonclicked(); });
    //ui->judgeWidget->hide();
    setLayoutWidgetVisible(m_judgeWidgetVLayout, false);
    setLayoutWidgetVisible(m_judgeBtnHLayout, false);
}

void MainWindow::initialMessageWidget()
{
    int margins = 0;
    if (m_screen.width() > 1088) {
        margins = (m_screen.width() - 60 * 6 - 140 * 7) / 2;
    } else {
        margins = (m_screen.width() - 60 * 2 - 140 * 3) / 2;
    }

    m_messageLabel1->setGeometry(0, 0, m_screen.width() - 160, 40);
    m_messageLabel2->setGeometry(0, 0, m_screen.width() - 160, 40);
    m_messageLabel1->setStyleSheet("color:white;font:12pt;");
    m_messageLabel2->setStyleSheet("color:white;font:12pt;");
    m_messageLabel1->setObjectName("messagelabel1");
    m_messageLabel2->setObjectName("messagelabel2");
//    m_messageLabel1->setFixedWidth(m_screen.width() - 20);
//    m_messageLabel2->setFixedWidth(m_screen.width() - 20);
    m_messageLabel1->setWordWrap(true);
    m_messageLabel2->setWordWrap(true);
    m_messageLabel2->setAlignment(Qt::AlignHCenter);
    m_messageLabel1->setAlignment(Qt::AlignHCenter);
    m_messageLabel1->setMargin(0);
    m_messageLabel2->setMargin(0);
    m_messageLabel1->setWordWrap(true);
    m_messageLabel2->setWordWrap(true);

    m_messageVLayout->addStretch();
    m_messageVLayout->addWidget(m_messageLabel1);
    m_messageVLayout->addSpacing(10);
    m_messageVLayout->addWidget(m_messageLabel2);
    //m_messageVLayout->setAlignment(Qt::AlignHCenter);
    //m_messageVLayout->setContentsMargins(0,0,0,0);

    if (lockfile) {
        QFile   file_update("/tmp/lock/kylin-update.lock");
        QFile   file_backup("/tmp/lock/kylin-backup.lock");
        QString lable1_text;
        QString lable2_text;

        QString a1 = QApplication::tr(
            "(user),ukui-control-center is performing a system update or package installation.");
        QString a2 = QApplication::tr(
            "(user),yhkylin-backup-tools is performing a system backup or restore.");
        QString b1 = QApplication::tr("For system security,Reboot、Shutdown、Logout and Hibernate "
                                      "are temporarily unavailable.");
        QString b2 = QApplication::tr(
            "For system security,Reboot、Shutdown and Hibernate are temporarily unavailable.");
        if (file_update.exists()) {
            user        = getUserName(&file_update);
            lable1_text = a1;
        }
        if (file_backup.exists()) {
            user        = getUserName(&file_backup);
            lable1_text = a2;
        }
        if (lockuser) {
            lable2_text = b1;
            m_logoutBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        } else {
            lable2_text = b2;
        }

        m_messageLabel1->setText(user + lable1_text);
        m_messageLabel2->setText(lable2_text);
        m_shutDownBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        m_rebootBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        m_hibernateBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        //        m_rebootBtn->setStyleSheet("QWidget#reboot{background-color: rgb(255,255,0,50);}");
    } else {
        //ui->message->hide();
        setLayoutWidgetVisible(m_messageVLayout, false);
    }
}

void MainWindow::initialDateTimeWidget()
{
    QDateTime        current_date_time = QDateTime::currentDateTime();
    const QByteArray id_control("org.ukui.control-center.panel.plugins");
    QString          current_date;
    QString          current_time;
    if (QGSettings::isSchemaInstalled(id_control)) {
        QGSettings *controlSetting = new QGSettings(id_control, QByteArray(), this);
        QString     formate_a      = controlSetting->get("date").toString();
        QString     formate_b      = controlSetting->get("hoursystem").toString();
        if (formate_a == "en") {
            current_date = current_date_time.toString("yyyy-MM-dd ddd");
        } else if (formate_a == "cn") {
            current_date = current_date_time.toString("yyyy/MM/dd ddd");
        } else {
            current_date = current_date_time.toString("yyyy-MM-dd ddd");
        }

        if (formate_b == "12") {
            current_time = current_date_time.toString("A hh:mm");
        } else if (formate_b == "24") {
            current_time = current_date_time.toString("hh:mm");
        } else {
            current_time = current_date_time.toString("hh:mm");
        }
    } else {
        current_date = current_date_time.toString("yyyy-MM-dd ddd");
        current_time = current_date_time.toString("hh:mm");
    }

    m_timeLabel->setText(current_time);
    m_dateLabel->setText(current_date);

    m_timeLabel->setFont(QFont("Noto Sans CJK SC", 28, 50));
    m_dateLabel->setFont(QFont("Noto Sans CJK SC", 12, 50));
    m_timeLabel->setStyleSheet("color: white; font: 28pt");
    m_dateLabel->setStyleSheet("color: white; font: 12pt");
    m_dateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_timeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    m_dateLabel->setObjectName("date_label");
    m_timeLabel->setObjectName("time_lable");

    m_dateTimeLayout->addStretch();
    m_dateTimeLayout->addWidget(m_timeLabel);
    m_messageVLayout->addSpacing(10);
    //m_dateTimeLayout->setStretch(0,1);
    m_dateTimeLayout->addWidget(m_dateLabel);
    //m_dateTimeLayout->setStretch(1,2);
}

void MainWindow::initialBtnCfg()
{
    bool newIniFile = false;//ini文件是否为新建文件
    QString iniDir = "/usr/share/ukui/ukui-session-manager/config";
    if(!QFile::exists(iniDir + "/btnconfig.ini")){
        qDebug() << "btnconfig.ini file is not exists!!!";

        QDir dir(iniDir);
        if(!dir.exists(iniDir)){
            if(dir.mkdir(iniDir)){//目前创建不成功  没有权限
                QFile iniFile(iniDir + "/btnconfig.ini");
                if(iniFile.open(QIODevice::WriteOnly)){
                    newIniFile = true;
                    iniFile.close();
                }
                qDebug() << "inifile open faile!";
            }
            else
                qDebug() << "create inidir faile!";
        }
    }
    m_btnCfgSetting = new QSettings("/usr/share/ukui/ukui-session-manager/config/btnconfig.ini", QSettings::IniFormat);

    if(newIniFile){//貌似路径下文件只可读不可写
        m_btnCfgSetting->setValue("btn/SwitchUserBtnHide", false);
        m_btnCfgSetting->setValue("btn/HibernateBtnHide", false);
        m_btnCfgSetting->setValue("btn/LockScreenBtnHide", false);
        m_btnCfgSetting->setValue("btn/LogoutBtnHide", false);
        m_btnCfgSetting->setValue("btn/RebootBtnHide", false);
        m_btnCfgSetting->setValue("btn/ShutDownBtnHide", false);
        m_btnCfgSetting->setValue("btn/SuspendBtnHide", false);
    }

    m_btnCfgSetting->setValue("btn/SwitchUserBtnHide", isSwitchuserHide);
    m_btnCfgSetting->setValue("btn/HibernateBtnHide", isHibernateHide);
    m_btnCfgSetting->setValue("btn/SuspendBtnHide", isSuspendHide);
    m_btnCfgSetting->setValue("btn/LogoutBtnHide", isLogoutHide);
    m_btnCfgSetting->setValue("btn/RebootBtnHide", isRebootHide);
    m_btnCfgSetting->setValue("btn/ShutDownBtnHide", isPowerOffHide);

    qDebug() << "isHibernateHide..." << isHibernateHide;
    m_btnHideMap.insert(m_switchUserBtn, isSwitchuserHide);
    m_btnHideMap.insert(m_hibernateBtn, isHibernateHide);
    m_btnHideMap.insert(m_suspendBtn, isSuspendHide);
    m_btnHideMap.insert(m_lockScreenBtn, m_btnCfgSetting->value("btn/LockScreenBtnHide").toBool());
    m_btnHideMap.insert(m_logoutBtn, isLogoutHide);
    m_btnHideMap.insert(m_rebootBtn, isRebootHide);
    m_btnHideMap.insert(m_shutDownBtn, isPowerOffHide);
}

void MainWindow::setLayoutWidgetVisible(QLayout* layout, bool show)
{
    for (int i = 0;i < layout->count(); i++) {
        QLayoutItem*item = layout->layout()->itemAt(i);
        if (item->widget() != nullptr) {
            item->widget()->setVisible(show);
        }
    }
}

void MainWindow::changeBtnState(QString btnName, bool isKeySelect)
{
    for(auto item = map.begin(); item != map.end(); item++)
    {
        item.value()->changeIconBackColor((item.value()->objectName() == btnName), isKeySelect);
    }
}

QString MainWindow::getAppLocalName(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();

    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags,error);

    char *name = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Name", nullptr, nullptr);
    QString namestr = QString::fromLocal8Bit(name);

    g_key_file_free(keyfile);
    return namestr;
}

void MainWindow::mouseReleaseSlots(QEvent *event, QString objName)
{
    for (auto iter = map.begin(); iter != map.end(); iter++) {
        if (iter.value()->getIconLabel()->objectName() == objName) {
            changePoint(iter.value(), event);
            if (event->type() == QEvent::MouseButtonRelease) {
                qDebug() << "mouseReleaseSlots..." << objName;
                doEvent(objName, iter.key());
                return;
            }
        }
    }
}

void MainWindow::screenCountChanged()
{
    QDesktopWidget *desktop = QApplication::desktop();
    qDebug() << "inside screenCountChanged,screenCount = " << desktop->screenCount() << QApplication::desktop()->screenGeometry(QCursor::pos()).width() << QApplication::desktop()->screenGeometry(QCursor::pos()).height();
    //setGeometry(desktop->geometry());
    //updateGeometry();
    //move(0,0);
    //setFixedSize(QApplication::primaryScreen()->virtualSize());
    setGeometry(0, 0, QApplication::primaryScreen()->virtualSize().width(), QApplication::primaryScreen()->virtualSize().height());
    ResizeEvent();
    update();
}

void MainWindow::calculateBtnSpan(int allNum, int lineMaxNum, MyPushButton *btn, int &row, int &colum)
{
    int afterHideNum = 0;
    for (int i = 0; i < m_btnHideMap.count(); i++) {
        if (map.key(btn) > i) {
            if (m_btnHideMap.value(map.value(i))) {
                afterHideNum++;
            }
        }
        else if(map.key(btn) == i)
        {
            colum = (i - afterHideNum)%lineMaxNum;
            row = (i - afterHideNum)/lineMaxNum;
            return;
        }
    }
}

void MainWindow::showNormalBtnWidget(int hideNum)
{
    int margins = 0;

    margins = (m_screen.width() -160 - 128 * (7 - hideNum))/(6-hideNum);
    qDebug() << "showNormalBtnWidget hideNum:" << hideNum << "margins:" << margins;
    int btnWidgetWidth = 0;
    if(margins > 60)
    {
        //m_buttonHLayout->addWidget(m_listView);
        btnWidgetWidth = (128 * (7 - hideNum) + 60 * (6 - hideNum));
        m_buttonHLayout->setHorizontalSpacing(60);
    }
    else
    {
        btnWidgetWidth = 128 * (7 - hideNum) + margins * (6 - hideNum);
        m_buttonHLayout->setHorizontalSpacing(margins);
    }
    m_btnWidget->setGeometry(QRect(0,0,btnWidgetWidth+ 24, 632 * m_screen.height()/1080));
    m_btnWidget->setContentsMargins(0,0,0,100 * m_screen.height()/1080);

    m_scrollArea->setGeometry(QRect(0,0,btnWidgetWidth + 24, 632 * m_screen.height()/1080));

    m_scrollArea->setContentsMargins(0,0,0,0);
    m_scrollArea->verticalScrollBar()->setVisible(false);
    m_scrollArea->verticalScrollBar()->setDisabled(true);

    //m_buttonHLayout->setContentsMargins(0,0,0,(m_scrollArea->height() - m_switchUserBtn->height()) * 3/5);

    for (int i = 0;i < m_buttonHLayout->count(); i++) {
        QLayoutItem*item = m_buttonHLayout->layout()->itemAt(i);
        if (item->widget() != nullptr) {
            m_buttonHLayout->removeWidget(item->widget());
        }
    }

    m_buttonHLayout->addWidget(m_switchUserBtn,0,0);
    m_buttonHLayout->addWidget(m_hibernateBtn,0,1);
    m_buttonHLayout->addWidget(m_suspendBtn,0,2);
    m_buttonHLayout->addWidget(m_lockScreenBtn,0,3);
    m_buttonHLayout->addWidget(m_logoutBtn,0,4);
    m_buttonHLayout->addWidget(m_rebootBtn,0,5);
    m_buttonHLayout->addWidget(m_shutDownBtn,0,6);
    m_btnWidgetNeedScrollbar = false;
}

void MainWindow::showHasScrollBarBtnWidget(int hideNum)
{
    int allBtnNum = 7 - hideNum;
    int lineWidth = m_screen.width() - 160 - 6;
    int lineMaxBtnNum = (lineWidth - 128)/188 + 1;
    m_lineNum = allBtnNum/lineMaxBtnNum + ((allBtnNum%lineMaxBtnNum > 0) ? 1 : 0);
    int needHeight = (m_lineNum * 171 + (m_lineNum - 1) * 32);
    int btnWidgetHeight = 632;

    qDebug() << "showHasScrollBarBtnWidget lineWidth:" << lineWidth << "lineMaxBtnNum:" << lineMaxBtnNum << "needHeight:" << needHeight << "lineNum:" << m_lineNum<< allBtnNum/lineMaxBtnNum << allBtnNum%lineMaxBtnNum;

    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_switchUserBtn, m_switchRow, m_switchColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_hibernateBtn, m_hibernateRow, m_hibernateColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_suspendBtn, m_suspendRow, m_suspendColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_lockScreenBtn, m_lockScreenRow, m_lockScreenColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_logoutBtn, m_logoutRow, m_logoutColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_rebootBtn, m_rebootRow, m_rebootColumn);
    calculateBtnSpan(allBtnNum, lineMaxBtnNum, m_shutDownBtn, m_shutDownRow, m_shutDownColumn);

//        qDebug() << "switchRow:" << m_switchRow << m_switchColumn;
//        qDebug() << "hibernateRow:" << m_hibernateRow << m_hibernateColumn;
//        qDebug() << "suspendRow:" << m_suspendRow << m_suspendColumn;
//        qDebug() << "lockScreenRow:" << m_lockScreenRow << m_lockScreenColumn;
//        qDebug() << "logoutRow:" << m_logoutRow << m_logoutColumn;
//        qDebug() << "rebootRow:" << m_rebootRow << m_rebootColumn;
//        qDebug() << "shutDownRow:" << m_shutDownRow << m_shutDownColumn;

    {
        m_scrollArea->verticalScrollBar()->setVisible(!m_judgeLabel->isVisible());
        m_scrollArea->verticalScrollBar()->setDisabled(false);
        m_scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar{ background: transparent; margin-top:0px;margin-bottom:0px ; }"\
                                                         "QScrollBar:vertical{width: 6px;background: transparent;border-radius:3px;}"\
                                                         "QScrollBar::handle:vertical{width: 6px; background: rgba(255,255,255, 40); border-radius:3px;}"\
                                                         "QScrollBar::handle:vertical:hover{width: 6px; background: rgba(255,255,255, 60); border-radius:3px;}"\
                                                         "QScrollBar::add-line:vertical{width:0px;height:0px}"\
                                                         "QScrollBar::sub-line:vertical{width:0px;height:0px}");

        qDebug() << "set bar pos...";
        //m_scrollArea->verticalScrollBar()->setGeometry(QRect(lineWidth + 20, 0, 6, 100));
        m_scrollArea->setContentsMargins(0,0,0,0);
        m_scrollArea->setGeometry(QRect(0,0,128 * lineMaxBtnNum + 60 * (lineMaxBtnNum-1) + 6, btnWidgetHeight * m_screen.height()/1080));

        m_buttonHLayout->setContentsMargins(0,0,0,0);// (needHeight > 632 ? 0 : (632 - needHeight)) * m_screen.height()/1080);

        m_btnWidget->setGeometry(QRect(0,0,128 * lineMaxBtnNum + 60 * (lineMaxBtnNum-1), needHeight));
        m_btnWidget->setContentsMargins(6,0,12,0);
        qDebug() << "m_btnWidget FixedHeight:" << needHeight << btnWidgetHeight << m_btnWidget->width() << m_scrollArea->width();
        qDebug() << "isSwitchuserHide:" << isSwitchuserHide << "isHibernateHide:" << isHibernateHide << "isSuspendHide:" << isSuspendHide;

        for (int i = 0;i < m_buttonHLayout->count(); i++) {
            QLayoutItem*item = m_buttonHLayout->layout()->itemAt(i);
            if (item->widget() != nullptr) {
                m_buttonHLayout->removeWidget(item->widget());
            }
        }
        m_buttonHLayout->addWidget(m_switchUserBtn, m_switchRow, m_switchColumn);
        m_buttonHLayout->addWidget(m_hibernateBtn, m_hibernateRow, m_hibernateColumn);
        m_buttonHLayout->addWidget(m_suspendBtn, m_suspendRow, m_suspendColumn);
        m_buttonHLayout->addWidget(m_lockScreenBtn, m_lockScreenRow, m_lockScreenColumn);
        m_buttonHLayout->addWidget(m_logoutBtn, m_logoutRow, m_logoutColumn);
        m_buttonHLayout->addWidget(m_rebootBtn, m_rebootRow, m_rebootColumn);
        m_buttonHLayout->addWidget(m_shutDownBtn, m_shutDownRow, m_shutDownColumn);
        m_buttonHLayout->setHorizontalSpacing(60);
        m_buttonHLayout->setAlignment(Qt::AlignHCenter);
        m_btnWidgetNeedScrollbar = true;
    }
}

void MainWindow::ResizeEvent()
{
    m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    setGeometry(0, 0, QApplication::primaryScreen()->virtualSize().width(), QApplication::primaryScreen()->virtualSize().height());

    if(m_showWarningMesg)
    {
        showInhibitWarning();
        m_toolWidget->setGeometry(m_screen);
        return;
    }

    qDebug() << "ResizeEvent moveWidget  m_screen:" << m_screen.width() << m_screen.height();
    int hideNum = 0;
    for (int i = 0; i < m_btnHideMap.count(); i++) {
        if (m_btnHideMap.value(map.value(i))) {
            hideNum++;
        }
    }

    // Move the widget to the direction where they should be
    for (int i = 0; i <= 6; i++) {
        if (m_btnHideMap.value(map.value(i)))
        {
            map[i]->hide();
        }
        else
        {
            map[i]->show();
        }
    }
    if((m_screen.width() -160 - 128 * (7 - hideNum))/(6-hideNum) >= 16)
    {
        showNormalBtnWidget(hideNum);
    }
    else
    {
        showHasScrollBarBtnWidget(hideNum);
    }
    m_btnWidget->setStyleSheet("QWidget#btnWidget{background-color: transparent;}");
    m_btnWidget->setLayout(m_buttonHLayout);

    m_scrollArea->horizontalScrollBar()->setVisible(false);
    m_scrollArea->horizontalScrollBar()->setDisabled(true);
    m_scrollArea->setWidget(m_btnWidget);
    m_scrollArea->setStyleSheet("QScrollArea#scrollArea{background-color: transparent;}");
    m_scrollArea->setAlignment(Qt::AlignHCenter);
    qDebug() << "m_scrollArea geometry:" << m_scrollArea->geometry();
    qDebug() << "m_btnWidget geometry:" << m_btnWidget->geometry();
    qDebug() << "m_buttonHLayout geometry:" << m_buttonHLayout->geometry();

    m_vBoxLayout->setContentsMargins((m_screen.width() - m_scrollArea->width() - 20)/2,0,(m_screen.width() - m_scrollArea->width() - 20)/2,0);

    //m_scrollArea->adjustSize();
    //m_scrollArea->setWidgetResizable(true);
    m_toolWidget->setGeometry(m_screen);
}

// Paint the background picture
void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(0, 0, 0, 78));   // 178
    for (QScreen *screen: QApplication::screens()) {
        // draw picture to every screen
        QRect rect = screen->geometry();
        painter.drawPixmap(rect, pix.scaled(screen->size()));
        //drawRect可以避免在白色壁纸的情况下模糊背景造成的问题
        painter.drawRect(rect);
    }
    QWidget::paintEvent(e);
}

//lock screen
void MainWindow::doLockscreen()
{
    QDBusInterface *interface = new QDBusInterface("org.ukui.ScreenSaver",
                                                   "/",
                                                   "org.ukui.ScreenSaver"
                                                   );
    /*监听锁屏起来的信号，再执行界面退出操作，规避点击锁屏后先显示桌面再打开锁屏
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                         QString("/"),
                                         QString("org.ukui.ScreenSaver"),
                                         QString("lock"), this, SLOT(exitt()));*/
    QDBusMessage msg = interface->call("Lock");
    //延迟界面退出操作，规避点击锁屏后先显示桌面再打开锁屏
//    QTimer::singleShot(500, this, SLOT(exitt()));
    exit(0);
}

/*
void MainWindow::doSystemMonitor()
{
    qDebug() << "doSystemMonitor....";
    QProcess::startDetached("ukui-system-monitor", QStringList());
    exitt();
}
*/

// handle mouse-clicked event
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    /*
    if (obj->objectName() == "systemMonitor") {
           if (event->type() == QEvent::MouseButtonRelease) {
               doSystemMonitor();
           }
           else{
               if(event->type() == QEvent::Leave){
                   QString str = "QWidget#systemMonitor{background-color: transparent;border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
                   m_systemMonitorBtn->setStyleSheet(str);
                   m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);

//                   tableNum   = -1;
//                   flag = false;
//                   changeBtnState("empty");
               }
               else if(event->type() == QEvent::Enter){
                   QString str = "QWidget#systemMonitor{background-color: rgb(255,255,255,40);border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
                   m_systemMonitorBtn->setStyleSheet(str);
                   m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);

//                   tableNum   = -1;
//                   flag = true;
//                   changeBtnState("empty");
               }
               else if(event->type() == QEvent::MouseButtonPress){
                   QString str = "QWidget#systemMonitor{background-color: rgb(255,255,255,80);border-radius: " + QString::number(m_systemMonitorBtn->height()/2) + "px;}";
                   m_systemMonitorBtn->setStyleSheet(str);
                   m_systemMonitorBtn->setAttribute(Qt::WA_StyledBackground);
               }
           }
       }
    */
    return QWidget::eventFilter(obj, event);
}

void MainWindow::changePoint(QWidget *widget, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        changeBtnState(widget->objectName());
    }
    if (event->type() == QEvent::Leave) {
        flag       = false;
    }
}

void MainWindow::doEvent(QString test, int i)
{
    qDebug() << "doevent... i:" << i << test << inhibitShutdown << inhibitSleep << close_system_needed_to_confirm;
    defaultnum = i;
    if (inhibitShutdown && (i ==5 || i ==6)) {
        //显示禁止shutdown的提示信息
        qDebug()<<"showInhibitWarning...";
        showInhibitWarning();
    } else if (inhibitSleep && (i == 2 || i == 1)) {
        //显示禁止sleep的提示信息
        qDebug()<<"showInhibitWarning...";
        showInhibitWarning();
    } else if (close_system_needed_to_confirm && (i == 5 || i == 6)) {
        connect(this, &MainWindow::confirmButtonclicked, [&](){
            gs->set("win-key-release", false);
            qDebug() << "Start do action" << defaultnum;
            if (closeGrab()) {
                qDebug() << "success to close Grab";
            } else {
                qDebug() << "failure to close Grab";
            }
            // this->hide();
            emit signalTostart();
        });
        this->judgeboxShow();
    } else {
        try {
            gs->set("win-key-release", false);

            defaultnum = i;
            qDebug() << "Start do action" << test << defaultnum;
            if (click_blank_space_need_to_exit) {
                if (closeGrab()) {
                    qDebug() << "success to close Grab";
                } else {
                    qDebug() << "failure to close Grab";
                }
            }
            //this->hide();
            if (i == 3) {
                doLockscreen();
            } else {
                this->hide();
                emit signalTostart();
            }
        } catch (QException &e) {
            qWarning() << e.what();
        }
    }
}

// handle the blank-area mousePressEvent
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (click_blank_space_need_to_exit) {
        /*QPainterPath path;
        QRect rect(m_systemMonitorBtn->geometry());
        const qreal radius = m_systemMonitorBtn->height()/2;
        path.moveTo(rect.topRight() - QPointF(radius, 0));
        path.lineTo(rect.topLeft() + QPointF(radius, 0));
        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
        path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
        path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
        path.lineTo(rect.bottomRight() - QPointF(radius, 0));
        path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
        path.lineTo(rect.topRight() + QPointF(0, radius));
        path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));
*/

        qDebug() << "mousePressEvent" << m_switchUserBtn->geometry() << m_scrollArea->mapFromGlobal(event->pos());

        if (!m_suspendBtn->getIconLabel()->containsPoint(m_suspendBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_hibernateBtn->getIconLabel()->containsPoint(m_hibernateBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_lockScreenBtn->getIconLabel()->containsPoint(m_lockScreenBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_switchUserBtn->getIconLabel()->containsPoint(m_switchUserBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_logoutBtn->getIconLabel()->containsPoint(m_logoutBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_rebootBtn->getIconLabel()->containsPoint(m_rebootBtn->getIconLabel()->mapFromGlobal(event->pos()))
            && !m_shutDownBtn->getIconLabel()->containsPoint(m_shutDownBtn->getIconLabel()->mapFromGlobal(event->pos()))) {
            exitt();
        }

    }
}

bool MainWindow::exitt()
{
    gs->set("win-key-release", false);
    if (closeGrab()) {
        qDebug() << "success to close Grab";
    } else {
        qDebug() << "failure to close Grab";
    }
    close();
    exit(0);
}

bool MainWindow::judgeBtnIsEnable(int index)
{
    if (!m_btnHideMap.value(map.value(index)) && (((lockfile && index != 1 && index != 5 && index != 6 )&&
                                              (!lockuser || (lockuser && index != 4))) ||!lockfile)) {
        return true;
    }
    return false;
}

void MainWindow::calculateKeyBtn(const QString &key)
{
    if (key == "Left") {
        if (tableNum == 0 || tableNum == -1) {
            for (int i = 6; i >= 0; i--) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
        } else {
            for (int i = tableNum - 1; i >= 0; i--) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
            for (int i = 6; i >= tableNum + 1; i--) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
        }
    }
    if (key == "Right") {
        if (tableNum == 6 || tableNum == -1) {
            for (int i = 0; i <= 6; i++) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
        } else {
            for (int i = tableNum + 1; i <= 6; i++) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
            for (int i = 0; i <= tableNum - 1; i++) {
                if (judgeBtnIsEnable(i)) {
                    tableNum = i;
                    return;
                }
            }
        }
    }
}

void MainWindow::onGlobalKeyPress(const QString &key)
{
}

// handle "Esc","Left","Right","Enter" keyPress event
void MainWindow::onGlobalkeyRelease(const QString &key)
{
    qDebug() << "key: " << key << "flag:" << flag << "click_blank_space_need_to_exit:" << click_blank_space_need_to_exit;

    if (!click_blank_space_need_to_exit) {
        return;
    }

    if (key == "Escape") {
        exitt();
    } else if (key == "Left" || key == "Right") {
        //if (flag) return;
        calculateKeyBtn(key);
        QString button = map[tableNum]->objectName();

        if(m_btnWidgetNeedScrollbar)
        {
            int currentLine = rowMap[tableNum];
            QScrollBar *scrollBar = m_scrollArea->verticalScrollBar();
            if(currentLine == 0)
                scrollBar->setValue(0);
            else
            {
                int sValue = scrollBar->maximum() - scrollBar->minimum();
                float scale = (currentLine * 180 + currentLine * m_buttonHLayout->verticalSpacing() + 128) * 1.0 / (m_btnWidget->height() * 1.0);
                scrollBar->setValue(sValue * scale);
            }
        }
        changeBtnState(button, true);
    }
    else if (key == "Return" || key == "KP_Enter") {   // space,KP_Enter
        for(auto item = map.begin(); item != map.end(); item++)
        {
            if(item.value()->getIsKeySelect())
                return doEvent(item.value()->objectName(), item.key());
        }
        for(auto item = map.begin(); item != map.end(); item++)
        {
            if(item.value()->getIsMouseSelect())
                return doEvent(item.value()->objectName(), item.key());
        }
    }
}

void MainWindow::showInhibitWarning()
{
    QRect mainScreen;
    QList<QScreen*> screens = QApplication::screens();
    QPoint ptf(QCursor::pos());
    for (QScreen *screen : screens) {
        QRect rec = screen->geometry();
        if (rec.contains(ptf)) {
            mainScreen = rec;//获取鼠标所在屏幕
        }
    }

    click_blank_space_need_to_exit = false;
    for (int j = 0; j < 7; j++) {
        map[j]->hide();//隐藏界面上原有的部件
    }

    m_scrollArea->verticalScrollBar()->setVisible(false);
    drawWarningWindow(mainScreen);
}

void MainWindow::drawWarningWindow(QRect &rect)
{
    int xx = rect.x();
    int yy = rect.y();//用于设置相对位置

    bool isEnoughBig = m_screen.height() - 266 - 467 > 0 ? true : false;

    m_showWarningArea->setGeometry(0, 0, isEnoughBig ? 740 : 1200 * m_screen.width()/1920, isEnoughBig ? 467 : (500 * m_screen.height()/1080));
    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    //顶部提醒信息
    QLabel *tips = new QLabel(m_showWarningArea);
    tips->setObjectName(QString::fromUtf8("tips"));
    tips->setGeometry(0, 0, isEnoughBig ? 740 : m_showWarningArea->width(), 27);
    tips->setWordWrap(true);

    QString str;
    //defaultnum会在doevent中初始化为按钮的编号，结合defaultnum判断可以保证sleep和shutdown都被阻止时能够正确显示信息
    if (inhibitSleep) {
        if(defaultnum == 1)
            str = QObject::tr("The following program is running to prevent the system from hibernate!");
        else if(defaultnum == 2)
            str = QObject::tr("The following program is running to prevent the system from suspend!");
    }
    if (inhibitShutdown) {
        if(defaultnum ==5)
            str = QObject::tr("The following program is running to prevent the system from reboot!");
        else if(defaultnum ==6)
            str = QObject::tr("The following program is running to prevent the system from shutting down!");
    }
    tips->setText(str);
    tips->setAlignment(Qt::AlignCenter);
    tips->setContentsMargins(0,0,0,0);
    tips->setStyleSheet(QString::fromUtf8("color:white;font:14pt;"));

    //数据模型
    QStandardItemModel *model = new QStandardItemModel(this);
    QStringList appNameList;
    if (inhibitSleep && (defaultnum == 2 || defaultnum == 1))
    {
        appNameList = sleepInhibitors;
    }
    else if (inhibitShutdown && (defaultnum ==5 || defaultnum ==6))
    {
        appNameList = shutdownInhibitors;
    }
    for (int i = 0; i < appNameList.length(); ++i) {
        QIcon icon;
        QString oneAppName;
        QString iconName;
        QString appName = appNameList.at(i);
        QMap<QString, QString> nameAndIcon = findNameAndIcon(appName);
        if (nameAndIcon.size() != 0) {
            oneAppName = nameAndIcon.begin().key();
            iconName = nameAndIcon.begin().value();
        }

        if (!iconName.isEmpty() && QIcon::hasThemeIcon(iconName)) {
            icon = QIcon::fromTheme(iconName);
        } else if (QIcon::hasThemeIcon("application-x-desktop")) {
            icon = QIcon::fromTheme("application-x-desktop");
        }

        if (!oneAppName.isEmpty()) {
            oneAppName.swap(appName);
        }

        model->appendRow(new QStandardItem(icon, appName));
    }

    //列表视图
    QListView *applist = new QListView(m_showWarningArea);
    applist->setObjectName(QString::fromUtf8("applist"));
    if(isEnoughBig)
        applist->setFixedSize(520 * (isEnoughBig ? 1: m_screen.width()/1920), 320 * (isEnoughBig ? 1 : m_screen.height()/1080));
    else
        applist->setGeometry(0,0,520 * (isEnoughBig ? 1: m_screen.width()/1920), 320 * (isEnoughBig ? 1 : m_screen.height()/1080));
    applist->verticalScrollMode();
    applist->setStyleSheet("font:10pt;color:white;");
    applist->setEditTriggers(QAbstractItemView::NoEditTriggers);
    applist->setIconSize(QSize(32,32));
    applist->setModel(model);
    applist->setMinimumHeight(40);
    applist->verticalScrollBar()->setStyleSheet("QScrollBar{ background: transparent; margin-top:0px;margin-bottom:0px ; }"\
                                                "QScrollBar:vertical{width: 6px;background: transparent;border-radius:3px;}"\
                                                "QScrollBar::handle:vertical{width: 6px; background: rgba(255,255,255, 40); border-radius:3px;}"\
                                                "QScrollBar::handle:vertical:hover{width: 6px; background: rgba(255,255,255, 60); border-radius:3px;}"\
                                                "QScrollBar::add-line:vertical{width:0px;height:0px}"\
                                                "QScrollBar::sub-line:vertical{width:0px;height:0px}");

    //继续操作按钮
    QHBoxLayout *hBoxLayout = new QHBoxLayout();
    QString confirBTnText = "";

    if (inhibitSleep) {
        if(defaultnum == 1)
            confirBTnText = (QObject::tr("Still Hibernate"));
        else if(defaultnum == 2)
            confirBTnText = (QObject::tr("Still Suspend"));
    }
    if (inhibitShutdown) {
        if(defaultnum ==5)
            confirBTnText = (QObject::tr("Still Reboot"));
        else if(defaultnum ==6)
            confirBTnText = (QObject::tr("Still Shutdown"));
    }
    CommonPushButton *confirmBtn = new CommonPushButton(confirBTnText, QString::fromUtf8("confirmBtn"), 120, 48, m_showWarningArea);

    connect(confirmBtn, &CommonPushButton::clicked, [this]() {
        gs->set("win-key-release", false);
        qDebug() << "Start do action" << defaultnum;
        if (closeGrab()) {
            qDebug() << "success to close Grab";
        } else {
            qDebug() << "failure to close Grab";
        }
        this->signalTostart();
    });

    //取消按钮
    CommonPushButton *cancelBtn = new CommonPushButton(QObject::tr("Cancel"), QString::fromUtf8("cancelBtn"), 120, 48, m_showWarningArea);

    connect(cancelBtn, &CommonPushButton::clicked, this, &MainWindow::exitt);

    qDebug() << "applist->width():" << applist->width() << cancelBtn->width();
    //hBoxLayout->setContentsMargins(0,0,0,0);
    //hBoxLayout->setSpacing(isEnoughBig ? 24 : 12);
    hBoxLayout->addStretch();
    //hBoxLayout->addWidget(confirmBtn);
    hBoxLayout->addWidget(cancelBtn);
    hBoxLayout->addStretch();

    vBoxLayout->addWidget(tips);
    vBoxLayout->addWidget(applist, 0, Qt::AlignHCenter);
    vBoxLayout->addSpacing(isEnoughBig ? 32 : 0);
    vBoxLayout->addLayout(hBoxLayout, Qt::AlignHCenter);

    //移动整个区域到指定的相对位置
    m_showWarningArea->move(xx + (m_screen.width() - m_showWarningArea->width()) / 2, (yy + 266 * m_screen.height()/1080) + (isEnoughBig ? 0 : 10));
//    applist->move((area->width() - applist->width()) / 2, isEnoughBig ? 51 : 32);
    m_showWarningArea->setContentsMargins(0,0,0,0);
    m_showWarningArea->setLayout(vBoxLayout);
    m_showWarningArea->show();
    m_showWarningMesg = true;
}

QMap<QString, QString> MainWindow::findNameAndIcon(QString &basename)
{
    QMap<QString, QString> nameAndIcon;
    QString icon;
    QString name;
    QStringList desktop_paths;
    desktop_paths << "/usr/share/applications";
    desktop_paths << "/etc/xdg/autostart";

    for (const QString &dirName : const_cast<const QStringList&>(desktop_paths)) {
        QDir dir(dirName);
        if (!dir.exists()) {
            continue;
        }

        const QFileInfoList files = dir.entryInfoList(QStringList(QLatin1String("*.desktop")), QDir::Files | QDir::Readable);
        for (const QFileInfo &fi : files) {
            QString base = fi.baseName();
            if (base == basename) {
                XdgDesktopFile desktopFile;
                desktopFile.load(fi.absoluteFilePath());
                icon = desktopFile.value("Icon").toString();
//                Name = desktopFile.value("Name[zh_CN]").toString();
                name = getAppLocalName(fi.absoluteFilePath());//根据系统的本地语言设置获取对应的名称
                nameAndIcon[name] = icon;
            }
        }
    }

    return nameAndIcon;
}

void MainWindow::judgeboxShow()
{
    QRect m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    click_blank_space_need_to_exit = false;
    for (int j = 0; j < 7; j++) {
        map[j]->hide();
    }
    m_scrollArea->verticalScrollBar()->setVisible(false);

    setLayoutWidgetVisible(m_dateTimeLayout, false);
    setLayoutWidgetVisible(m_judgeWidgetVLayout, true);
    setLayoutWidgetVisible(m_judgeBtnHLayout, true);
}

void MainWindow::keyPressEmulate()
{
    QTimer::singleShot(500, this, [&](){
        qDebug()<<"Emulate press key A";
        XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_A), True, 1);
        XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(),XK_A), False, 1);
        //XFlush(QX11Info::display());
    });
}

//void MainWindow::closeEvent(QCloseEvent *event)
//{
//    qDebug()<<"MainWindow:: CloseEvent";
//    if (closeGrab()) {
//        qDebug()<<"success to close Grab";
//    } else {
//        qDebug()<<"failure to close Grab";
//    }
//    return QWidget::closeEvent(event);
//}


bool MainWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
        return false;
    }
    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
    const uint8_t responseType = event->response_type & ~0x80;
    if (responseType == XCB_CONFIGURE_NOTIFY) {
        xcb_configure_notify_event_t *xc = reinterpret_cast<xcb_configure_notify_event_t*>(event);
        if (xc->event == QX11Info::appRootWindow()) {
            XRaiseWindow(QX11Info::display(), this->winId());
            XFlush(QX11Info::display());
            //raise();
        }
        return false;
    } else if (responseType == XCB_PROPERTY_NOTIFY) {
        //raise();
        XRaiseWindow(QX11Info::display(), this->winId());
        XFlush(QX11Info::display());
    }
    return false;
}

