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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "powerprovider.h"
#include <QTimer>
#include "QProcess"
#include <QPixmap>
#include <QHash>
#include <QGSettings/QGSettings>
#include <QAbstractNativeEventFilter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include "mypushbutton.h"
class XEventMonitor;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    MainWindow(bool a , bool b ,QWidget *parent = nullptr);
    ~MainWindow();

    void doEvent(QString test2,int i);
    void changePoint(QWidget *widget ,QEvent *event ,int i);
    void judgeboxShow();
    void keyPressEmulate();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    //void closeEvent(QCloseEvent *event);

    void initialBtn();
    void initialJudgeWidget();
    void initialMessageWidget();
    void initialDateTimeWidget();
    void initialBtnCfg();
    void setLayoutWidgetVisible(QLayout* layout, bool show);
    void changeBtnState(QString btnName, bool isEnterKey = false);

private:
    QString getAppLocalName(QString desktopfp);//获取应用名
    void ResizeEvent();
    void showInhibitWarning();//当有inhibitor存在时显示提醒界面
    void drawWarningWindow(QRect &rect);//画出提醒界面
    QMap<QString, QString> findNameAndIcon(QString &basename);//根据inhibitor的名称获取对应desktop文件中的中文名和icon路径

    void calculateBtnSpan(int allNum, MyPushButton*, int& colum, int& row);
    void calculateKeyBtn(const QString &key);
    bool judgeBtnIsEnable(int index);

Q_SIGNALS:
    void signalTostart();
    void confirmButtonclicked();

private Q_SLOTS:
    bool exitt();
    void onGlobalKeyPress(const QString &key);
    void onGlobalkeyRelease(const QString &key);
    void screenCountChanged();
    void mouseReleaseSlots(QEvent *event, QString objName);

protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *event);

public:
    //    QRect m_screen;
    QTimer *timer;
    int defaultnum = 0;

private:
    QString user;
    QPixmap pix;
    QGSettings *gs;
    QWidget *lastWidget;
    QHash<int, MyPushButton*> map;
    QStringList shutdownInhibitors;//阻止shutdown的inhibitors
    QStringList shutdownInhibitorsReason;//阻止shutdown的inhibitor对应的原因
    QStringList sleepInhibitors;//阻止sleep的inhibitors
    QStringList sleepInhibitorsReason;//阻止sleep的inhibitor对应的原因
    Ui::MainWindow *ui;
    UkuiPower *m_power;
    XEventMonitor *xEventMonitor;
    int tableNum;
    bool flag = false;
    bool isSwitchuserHide = true;
    bool isHibernateHide = true;
    bool lockfile = false;
    bool lockuser = false;
    bool click_blank_space_need_to_exit = true;
    bool close_system_needed_to_confirm = false;
    bool inhibitSleep = false;
    bool inhibitShutdown = false;


    QHash<MyPushButton*, bool> m_btnHideMap;

    bool m_Is_UKUI_3_1 = false;
    QString m_btnImagesPath = "/usr/share/ukui/ukui-session-manager/images";
    MyPushButton *m_switchUserBtn = nullptr;
    MyPushButton *m_hibernateBtn = nullptr;
    MyPushButton *m_suspendBtn = nullptr;
    MyPushButton *m_lockScreenBtn = nullptr;
    MyPushButton *m_logoutBtn = nullptr;
    MyPushButton *m_rebootBtn = nullptr;
    MyPushButton *m_shutDownBtn = nullptr;

    QLabel *m_dateLabel = nullptr;
    QLabel *m_timeLabel = nullptr;

    QLabel *m_judgeLabel = nullptr;
    QPushButton *m_cancelBtn = nullptr;
    QPushButton *m_confirmBtn = nullptr;

    QLabel *m_messageLabel1 = nullptr;
    QLabel *m_messageLabel2 = nullptr;

    QWidget *m_toolWidget = nullptr;
    QWidget *m_systemMonitorBtn = nullptr;

    QGridLayout *m_buttonHLayout = nullptr;

    QVBoxLayout *m_dateTimeLayout = nullptr;
    QVBoxLayout *m_judgeWidgetVLayout = nullptr;
    QHBoxLayout *m_judgeBtnHLayout = nullptr;
    QVBoxLayout *m_messageVLayout = nullptr;
    QVBoxLayout *m_vBoxLayout = nullptr;
    QSettings *m_btnCfgSetting = nullptr;
    QRect m_screen;

};
#endif // MAINWINDOW_H
