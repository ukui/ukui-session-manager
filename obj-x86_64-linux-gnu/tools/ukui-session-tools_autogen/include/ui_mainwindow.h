/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *switchuser;
    QLabel *switchuser_icon;
    QLabel *switchuser_lable;
    QWidget *hibernate;
    QLabel *hibernate_icon;
    QLabel *hibernate_label;
    QWidget *widget;
    QLabel *time_lable;
    QLabel *date_label;
    QWidget *suspend;
    QLabel *suspend_icon;
    QLabel *suspend_label;
    QWidget *logout;
    QLabel *logout_icon;
    QLabel *logout_label;
    QWidget *reboot;
    QLabel *reboot_icon;
    QLabel *reboot_label;
    QWidget *shutdown;
    QLabel *shutdown_icon;
    QLabel *shutdown_label;
    QWidget *lockscreen;
    QLabel *lockscreen_icon;
    QLabel *lockscreen_label;
    QWidget *message;
    QLabel *message_label1;
    QLabel *message_label2;
    QWidget *judgeWidget;
    QPushButton *cancelButton;
    QPushButton *confirmButton;
    QLabel *label;

    void setupUi(QWidget *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1086, 900);
        MainWindow->setAutoFillBackground(false);
        switchuser = new QWidget(MainWindow);
        switchuser->setObjectName(QString::fromUtf8("switchuser"));
        switchuser->setGeometry(QRect(0, 300, 140, 140));
        switchuser_icon = new QLabel(switchuser);
        switchuser_icon->setObjectName(QString::fromUtf8("switchuser_icon"));
        switchuser_icon->setGeometry(QRect(46, 30, 48, 48));
        switchuser_lable = new QLabel(switchuser);
        switchuser_lable->setObjectName(QString::fromUtf8("switchuser_lable"));
        switchuser_lable->setGeometry(QRect(0, 100, 140, 40));
        QFont font;
        font.setFamily(QString::fromUtf8("Noto Sans CJK SC"));
        font.setPointSize(12);
        font.setBold(false);
        font.setItalic(false);
        font.setWeight(50);
        switchuser_lable->setFont(font);
        switchuser_lable->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        switchuser_lable->setAlignment(Qt::AlignCenter);
        hibernate = new QWidget(MainWindow);
        hibernate->setObjectName(QString::fromUtf8("hibernate"));
        hibernate->setGeometry(QRect(158, 300, 140, 140));
        hibernate_icon = new QLabel(hibernate);
        hibernate_icon->setObjectName(QString::fromUtf8("hibernate_icon"));
        hibernate_icon->setGeometry(QRect(46, 30, 48, 48));
        hibernate_label = new QLabel(hibernate);
        hibernate_label->setObjectName(QString::fromUtf8("hibernate_label"));
        hibernate_label->setGeometry(QRect(0, 100, 140, 40));
        hibernate_label->setFont(font);
        hibernate_label->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        hibernate_label->setAlignment(Qt::AlignCenter);
        widget = new QWidget(MainWindow);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(130, 20, 260, 60));
        time_lable = new QLabel(widget);
        time_lable->setObjectName(QString::fromUtf8("time_lable"));
        time_lable->setGeometry(QRect(0, 0, 260, 40));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Noto Sans CJK SC"));
        font1.setPointSize(28);
        font1.setBold(false);
        font1.setItalic(false);
        font1.setWeight(50);
        time_lable->setFont(font1);
        time_lable->setContextMenuPolicy(Qt::NoContextMenu);
        time_lable->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:28pt;"));
        time_lable->setLineWidth(1);
        time_lable->setAlignment(Qt::AlignCenter);
        date_label = new QLabel(widget);
        date_label->setObjectName(QString::fromUtf8("date_label"));
        date_label->setGeometry(QRect(0, 40, 260, 20));
        date_label->setFont(font);
        date_label->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        date_label->setAlignment(Qt::AlignCenter);
        suspend = new QWidget(MainWindow);
        suspend->setObjectName(QString::fromUtf8("suspend"));
        suspend->setGeometry(QRect(316, 300, 140, 140));
        suspend_icon = new QLabel(suspend);
        suspend_icon->setObjectName(QString::fromUtf8("suspend_icon"));
        suspend_icon->setGeometry(QRect(46, 30, 48, 48));
        suspend_label = new QLabel(suspend);
        suspend_label->setObjectName(QString::fromUtf8("suspend_label"));
        suspend_label->setGeometry(QRect(0, 100, 140, 40));
        suspend_label->setFont(font);
        suspend_label->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        suspend_label->setAlignment(Qt::AlignCenter);
        logout = new QWidget(MainWindow);
        logout->setObjectName(QString::fromUtf8("logout"));
        logout->setGeometry(QRect(632, 300, 140, 140));
        logout_icon = new QLabel(logout);
        logout_icon->setObjectName(QString::fromUtf8("logout_icon"));
        logout_icon->setGeometry(QRect(46, 30, 48, 48));
        logout_label = new QLabel(logout);
        logout_label->setObjectName(QString::fromUtf8("logout_label"));
        logout_label->setGeometry(QRect(0, 100, 140, 40));
        logout_label->setFont(font);
        logout_label->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        logout_label->setAlignment(Qt::AlignCenter);
        reboot = new QWidget(MainWindow);
        reboot->setObjectName(QString::fromUtf8("reboot"));
        reboot->setGeometry(QRect(790, 300, 140, 140));
        reboot_icon = new QLabel(reboot);
        reboot_icon->setObjectName(QString::fromUtf8("reboot_icon"));
        reboot_icon->setGeometry(QRect(46, 30, 48, 48));
        reboot_label = new QLabel(reboot);
        reboot_label->setObjectName(QString::fromUtf8("reboot_label"));
        reboot_label->setGeometry(QRect(0, 100, 140, 40));
        reboot_label->setFont(font);
        reboot_label->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        reboot_label->setAlignment(Qt::AlignCenter);
        shutdown = new QWidget(MainWindow);
        shutdown->setObjectName(QString::fromUtf8("shutdown"));
        shutdown->setGeometry(QRect(948, 300, 140, 140));
        shutdown_icon = new QLabel(shutdown);
        shutdown_icon->setObjectName(QString::fromUtf8("shutdown_icon"));
        shutdown_icon->setGeometry(QRect(46, 30, 48, 48));
        shutdown_label = new QLabel(shutdown);
        shutdown_label->setObjectName(QString::fromUtf8("shutdown_label"));
        shutdown_label->setGeometry(QRect(0, 100, 140, 40));
        shutdown_label->setFont(font);
        shutdown_label->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        shutdown_label->setAlignment(Qt::AlignCenter);
        lockscreen = new QWidget(MainWindow);
        lockscreen->setObjectName(QString::fromUtf8("lockscreen"));
        lockscreen->setGeometry(QRect(474, 300, 140, 140));
        lockscreen_icon = new QLabel(lockscreen);
        lockscreen_icon->setObjectName(QString::fromUtf8("lockscreen_icon"));
        lockscreen_icon->setGeometry(QRect(46, 30, 48, 48));
        lockscreen_label = new QLabel(lockscreen);
        lockscreen_label->setObjectName(QString::fromUtf8("lockscreen_label"));
        lockscreen_label->setGeometry(QRect(0, 100, 140, 40));
        lockscreen_label->setFont(font);
        lockscreen_label->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        lockscreen_label->setAlignment(Qt::AlignCenter);
        message = new QWidget(MainWindow);
        message->setObjectName(QString::fromUtf8("message"));
        message->setGeometry(QRect(190, 614, 700, 80));
        message_label1 = new QLabel(message);
        message_label1->setObjectName(QString::fromUtf8("message_label1"));
        message_label1->setGeometry(QRect(0, 0, 700, 40));
        message_label1->setLayoutDirection(Qt::LeftToRight);
        message_label1->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        message_label1->setAlignment(Qt::AlignCenter);
        message_label2 = new QLabel(message);
        message_label2->setObjectName(QString::fromUtf8("message_label2"));
        message_label2->setGeometry(QRect(0, 40, 700, 40));
        message_label2->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:12pt;"));
        message_label2->setAlignment(Qt::AlignCenter);
        judgeWidget = new QWidget(MainWindow);
        judgeWidget->setObjectName(QString::fromUtf8("judgeWidget"));
        judgeWidget->setGeometry(QRect(110, 170, 850, 140));
        cancelButton = new QPushButton(judgeWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(300, 90, 96, 36));
        confirmButton = new QPushButton(judgeWidget);
        confirmButton->setObjectName(QString::fromUtf8("confirmButton"));
        confirmButton->setGeometry(QRect(450, 90, 96, 36));
        label = new QLabel(judgeWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(0, 10, 841, 51));
        label->setStyleSheet(QString::fromUtf8("color:white;\n"
"font:14pt;"));
        label->setAlignment(Qt::AlignCenter);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QWidget *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Form", nullptr));
        switchuser_icon->setText(QString());
        switchuser_lable->setText(QApplication::translate("MainWindow", "Switch User", nullptr));
        hibernate_icon->setText(QString());
        hibernate_label->setText(QApplication::translate("MainWindow", "Hibernate", nullptr));
        time_lable->setText(QString());
        date_label->setText(QString());
        suspend_icon->setText(QString());
        suspend_label->setText(QApplication::translate("MainWindow", "Suspend", nullptr));
        logout_icon->setText(QString());
        logout_label->setText(QApplication::translate("MainWindow", "Logout", nullptr));
        reboot_icon->setText(QString());
        reboot_label->setText(QApplication::translate("MainWindow", "Reboot", nullptr));
        shutdown_icon->setText(QString());
        shutdown_label->setText(QApplication::translate("MainWindow", "Shut Down", nullptr));
        lockscreen_icon->setText(QString());
        lockscreen_label->setText(QApplication::translate("MainWindow", "Lock Screen", nullptr));
        message_label1->setText(QString());
        message_label2->setText(QString());
        cancelButton->setText(QApplication::translate("MainWindow", "cancel", nullptr));
        confirmButton->setText(QApplication::translate("MainWindow", "confirm", nullptr));
        label->setText(QApplication::translate("MainWindow", "<html><head/><body><p><br/></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
