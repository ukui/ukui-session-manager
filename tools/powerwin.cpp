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
#include <QPushButton>
#include <QPainter>
#include <QKeyEvent>
#include <QApplication>

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

    setWindowFlags(Qt::FramelessWindowHint);//设置窗体无边框，不可拖动拖拽拉伸
    setAttribute(Qt::WA_TranslucentBackground, true);//设定该窗口透明显示
    setGeometry(this->rect());

    //重启和关机一定存在，根据是否能挂起和休眠确定窗口宽度
    int cnt = 0;
    if(m_power->canAction(UkuiPower::PowerHibernate))
        cnt++;
    if(m_power->canAction(UkuiPower::PowerSuspend))
        cnt++;

    setDialogSize((643 + 188 * cnt + 188), (280 + 85));

    QVBoxLayout *vbox = new QVBoxLayout(m_dialog);
    vbox->setContentsMargins(20, 10, 20, 2);//设置左侧、顶部、右侧和底部边距
    vbox->setSpacing(10);

    m_hbox = new QHBoxLayout();
    m_hbox->setSpacing(20);//表示各个控件之间的上下间距

    QWidget *center_widget = m_dialog;
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

    vbox->addLayout(m_hbox);
    vbox->addStretch();

    //取消按钮
    cancel = new QLabel(center_widget);
    cancel->setObjectName("cancel");
    QString text = tr("取消");
    cancel->setText(text);
    cancel->setFont(QFont("ubuntu", 18));
    cancel->setGeometry((dialogWidth-142)/2,(dialogHeight-58),142,38);
    cancel->setAlignment(Qt::AlignCenter);
    cancel->installEventFilter(this);
}

void PowerWin::addButton(QString name, QLabel *button, QLabel *label)
{
    QVBoxLayout *vboxSuspend = new QVBoxLayout();
    button->setFixedSize(168, 168);
    button->setObjectName(name);
    button->installEventFilter(this);

    label->setAlignment(Qt::AlignCenter);//居中
    label->setText(name);
    label->setFont(QFont("ubuntu", 16));
    label->setFixedSize(168, 30);

    vboxSuspend->addWidget(button);
    vboxSuspend->addWidget(label);

    m_hbox->addLayout(vboxSuspend);
}

/*响应事件*/
void PowerWin::doevent(QEvent *event, QString test2,int i){
    if (event->type() == QEvent::MouseButtonRelease){
        qDebug() << test2;
        try{
            close();
            m_power->doAction(UkuiPower::Action(i));
        }catch(QException &e) {
            qWarning() << e.what();
        }
    }
}

//鼠标事件
bool PowerWin::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == "suspend") {
        doevent(event,"suspend",5);
    } else if (obj == m_hibernate) {
        doevent(event,"hibernate",2);
    } else if (obj == m_reboot) {
        doevent(event,"reboot",3);
    } else if(obj == m_shutdown) {
        doevent(event,"shutdown",4);
    } else if (obj == m_logout) {
        doevent(event,"logout",0);
    } else if (obj == m_switchuser) {
        doevent(event,"switchuser",1);
    } else if (obj == cancel) {
        if (event->type() == QEvent::MouseButtonPress){
            close();
        }
    }
    return QWidget::eventFilter(obj, event);
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
    painter.setBrush(QColor("#0a4989"));
    painter.drawRect(rect());

    QWidget::paintEvent(e);
}

//调整窗口布局
void PowerWin::resizeEvent(QResizeEvent */*event*/)
{
    m_dialog->setGeometry((width() - dialogWidth) / 2,
                          (height() - dialogHeight) / 2,
                          dialogWidth,dialogHeight);
}

void PowerWin::ResizeEvent(QRect screen,QWidget *widget){
    int xx = screen.x();
    int yy = screen.y();//取得当前鼠标所在屏幕的最左，上坐标
    widget->move(xx+(screen.width() - widget->width())/2,
                 yy+(screen.height()- widget->height())/2);
}
