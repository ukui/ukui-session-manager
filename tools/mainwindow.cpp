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

#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <glib.h>

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

    //Make a hash-map to store tableNum-to-lastWidget
    if (m_power->canAction(UkuiPower::PowerHibernate)) {//m_power->canAction(UkuiPower::PowerHibernate)
        isHibernateHide = false;
    }

    if (LockChecker::getCachedUsers() > 1) {
        isSwitchuserHide = false;
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

    m_vBoxLayout->addStretch(20);
    m_vBoxLayout->addLayout(m_dateTimeLayout, 60);
    m_vBoxLayout->addStretch(60);
    m_vBoxLayout->addLayout(m_judgeWidgetVLayout, 140);
    if (m_screen.width() > 1088) {
        m_vBoxLayout->addLayout(m_buttonHLayout, 140);
    } else {
        m_vBoxLayout->addLayout(m_buttonHLayout, 280);
    }
    m_vBoxLayout->addStretch(174);
    m_vBoxLayout->addLayout(m_messageVLayout, 80);
    m_vBoxLayout->addStretch(106);

    //根据屏幕分辨率与鼠标位置重设界面
    //m_screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    setFixedSize(QApplication::primaryScreen()->virtualSize());
    move(0, 0);//设置初始位置的值
    ResizeEvent();

    //设置窗体无边框，不可拖动拖拽拉伸;为顶层窗口，无法被切屏;不使用窗口管理器
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    // setAttribute(Qt::WA_TranslucentBackground, true);//设定该窗口透明显示
    m_toolWidget->setLayout(m_vBoxLayout);
    //setCentralWidget(m_toolWidget);

    qDebug() << "m_toolWidget...." << m_toolWidget->geometry();
    qDebug() << "pos..." << QCursor::pos() << this->geometry();
    qDebug() << "m_screen..." << m_screen;

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

    qApp->installNativeEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete m_power;
    delete xEventMonitor;
    delete ui;
}

void MainWindow::initialBtn()
{
    m_switchUserBtn = new MyPushButton(m_btnImagesPath+"/switchuser.svg", QApplication::tr("Switch User"), "switchuser", m_toolWidget, !m_Is_UKUI_3_1);
    m_hibernateBtn = new MyPushButton(m_btnImagesPath+"/hibernate.svg", QApplication::tr("Hibernate"), "hibernate", m_toolWidget, !m_Is_UKUI_3_1);
    m_suspendBtn = new MyPushButton(m_btnImagesPath+"/suspend.svg", QApplication::tr("Suspend"), "suspend", m_toolWidget, !m_Is_UKUI_3_1);
    m_logoutBtn = new MyPushButton(m_btnImagesPath+"/logout.svg", QApplication::tr("Logout"), "logout", m_toolWidget, !m_Is_UKUI_3_1);
    m_rebootBtn = new MyPushButton(m_btnImagesPath+"/reboot.svg", QApplication::tr("Reboot"), "reboot", m_toolWidget, !m_Is_UKUI_3_1);
    m_shutDownBtn = new MyPushButton(m_btnImagesPath+"/shutdown.svg", QApplication::tr("Shut Down"), "shutdown", m_toolWidget, !m_Is_UKUI_3_1);
    m_lockScreenBtn = new MyPushButton(m_btnImagesPath+"/lockscreen.svg", QApplication::tr("Lock Screen"), "lockscreen", m_toolWidget, !m_Is_UKUI_3_1);

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
    int margins = 0;
    if (m_screen.width() > 1088) {
        margins = (m_screen.width() - 60 * 6 - 140 * 7) / 2;
    } else {
        margins = (m_screen.width() - 60 * 2 - 140 * 3) / 2;
    }
//    QStringList userlist = getLoginedUsers();
    QStringList userlist = LockChecker::getLoginedUsers();
    if (userlist.count() > 1) {
        close_system_needed_to_confirm = true;
    }
    QString tips = QApplication::tr("Multiple users are logged in at the same time.Are you sure "
                                    "you want to close this system?");
    m_judgeLabel->setText(tips);
    m_judgeLabel->setStyleSheet("color:white;font:14pt;");
    m_judgeLabel->setObjectName("label");
    m_judgeLabel->setFixedWidth(m_screen.width() - 2 * margins);
    qDebug() << "m_judgeLabel width:" << m_judgeLabel->width();
    m_judgeLabel->setAlignment(Qt::AlignHCenter| Qt::AlignBottom);
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

    m_judgeWidgetVLayout->addWidget(m_judgeLabel);
    m_judgeWidgetVLayout->addSpacing(30);
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

    m_messageLabel1->setGeometry(QRect(0, 0, 700, 40));
    m_messageLabel1->setGeometry(QRect(0, 0, 700, 40));
    m_messageLabel1->setStyleSheet("color:white;font:12pt;");
    m_messageLabel2->setStyleSheet("color:white;font:12pt;");
    m_messageLabel1->setObjectName("messagelabel1");
    m_messageLabel2->setObjectName("messagelabel2");
    m_messageLabel1->setFixedWidth(m_screen.width() - 2 * margins);
    m_messageLabel2->setFixedWidth(m_screen.width() - 2 * margins);
    m_messageLabel1->setWordWrap(true);
    m_messageLabel2->setWordWrap(true);
    m_messageLabel2->setAlignment(Qt::AlignCenter);
    m_messageLabel1->setAlignment(Qt::AlignCenter);

    m_messageVLayout->addWidget(m_messageLabel1);
    m_messageVLayout->addSpacing(20);
    m_messageVLayout->addWidget(m_messageLabel2);
    m_messageVLayout->setAlignment(Qt::AlignCenter);

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

    m_dateTimeLayout->addWidget(m_timeLabel);
    m_dateTimeLayout->setStretch(0,1);
    m_dateTimeLayout->addWidget(m_dateLabel);
    m_dateTimeLayout->setStretch(1,2);
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

    qDebug() << "isHibernateHide..." << isHibernateHide;
    m_btnHideMap.insert(m_switchUserBtn, isSwitchuserHide);
    m_btnHideMap.insert(m_hibernateBtn, isHibernateHide);
    m_btnHideMap.insert(m_suspendBtn, m_btnCfgSetting->value("btn/SuspendBtnHide").toBool());
    m_btnHideMap.insert(m_lockScreenBtn, m_btnCfgSetting->value("btn/LockScreenBtnHide").toBool());
    m_btnHideMap.insert(m_logoutBtn, m_btnCfgSetting->value("btn/LogoutBtnHide").toBool());
    m_btnHideMap.insert(m_rebootBtn, m_btnCfgSetting->value("btn/RebootBtnHide").toBool());
    m_btnHideMap.insert(m_shutDownBtn, m_btnCfgSetting->value("btn/ShutDownBtnHide").toBool());
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
    if (event->type() == QEvent::Leave) {
        if (objName == "switchuser_button" || objName == "hibernate_button" ||
                objName == "suspend_button" || objName == "lockscreen_button" ||
                objName == "logout_button" || objName == "reboot_button" ||
                objName == "shutdown_button"){
        changeBtnState("empty");
        //return;
        }
    }

    for (auto iter = map.begin(); iter != map.end(); iter++) {
        if (iter.value()->getIconLabel()->objectName() == objName) {
            changePoint(iter.value(), event, iter.key());
            if (event->type() == QEvent::MouseButtonRelease) {
                doEvent(objName, iter.key());
                return;
            }
        }
    }
}

void MainWindow::screenCountChanged()
{
    QDesktopWidget *desktop = QApplication::desktop();
    qDebug() << "inside screenCountChanged,screenCount = " << desktop->screenCount();
    //setGeometry(desktop->geometry());
    //updateGeometry();
    //move(0,0);
    setFixedSize(QApplication::primaryScreen()->virtualSize());
    ResizeEvent();
    update();
}

void MainWindow::calculateBtnSpan(int allNum, MyPushButton* btn, int& colum, int& row)
{
    int afterHideNum = 0;
    for (int i = 0; i < m_btnHideMap.count(); i++) {
        if (map.key(btn) > i) {
            if (m_btnHideMap.value(map.value(i))) {
                afterHideNum++;
            }
        }
    }
    int dowmRows = allNum / 2 + allNum % 2;
    int upRows = allNum - dowmRows;
    //qDebug() << "allNum:" << allNum << dowmRows << upRows << afterHideNum;
    if ((map.key(btn) - afterHideNum) < upRows) {//在上面
        row = allNum%2 + (map.key(btn) - afterHideNum) * 2;
        colum = 0;
    } else {//在下面
        row = (map.key(btn) - afterHideNum - upRows) * 2;
        colum = 1;
    }
}

void MainWindow::ResizeEvent()
{
    int xx = m_screen.x();
    int yy = m_screen.y();   //取得当前鼠标所在屏幕的最左，上坐标

    qDebug() << "moveWidget  m_screen:" << m_screen.width() << m_screen.height();
    int hideNum = 0;
    for (int i = 0; i < m_btnHideMap.count(); i++) {
        if (m_btnHideMap.value(map.value(i))) {
            hideNum++;
        }
    }

    // Move the widget to the direction where they should be
    int spaceW, spaceH;
    int sum = 0;
    int k   = 0;
    int recWidth;
    if (m_Is_UKUI_3_1) {
        recWidth = 128;
    } else {
        recWidth = 140;
    }
    for (int i = 0; i <= 6; i++) {
        if (m_btnHideMap.value(map.value(i))) {
            map[i]->hide();
        }
    }
    if (m_Is_UKUI_3_1 || (!m_Is_UKUI_3_1 && (m_screen.width() > 1088 || (7 - hideNum) <= 4))) {
        int margins = 0;
        qDebug() << "margins::::" << (m_screen.width() - recWidth * (6 - hideNum))/(7 - hideNum);

        if ((m_screen.width() - recWidth * (7 - hideNum)) / (8 - hideNum) > 60) {
            margins = (m_screen.width() - 60 * (6 - hideNum) - recWidth * (7 - hideNum)) / 2;
        } else {
            margins = (m_screen.width() - recWidth * (7 - hideNum)) / (8 - hideNum);
        }
        qDebug() << "margins:" << margins;
        m_buttonHLayout->setContentsMargins(margins, 0, margins, 0);
        m_buttonHLayout->addWidget(m_switchUserBtn, 0, 0);
        m_buttonHLayout->addWidget(m_hibernateBtn, 0, 1);
        m_buttonHLayout->addWidget(m_suspendBtn, 0, 2);
        m_buttonHLayout->addWidget(m_lockScreenBtn, 0, 3);
        m_buttonHLayout->addWidget(m_logoutBtn, 0, 4);
        m_buttonHLayout->addWidget(m_rebootBtn, 0, 5);
        m_buttonHLayout->addWidget(m_shutDownBtn, 0, 6);
        m_buttonHLayout->setAlignment(Qt::AlignHCenter);
    } else {
        int margins = 0;
        int allBtnNum = 7 - hideNum;
        int rows = allBtnNum / 2 + allBtnNum % 2;
        qDebug() << "margins:" << margins;
        margins = (m_screen.width() - 40 * (rows - 1) - recWidth * rows) / 2;
        qDebug() << "ResizeEvent margins:" << margins;

        int switchRow, switchColumn;
        int hibernateRow, hibernateColumn;
        int suspendRow, suspendColumn;
        int lockScreenRow, lockScreenColumn;
        int logoutRow, logoutColumn;
        int rebootRow, rebootColumn;
        int shutDownRow, shutDownColumn;

        calculateBtnSpan(allBtnNum, m_switchUserBtn, switchRow, switchColumn);
        calculateBtnSpan(allBtnNum, m_hibernateBtn, hibernateRow, hibernateColumn);
        calculateBtnSpan(allBtnNum, m_suspendBtn, suspendRow, suspendColumn);
        calculateBtnSpan(allBtnNum, m_lockScreenBtn, lockScreenRow, lockScreenColumn);
        calculateBtnSpan(allBtnNum, m_logoutBtn, logoutRow, logoutColumn);
        calculateBtnSpan(allBtnNum, m_rebootBtn, rebootRow, rebootColumn);
        calculateBtnSpan(allBtnNum, m_shutDownBtn, shutDownRow, shutDownColumn);
        //qDebug() << "switchRow:" << switchRow << switchColumn;
        //qDebug() << "hibernateRow:" << hibernateRow << hibernateColumn;
        //qDebug() << "suspendRow:" << suspendRow << suspendColumn;
        //qDebug() << "lockScreenRow:" << lockScreenRow << lockScreenColumn;
        //qDebug() << "logoutRow:" << logoutRow << logoutColumn;
        //qDebug() << "rebootRow:" << rebootRow << rebootColumn;
        //qDebug() << "shutDownRow:" << shutDownRow << shutDownColumn;
        m_buttonHLayout->setContentsMargins(margins, 0, margins, 0);
        m_buttonHLayout->addWidget(m_switchUserBtn, switchRow, switchColumn, 1, 2);
        m_buttonHLayout->addWidget(m_hibernateBtn, hibernateRow, hibernateColumn, 1, 2);
        m_buttonHLayout->addWidget(m_suspendBtn, suspendRow, suspendColumn, 1, 2);
        m_buttonHLayout->addWidget(m_lockScreenBtn, lockScreenRow, lockScreenColumn, 1, 2);
        m_buttonHLayout->addWidget(m_logoutBtn, logoutRow, logoutColumn, 1, 2);
        m_buttonHLayout->addWidget(m_rebootBtn, rebootRow, rebootColumn, 1, 2);
        m_buttonHLayout->addWidget(m_shutDownBtn, shutDownRow, shutDownColumn, 1, 2);
        m_buttonHLayout->setAlignment(Qt::AlignHCenter);
    }
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
void doLockscreen()
{
    QDBusInterface *interface = new QDBusInterface("org.ukui.ScreenSaver",
                                                   "/",
                                                   "org.ukui.ScreenSaver"
                                                   );
    QDBusMessage msg = interface->call("Lock");
    exit(0);
}

// handle mouse-clicked event
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_Is_UKUI_3_1) {
        for (auto iter = map.begin(); iter != map.end(); iter++) {
            if (iter.value()->objectName() == obj->objectName()) {
                changePoint(iter.value(), event, iter.key());
                if (event->type() == QEvent::MouseButtonRelease) {
                    doEvent(iter.value()->objectName(), iter.key());
                    return QWidget::eventFilter(obj, event);
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MainWindow::changePoint(QWidget *widget, QEvent *event, int i)
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
    qDebug() << "doevent... i:" << i << test;
    defaultnum = i;
    if (inhibitShutdown && (i ==5 || i ==6)) {
        //显示禁止shutdown的提示信息
        showInhibitWarning();
    } else if (inhibitSleep && (i == 2 || i == 1)) {
        //显示禁止sleep的提示信息
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
            this->hide();
            if (i == 3) {
                doLockscreen();
            } else {
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
        if (!m_suspendBtn->geometry().contains(m_toolWidget->mapFromGlobal(event->pos()))
            && !m_hibernateBtn->geometry().contains(m_toolWidget->mapFromGlobal(event->pos()))
            && !m_lockScreenBtn->geometry().contains(m_toolWidget->mapFromGlobal(event->pos()))
            && !m_switchUserBtn->geometry().contains(m_toolWidget->mapFromGlobal(event->pos()))
            && !m_logoutBtn->geometry().contains(m_toolWidget->mapFromGlobal(event->pos()))
            && !m_rebootBtn->geometry().contains(m_toolWidget->mapFromGlobal(event->pos()))
            && !m_shutDownBtn->geometry().contains(m_toolWidget->mapFromGlobal(event->pos()))) {
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

        changeBtnState(button, true);
    }
    else if (key == "Return") {   // space,KP_Enter
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

    drawWarningWindow(mainScreen);
}

void MainWindow::drawWarningWindow(QRect &rect)
{
    int xx = rect.x();
    int yy = rect.y();//用于设置相对位置

    bool isEnoughBig = m_screen.height() - 266 - 467 > 0 ? true : false;
    //area作为该界面所有组件的父指针，方便排版
    QWidget *area = new QWidget(this);
    area->setObjectName(QString::fromUtf8("area"));
    area->setGeometry(0, 0, 714, isEnoughBig ? 467 : 415);

    //顶部提醒信息
    QLabel *tips = new QLabel(area);
    tips->setObjectName(QString::fromUtf8("tips"));
    tips->setGeometry(0, 0, 714, 27);
    QString str;
    //defaultnum会在doevent中初始化为按钮的编号，结合defaultnum判断可以保证sleep和shutdown都被阻止时能够正确显示信息
    if (inhibitSleep) {
        if(defaultnum == 1)
            str = QObject::tr("The following program blocking system into hibernate");
        else if(defaultnum == 2)
            str = QObject::tr("The following program blocking system into sleep");
    }
    if (inhibitShutdown) {
        if(defaultnum ==5)
            str = QObject::tr("The following program blocking system reboot");
        else if(defaultnum ==6)
            str = QObject::tr("The following program blocking system shutdown");
    }
    tips->setText(str);
    tips->setAlignment(Qt::AlignCenter);
    tips->setStyleSheet(QString::fromUtf8("color:white;font:14pt;"));

    //数据模型
    QStandardItemModel *model = new QStandardItemModel(this);
    if (inhibitSleep && (defaultnum == 2 || defaultnum == 1)) {
        for (int i = 0; i < sleepInhibitors.length(); ++i) {
//            QIcon icon("/usr/share/icons/ukui-icon-theme-default/32x32/mimetypes/application-x-desktop.png");//默认图标
            QIcon icon;
            QString AppName;
            QString iconName;
            QString appName = sleepInhibitors.at(i);
            QMap<QString, QString> nameAndIcon = findNameAndIcon(appName);
            if (nameAndIcon.size() != 0) {
                AppName = nameAndIcon.begin().key();
                iconName = nameAndIcon.begin().value();
            }

            if (!iconName.isEmpty() && QIcon::hasThemeIcon(iconName)) {
                icon = QIcon::fromTheme(iconName);
            } else if (QIcon::hasThemeIcon("application-x-desktop")) {
                //无法从desktop文件获取指定图标时使用默认的图标
                icon = QIcon::fromTheme("application-x-desktop");
            }

            if (!AppName.isEmpty()) {
                //查找到的应用名存在则用应用名
                AppName.swap(appName);
            }

            model->appendRow(new QStandardItem(icon, appName));
        }
    } else if (inhibitShutdown && (defaultnum ==5 || defaultnum ==6)) {
        for (int i = 0; i < shutdownInhibitors.length(); ++i) {
            QIcon icon;
            QString AppName;
            QString iconName;
            QString appName = shutdownInhibitors.at(i);
            QMap<QString, QString> nameAndIcon = findNameAndIcon(appName);
            if (nameAndIcon.size() != 0) {
                AppName = nameAndIcon.begin().key();
                iconName = nameAndIcon.begin().value();
            }


            if (!iconName.isEmpty() && QIcon::hasThemeIcon(iconName)) {
                icon = QIcon::fromTheme(iconName);
            } else if (QIcon::hasThemeIcon("application-x-desktop")) {
                icon = QIcon::fromTheme("application-x-desktop");
            }

            if (!AppName.isEmpty()) {
                AppName.swap(appName);
            }

            model->appendRow(new QStandardItem(icon, appName));
        }
    }

    //列表视图
    QListView *applist = new QListView(area);
    applist->setObjectName(QString::fromUtf8("applist"));
    applist->setGeometry(97, isEnoughBig ? 51 : 32, 520, 320);
    applist->verticalScrollMode();
    applist->setStyleSheet("font:10pt;color:white");
    applist->setEditTriggers(QAbstractItemView::NoEditTriggers);
    applist->setIconSize(QSize(32,32));
    applist->setModel(model);

    //继续操作按钮
    QPushButton *confirmBtn = new QPushButton(area);
    confirmBtn->setObjectName(QString::fromUtf8("confirmBtn"));

    if (inhibitSleep) {
        if(defaultnum == 1)
            confirmBtn->setText(QObject::tr("Still Hibernate"));
        else if(defaultnum == 2)
            confirmBtn->setText(QObject::tr("Still Sleep"));
    }
    if (inhibitShutdown) {
        if(defaultnum ==5)
            confirmBtn->setText(QObject::tr("Still Reboot"));
        else if(defaultnum ==6)
            confirmBtn->setText(QObject::tr("Still Shutdown"));
    }

    confirmBtn->setGeometry(227, isEnoughBig ? 419 : 362, 120, 48);
    confirmBtn->setStyleSheet("font:12pt;color:white");
    connect(confirmBtn, &QPushButton::clicked, [this]() {
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
    QPushButton *cancelBtn = new QPushButton(area);
    cancelBtn->setObjectName(QString::fromUtf8("cancelBtn"));
    cancelBtn->setText(QObject::tr("Cancel"));
    cancelBtn->setGeometry(367, isEnoughBig ? 419 : 362, 120, 48);
    cancelBtn->setStyleSheet("font:12pt;color:white");
    connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::exitt);

    //移动整个区域到指定的相对位置
    area->move(xx + (rect.width() - 714) / 2, (yy + 266 * m_screen.height()/1440) + (isEnoughBig ? 0 : 10));
    area->show();
}

QMap<QString, QString> MainWindow::findNameAndIcon(QString &basename)
{
    QMap<QString, QString> nameAndIcon;
    QString icon;
    QString Name;
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
                Name = getAppLocalName(fi.absoluteFilePath());//根据系统的本地语言设置获取对应的名称
                nameAndIcon[Name] = icon;
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

    int xx = m_screen.x();
    int yy = m_screen.y();   //取得当前鼠标所在屏幕的最左，上坐标
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

