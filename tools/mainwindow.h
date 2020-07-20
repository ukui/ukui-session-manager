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
//#include <QAbstractNativeEventFilter>

class XEventMonitor;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow//,public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    QRect m_screen;
    ~MainWindow();
    void doevent(QString test2,int i);
    void changePoint(QWidget *widget ,QEvent *event ,int i);
    void closeEvent(QCloseEvent *event);
    //virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

    QTimer *timer;
    int defaultnum = 0;

private:
    Ui::MainWindow *ui;
    UkuiPower *m_power;
    XEventMonitor *xEventMonitor;
    void ResizeEvent();
    bool flag = false;
    QWidget *lastWidget;
    int tableNum;
    QHash<int , QWidget*> map;
    void refreshBlur(QWidget *last , QWidget *now);
    bool isSwitchuserHide = true;
    bool isHibernateHide = true;
    QPixmap pix;

Q_SIGNALS:
    void signalTostart();

private Q_SLOTS:
    void onGlobalKeyPress(const QString &key);

protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *event);
};
#endif // MAINWINDOW_H
