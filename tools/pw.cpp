#include "pw.h"
#include <QPushButton>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
//#include <QPixmap>

#define BORDER_WIDTH 24

PW::PW() : QWidget()
{
    //pixmap.load("resource/background-ubuntu.jpg");
    initUI();
}

void PW::initUI()
{
    setObjectName("PWBackground");
    setAttribute(Qt::WA_TranslucentBackground, true);
    setGeometry(this->rect());

    m_dialog = new QWidget(this);
    m_dialog->setObjectName(QStringLiteral("pw"));

    m_centerWidget = new QWidget(m_dialog);
    m_centerWidget->setObjectName(QStringLiteral("pwCenterWidget"));

    m_closeButton = new QPushButton(m_dialog);
    m_closeButton->setObjectName(QStringLiteral("closeButton"));
    connect(m_closeButton, &QPushButton::clicked, this, &PW::close);
}

void PW::setDialogSize(int w, int h)
{
    dialogWidth = w;
    dialogHeight = h;

    resizeEvent(nullptr);
}

QWidget* PW::centerWidget()
{
    return m_centerWidget;
}

QWidget* PW::dialog()
{
    return m_dialog;
}

void PW::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    //painter.drawPixmap(rect(),pixmap);
    painter.setBrush(QColor(0,0,0,50));//0,0,0,0
    painter.drawRect(rect());

    QWidget::paintEvent(e);
}

//调整窗口布局
void PW::resizeEvent(QResizeEvent */*event*/)
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

void PW::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        close();
    }
}
