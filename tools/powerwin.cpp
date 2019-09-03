#include "powerwin.h"
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
#include <QTimer>
#include <QProcess>
#include <QPushButton>
#include <QPainter>
#include <QKeyEvent>

#define BORDER_WIDTH 24
PowerWin::PowerWin(bool hasOpenSessions)
      :QWidget(),
      m_hasOpenSessions(hasOpenSessions),
      m_power(new UkuiPower(this))
{
    initUI();
}

void PowerWin::initUI()
{
    m_dialog = new QWidget(this);
    m_dialog->setObjectName(QStringLiteral("PowerWin"));

    m_centerWidget = new QWidget(m_dialog);
    m_centerWidget->setObjectName(QStringLiteral("PowerWinCenterWidget"));

    m_closeButton = new QPushButton(m_dialog);
    m_closeButton->setObjectName(QStringLiteral("closeButton"));
    connect(m_closeButton, &QPushButton::clicked, this, &PowerWin::close);

    setWindowFlags(Qt::FramelessWindowHint);//设置窗体无边框，不可拖动拖拽拉伸
    setAttribute(Qt::WA_TranslucentBackground, true);//设定该窗口透明显示
    setGeometry(this->rect());

    //重启和关机一定存在，根据是否能挂起和休眠确定窗口宽度
    int cnt = 0;
    if(m_power->canAction(UkuiPower::PowerHibernate))
        cnt++;
    if(m_power->canAction(UkuiPower::PowerSuspend))
        cnt++;

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
        lineNum = qCeil(textWide * 1.0 / m_centerWidget->width()) + 1 + lineNum;
    }
    setDialogSize((643 + 188 * cnt + 188), (280 + 20*lineNum + 85));

    QVBoxLayout *vbox = new QVBoxLayout(m_centerWidget);
    vbox->setContentsMargins(20, 10, 20, 2);//设置左侧、顶部、右侧和底部边距
    vbox->setSpacing(10);

    m_prompt = new QLabel(m_centerWidget);
    m_prompt->adjustSize();
    m_prompt->setText(text);
    m_prompt->setStyleSheet("color:white");
    m_prompt->setWordWrap(true);
    m_prompt->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_prompt->setFont(font);

    vbox->addWidget(m_prompt);

    m_hbox = new QHBoxLayout();
    m_hbox->setSpacing(20);//表示各个控件之间的上下间距

    QWidget *center_widget = m_centerWidget;
    /* 挂起 */
    if (m_power->canAction(UkuiPower::PowerSuspend)) {
        m_suspend = new QLabel(center_widget);
        m_suspendLabel = new QLabel(center_widget);
        addButton("suspend", m_suspend, m_suspendLabel);
    }
    /* 休眠 */
    if(m_power->canAction(UkuiPower::PowerHibernate)) {
        m_hibernate = new QLabel(center_widget);
        m_hibernateLabel = new QLabel(center_widget);
        addButton("hibernate", m_hibernate, m_hibernateLabel);
    }
    /*切换账户*/
    if(true){//m_power->canAction(UkuiPower::PowerSwitchUser)
        m_switchuser = new QLabel(center_widget);
        m_switchuserLabel = new QLabel(center_widget);
        addButton("switchuser",m_switchuser,m_switchuserLabel);
    }

    m_reboot = new QLabel(center_widget);
    m_rebootLabel = new QLabel(center_widget);
    m_shutdown = new QLabel(center_widget);
    m_shutdownLabel = new QLabel(center_widget);
    m_logout = new QLabel(center_widget);
    m_logoutLabel = new QLabel(center_widget);

    addButton("restart", m_reboot, m_rebootLabel);
    addButton("shutdown", m_shutdown, m_shutdownLabel);
    addButton("logout", m_logout, m_logoutLabel);

    //计时器
    progressBar = new QProgressBar(this);
    progressBar->setObjectName("bar");
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
    strTemp = QString::fromLocal8Bit(p.readAllStandardOutput());
    QString LoginUser = strTemp.trimmed();
    tip = new QLabel(m_centerWidget);
    QString tiptext = ("您目前以\""+LoginUser+"\"身份登录");
    tip->setText(tiptext);
    tip->setStyleSheet("color:white");
    tip->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    tip->setFont(font);

    vbox->addLayout(m_hbox);
    vbox->addWidget(tip);
    vbox->addWidget(progressBar);
    vbox->addStretch();
}

void PowerWin::addButton(QString name, QLabel *button, QLabel *label)
{
    QVBoxLayout *vboxSuspend = new QVBoxLayout();
    button->setFixedSize(168, 168);
    button->setObjectName(name);
    button->installEventFilter(this);

    label->setAlignment(Qt::AlignCenter);//居中
    label->setFixedSize(168, 30);

    vboxSuspend->addWidget(button);
    vboxSuspend->addWidget(label);

    m_hbox->addLayout(vboxSuspend);
}

//计时条更新
void PowerWin::update()
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

/*响应事件*/
void PowerWin::doevent(QEvent *event, QLabel *label, QString test1, QString test2,int i){
    if (event->type() == QEvent::Enter) {
        label->setText(test1);
    }else if (event->type() == QEvent::Leave) {
        label->setText(tr(""));
    }else if (event->type() == QEvent::MouseButtonRelease){
        qDebug() << test2;
        try{
            m_power->doAction(UkuiPower::Action(i));
            close();
        }catch(QException &e) {
            qWarning() << e.what();
        }
    }
}

//鼠标事件
bool PowerWin::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == "suspend") {
        doevent(event,m_suspendLabel,"挂起(S)","suspend",5);
    } else if (obj == m_hibernate) {
        doevent(event,m_hibernateLabel,"休眠(H)","hibernate",2);
    } else if (obj == m_reboot) {
        doevent(event,m_rebootLabel,"重启(R)","reboot",3);
    } else if(obj == m_shutdown) {
        doevent(event,m_shutdownLabel,"关机(P)","shutdown",4);
    } else if (obj == m_logout) {
        doevent(event,m_logoutLabel,"注销(L)","logout",0);
    } else if (obj == m_switchuser) {
        doevent(event,m_switchuserLabel,"切换用户(U)","switchuser",1);
    }
    return QWidget::eventFilter(obj, event);
}

/*点击会话框外的地方退出*/
void PowerWin::mousePressEvent(QMouseEvent *event)
{
    if(!m_dialog->geometry().contains(event->pos()))
        close();
}

void PowerWin::setDialogSize(int w, int h)
{
    dialogWidth = w;
    dialogHeight = h;

    resizeEvent(nullptr);
}

void PowerWin::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(0,0,0,50));
    painter.drawRect(rect());

    QWidget::paintEvent(e);
}

//调整窗口布局
void PowerWin::resizeEvent(QResizeEvent */*event*/)
{
    m_dialog->setGeometry((width() - dialogWidth) / 2,
                          (height() - dialogHeight) / 2,
                          dialogWidth,dialogHeight);
    m_centerWidget->setGeometry(BORDER_WIDTH, BORDER_WIDTH,
                                m_dialog->width() - BORDER_WIDTH * 2,
                                m_dialog->height() - BORDER_WIDTH * 2);
    m_closeButton->setGeometry(m_dialog->width() - BORDER_WIDTH, 0,
                               BORDER_WIDTH, BORDER_WIDTH);
}
