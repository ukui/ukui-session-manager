#include "toolswindow.h"
#include "ui_toolswindow.h"
#include <QGraphicsBlurEffect>
#include <QPainter>
#include <QDebug>

ToolsWindow::ToolsWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolsWindow),
    mPower(new UkuiPower(this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
//    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

//    setStyleSheet("background-color:rgba(244,244,244,200)");
//    QGraphicsBlurEffect* blur = new QGraphicsBlurEffect();
//    setGraphicsEffect(blur);

//    setWindowOpacity(0.5);

//    QPalette pal = palette();
//    pal.setColor(QPalette::Background, QColor(0x00, 0xff, 0x00, 156));
//    setAutoFillBackground(true);
//    setPalette(pal);
    ui->setupUi(this);
}

ToolsWindow::~ToolsWindow()
{
    delete ui;
}

void ToolsWindow::on_reboot_button_clicked()
{
    if (mPower->canAction(UkuiPower::PowerReboot)) {
        mPower->doAction(UkuiPower::PowerReboot);
    } else {
        qDebug() << "can't reboot";
    }
    close();
}

void ToolsWindow::on_logout_button_clicked()
{
    if (mPower->canAction(UkuiPower::PowerLogout)) {
        mPower->doAction(UkuiPower::PowerLogout);
    } else {
        qDebug() << "can't logout";
    }
    close();
}

void ToolsWindow::on_shutdown_button_clicked()
{
    if (mPower->canAction(UkuiPower::PowerShutdown)) {
        mPower->doAction(UkuiPower::PowerShutdown);
    } else {
        qDebug() << "can't shutdown";
    }
    close();
}
