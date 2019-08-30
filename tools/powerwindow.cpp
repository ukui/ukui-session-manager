#include "powerwindow.h"
#include "powerprovider.h"
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QException>
#include <QFontMetrics>
#include <qmath.h>
#include <QProgressBar>
#include <QPixmap>
#include <QTimer>
#include <QProcess>

PowerWindow::PowerWindow(bool hasOpenSessions)
    : PW(),
      m_hasOpenSessions(hasOpenSessions),
      m_power(new UkuiPower(this))
{
    initUI();
}

void PowerWindow::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint);//设置窗体无边框，不可拖动拖拽拉伸
    setAttribute(Qt::WA_TranslucentBackground, true);//设定该窗口透明显示
    setGeometry(this->rect());

    //重启和关机一定存在，根据是否能挂起和休眠确定窗口宽度
    int cnt = 0;
//    if(m_power->canAction(UkuiPower::PowerHibernate))
//        cnt++;
//    if(m_power->canAction(UkuiPower::PowerSuspend))
//        cnt++;

    //根据提示内容的长度确定窗口的高度
    QFont font("ubuntu", 12);
    QString text = tr("Goodbye. Would you like to…");
    int lineNum = 1;
    if(m_hasOpenSessions) {
        QString text2 = tr("Other users are currently logged in to this computer, "
                   "shutting down now will also close these other sessions.");
        text = QString("%1\n\n%2").arg(text2).arg(text);
        QFontMetrics fm(font);
        int textWide = fm.width(text2);
        lineNum = qCeil(textWide * 1.0 / centerWidget()->width()) + 1 + lineNum;
    }
    setDialogSize((643 + 188 * cnt), (280 + 20*lineNum + 60));

    QVBoxLayout *vbox = new QVBoxLayout(centerWidget());
    vbox->setContentsMargins(20, 10, 20, 2);//设置左侧、顶部、右侧和底部边距
    vbox->setSpacing(10);

    m_prompt = new QLabel(centerWidget());
    m_prompt->adjustSize();
    m_prompt->setText(text);
    m_prompt->setStyleSheet("color:white");
    m_prompt->setWordWrap(true);
    m_prompt->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_prompt->setFont(font);

    vbox->addWidget(m_prompt);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(20);//表示各个控件之间的上下间距

    /* 挂起 */
    if(false){//m_power->canAction(UkuiPower::PowerSuspend)
        QVBoxLayout *vboxSuspend = new QVBoxLayout();
        m_suspend = new QLabel(centerWidget());
        m_suspend->setFixedSize(168, 168);
        m_suspend->setObjectName(QStringLiteral("suspend"));
        m_suspend->setPixmap(QPixmap(":/images/suspend.png"));
        m_suspend->setStyleSheet("border-width: 1px;border-style: solid;border-color: #FFFFFF");
        m_suspend->installEventFilter(this);

        m_suspendLabel = new QLabel(centerWidget());
        m_suspendLabel->setAlignment(Qt::AlignCenter);//居中
        m_suspendLabel->setFixedSize(168, 30);

        vboxSuspend->addWidget(m_suspend);
        vboxSuspend->addWidget(m_suspendLabel);

        hbox->addLayout(vboxSuspend);
    }
    /* 休眠 */
    if(false) {//m_power->canAction(UkuiPower::PowerHibernate)
        QVBoxLayout *vboxHibernate = new QVBoxLayout();
        m_hibernate = new QLabel(centerWidget());
        m_hibernate->setFixedSize(168, 168);
        m_hibernate->setObjectName(QStringLiteral("hibernate"));
        m_hibernate->setPixmap(QPixmap(":/images/hibernate.png"));
        m_hibernate->setStyleSheet("border-width: 1px;border-style: solid;border-color: #FFFFFF;");
        m_hibernate->installEventFilter(this);

        m_hibernateLabel = new QLabel(centerWidget());
        m_hibernateLabel->setAlignment(Qt::AlignCenter);
        m_hibernateLabel->setFixedSize(168, 30);

        vboxHibernate->addWidget(m_hibernate);
        vboxHibernate->addWidget(m_hibernateLabel);

        hbox->addLayout(vboxHibernate);
    }

    /* 重启 */
    QVBoxLayout *vboxStart = new QVBoxLayout();
    m_reboot = new QLabel(centerWidget());
    m_reboot->setFixedSize(168, 168);
    m_reboot->setObjectName(QStringLiteral("restart"));
    m_reboot->setPixmap(QPixmap(":/images/restart.png"));
    m_reboot->setStyleSheet("border-width: 1px;border-style: solid;border-color: #FFFFFF;");
    m_reboot->installEventFilter(this);
    m_rebootLabel = new QLabel(centerWidget());
    m_rebootLabel->setAlignment(Qt::AlignCenter);
    m_rebootLabel->setFixedSize(168, 30);

    vboxStart->addWidget(m_reboot);
    vboxStart->addWidget(m_rebootLabel);

    /* 关机 */
    QVBoxLayout *vboxShutdown = new QVBoxLayout();
    m_shutdown = new QLabel(centerWidget());
    m_shutdown->setFixedSize(168, 168);
    m_shutdown->setObjectName(QStringLiteral("shutdown"));
    m_shutdown->setPixmap(QPixmap(":/images/shutdown.png"));
    m_shutdown->setStyleSheet("border-width: 1px;border-style: solid;border-color: #FFFFFF;");
    m_shutdown->installEventFilter(this);

    m_shutdownLabel = new QLabel(centerWidget());
    m_shutdownLabel->setAlignment(Qt::AlignCenter);
    m_shutdownLabel->setFixedSize(168, 30);

    vboxShutdown->addWidget(m_shutdown);
    vboxShutdown->addWidget(m_shutdownLabel);

    hbox->addLayout(vboxStart);
    hbox->addLayout(vboxShutdown);

    //注销
    QVBoxLayout *vboxLogout = new QVBoxLayout();
    m_logout = new QLabel(centerWidget());
    m_logout->setFixedSize(168, 168);
    m_logout->setObjectName(QStringLiteral("logout"));
    m_logout->setPixmap(QPixmap(":/images/logout.png"));
    m_logout->setStyleSheet("border-width: 1px;border-style: solid;border-color: #FFFFFF;");
    m_logout->installEventFilter(this);

    m_logoutLabel = new QLabel(centerWidget());
    m_logoutLabel->setAlignment(Qt::AlignCenter);
    m_logoutLabel->setFixedSize(168, 30);

    vboxLogout->addWidget(m_logout);
    vboxLogout->addWidget(m_logoutLabel);
    hbox->addLayout(vboxLogout);

    //计时器
    progressBar = new QProgressBar(this);
    progressBar->setStyleSheet("QProgressBar::chunk {\
                                      background-color: #33FFFF;\
                                      //margin: 0.5px;}");
    progressBar->setFormat("系统将在%vs后自动关机");
    progressBar->setRange(0,60);
    progressBar->setValue(60);
    temp = 60;
    qtimer = new QTimer(this);
    qtimer->start(1000);
    connect(qtimer, SIGNAL(timeout()), this, SLOT(update()));

    //提示
    QProcess p(nullptr);
    QString command = "";
    QStringList args;
    QString strTemp = "";
    command = "whoami";
    args.clear();
    p.start(command,args);
    p.waitForFinished();
    strTemp = QString::fromLocal8Bit( p.readAllStandardOutput() );
    QString LoginUser = strTemp.trimmed();
    tip = new QLabel(centerWidget());
    QString tiptext = ("您目前以\""+LoginUser+"\"身份登录");
    tip->setText(tiptext);
    tip->adjustSize();
    tip->setStyleSheet("color:white");
    //tip->setWordWrap(true);
    tip->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    tip->setFont(font);

    vbox->addLayout(hbox);
    vbox->addWidget(tip);
    vbox->addWidget(progressBar);
    vbox->addStretch();
}

//计时条更新
void PowerWindow::update()
{
    temp--;
    progressBar->setValue(temp);

    if(temp == 0)
    {
        progressBar->setValue(60);
        qtimer->stop();
        try{
            qDebug() << "shutdown";
            m_power->doAction(UkuiPower::PowerShutdown);
            close();
        }catch(QException &e) {
            qWarning() << e.what();
        }
    }
}

//鼠标事件
bool PowerWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_suspend) {
        if(event->type() == QEvent::Enter) {
            m_suspendLabel->setText(tr("挂起(S)"));
            m_suspendLabel->setStyleSheet("color:white");
            m_suspend->setPixmap(QPixmap(":/images/suspend_highlight.png"));
        } else if(event->type() == QEvent::Leave) {
            m_suspendLabel->setText("");
            m_suspend->setPixmap(QPixmap(":/images/suspend.png"));
        } else if(event->type() == QEvent::MouseButtonRelease){
            qDebug() << "suspend";
            try{
                m_power->doAction(UkuiPower::PowerSuspend);
                close();
            }catch(QException &e) {
                qWarning() << e.what();
            }
        }
    }else if(obj == m_hibernate) {
        if(event->type() == QEvent::Enter) {
            m_hibernateLabel->setText(tr("休眠(H)"));
            m_hibernateLabel->setStyleSheet("color:white");
            m_hibernate->setPixmap(QPixmap(":/images/hibernate_highlight.png"));
        } else if(event->type() == QEvent::Leave) {
            m_hibernateLabel->setText("");
            m_hibernate->setPixmap(QPixmap(":/images/hibernate.png"));
        } else if(event->type() == QEvent::MouseButtonRelease){
            qDebug() << "hibernate";
            try{
                m_power->doAction(UkuiPower::PowerHibernate);
                close();
            }catch(QException &e) {
                qWarning() << e.what();
            }
        }
    } else if(obj == m_reboot) {
        if(event->type() == QEvent::Enter) {
            m_rebootLabel->setText(tr("重启(R)"));
            m_rebootLabel->setStyleSheet("color:white");
            m_reboot->setPixmap(QPixmap(":/images/restart_highlight.png"));
        }else if(event->type() == QEvent::Leave) {
            m_rebootLabel->setText("");
            m_reboot->setPixmap(QPixmap(":/images/restart.png"));
        }else if(event->type() == QEvent::MouseButtonRelease){
            qDebug() << "reboot";
            try{
                m_power->doAction(UkuiPower::PowerReboot);
                close();
            }catch(QException &e) {
                qWarning() << e.what();
            }
        }
    } else if(obj == m_shutdown) {
        if(event->type() == QEvent::Enter) {
            m_shutdownLabel->setText(tr("关机(P)"));
            m_shutdownLabel->setStyleSheet("color:white");
            m_shutdown->setPixmap(QPixmap(":/images/shutdown_highlight.png"));
        }else if(event->type() == QEvent::Leave) {
            m_shutdownLabel->setText("");
            m_shutdown->setPixmap(QPixmap(":/images/shutdown.png"));
        }else if(event->type() == QEvent::MouseButtonRelease){
            try{
                qDebug() << "shutdown";
                m_power->doAction(UkuiPower::PowerShutdown);
                close();
            }catch(QException &e) {
                qWarning() << e.what();
            }
        }
    } else if (obj == m_logout) {
        if(event->type() == QEvent::Enter) {
            m_logoutLabel->setText(tr("注销(L)"));
            m_logoutLabel->setStyleSheet("color:white");
            m_logout->setPixmap(QPixmap(":/images/logout_highlight.png"));
        }else if(event->type() == QEvent::Leave) {
            m_logoutLabel->setText("");
            m_logout->setPixmap(QPixmap(":/images/logout.png"));
        }else if(event->type() == QEvent::MouseButtonRelease){
            try{
                qDebug() << "logout";
                m_power->doAction(UkuiPower::PowerLogout);
                close();
            }catch(QException &e) {
                qWarning() << e.what();
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void PowerWindow::mousePressEvent(QMouseEvent *event)
{
    if(!dialog()->geometry().contains(event->pos()))
        close();
}
