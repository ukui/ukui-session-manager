#include "myiconlabel.h"
#include <QPushButton>
#include <QPixmap>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QPoint>
#include <QMouseEvent>

MyIconLabel::MyIconLabel(int labelWidth, int iconWidth, QString path, bool showBackColor, QWidget *parent)
    :m_showBackColor(showBackColor)
{
    this->setFixedSize(labelWidth, labelWidth);
    this->setObjectName("iconlabel");
    m_btnIcon = new PushButton(this);
    m_btnIcon->setIcon(QIcon(path));
    m_btnIcon->setIconSize(QSize(iconWidth, iconWidth));
    m_btnIcon->setObjectName("btn");
    m_btnIcon->setStyleSheet("QPushButton#btn{background-color: transparent;border:none;} \
                             QPushButton:checked  { background-color: transparent;border:none;}\
                             QPushButton:hover  { background-color: transparent;border:none;}");
    m_btnIcon->setGeometry(QRect((width() - iconWidth)/2, (width() - iconWidth)/2, iconWidth, iconWidth));
    m_btnIcon->setCheckable(false);
    m_btnIcon->setChecked(false);
    m_btnIcon->setGeometry(QRect((width() - iconWidth)/2, (width() - iconWidth)/2, iconWidth, iconWidth));
    m_btnIcon->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    if (m_showBackColor) {
        //mask不要与控件一样大  锯齿明显  稍微大一点
        this->setMask(QRegion(this->x() - 1, this->y() - 1, this->width() + 2, this->height() + 2,QRegion::Ellipse));
        this->setStyleSheet("QLabel#"+ this->objectName() + "{background-color: rgb(255,255,255,40);border-radius:" + QString::number(this->width()/2) + "px;}");
    } else {
        this->setStyleSheet("QLabel#"+ this->objectName() + "{background-color: transparent;}");
    }

    //this->setPixmap(m_pixMap);
    this->setAlignment(Qt::AlignCenter);

   //setMouseTracking(true);
}

MyIconLabel::~MyIconLabel()
{
}

/*
void MyIconLabel::paintEvent(QPaintEvent *event)
{
    return;
    QPainter painter(this);
    QColor color;
    if(m_isNormal)
        color = QColor(255,255,255,40);
    else if(m_isHover)
        color = QColor(255,255,255,80);
    else if(m_isClicked)
        color = QColor(255,255,255,100);
    painter.setPen(QPen(color,0));
    painter.setBrush(color);
    painter.drawEllipse(QRect(0,0,width(),height()));
    painter.drawPixmap(QRect((width() - m_btnIcon.width())/2, (height() - m_btnIcon.height())/2, m_btnIcon.width(), m_btnIcon.height()), m_btnIcon);
}
*/
void MyIconLabel::enterEvent(QEvent *event)
{
    if (!m_showBackColor) return;
    emit mouseEventSignals(event);

    QString str = "QLabel{background-color: rgb(255,255,255,80);border-radius: " + QString::number(this->width()/2) + "px;}";

    this->setStyleSheet(str);
    this->setAttribute(Qt::WA_StyledBackground);
}

void MyIconLabel::leaveEvent(QEvent *event)
{
    return;
    if (!m_showBackColor) return;
    QString str = "QLabel{background-color: rgb(255,255,255,40);border-radius: " + QString::number(this->width()/2) + "px;}";
    this->setStyleSheet(str);
    this->setAttribute(Qt::WA_StyledBackground);
}

bool MyIconLabel::event(QEvent *event)
{
//    qDebug() << m_showBackColor << "event..." << event->type();
    if (!m_showBackColor) {
        return QWidget::event(event);
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QString str = "QLabel{background-color: rgb(255,255,255,100);border-radius: " + QString::number(this->width()/2) + "px;}";
        this->setStyleSheet(str);
        this->setAttribute(Qt::WA_StyledBackground);
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QString str = "QLabel{background-color: rgb(255,255,255,80);border-radius: " + QString::number(this->width()/2) + "px;}";
        this->setStyleSheet(str);
        this->setAttribute(Qt::WA_StyledBackground);
        emit mouseEventSignals(event);
    } else {
        emit mouseEventSignals(event);
    }
    return QWidget::event(event);
}

/*
void MyIconLabel::mouseMoveEvent(QMouseEvent *event)
{
    return;
//    m_point = event->pos();
//    qDebug() << "MyIconLabel mouseMoveEvent..." << m_point;

//    if(!m_showBackColor)
//        return;
//    QRegion region(this->x(), this->y(), this->width(), this->height(),QRegion::Ellipse);
//    qDebug() << "MyIconLabel mouseMoveEvent..." << m_point << region;

//    QString str;

//    if(!region.contains(m_point))
//        str = "QLabel{background-color: rgb(255,255,255,40);border-radius: " + QString::number(this->width()/2) + "px;}";
//    else
//        str = "QLabel{background-color: rgb(255,255,255,80);border-radius: " + QString::number(this->width()/2) + "px;}";

//    this->setStyleSheet(str);
//    this->setAttribute(Qt::WA_StyledBackground);

}
*/

bool MyIconLabel::containsPoint(QPoint p)
{
    QPainterPath path;
    path.addEllipse(QRect(0, 0, width(), height()));
    //qDebug() << "containsPoint..." << width() << height() << p << QRect(0,0,width(),height());
    return path.contains(p);
}

/*
void MyIconLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(0,0,0),2));
    painter.setBrush(QColor(10,10,10));
    QPainterPath path;
    QRect rect(10,50,100,48);
    const qreal radius = 24;
    path.moveTo(rect.topRight() - QPointF(radius, 0));
    path.lineTo(rect.topLeft() + QPointF(radius, 0));
    path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
    path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
    path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
    path.lineTo(rect.bottomRight() - QPointF(radius, 0));
    path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
    path.lineTo(rect.topRight() + QPointF(0, radius));
    path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));
    painter.drawPath(path);
    qDebug() << "1111" << path.contains(QPoint(11,51));
}


void MyIconLabel::mousePressEvent(QMouseEvent* event)
{
    qDebug() << "mousePressEvent..." << event->pos();
}
*/
