#ifndef POWERWIN_H
#define POWERWIN_H

#include <QWidget>
#include <QProgressBar>
#include <QHBoxLayout>

#include "powerprovider.h"

class QPushButton;
class QLabel;
class PowerWin :public QWidget
{
    Q_OBJECT
public:
    PowerWin(bool hasOpenSessions = false);
    void initUI();
    int temp;

protected:
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *event);

private:
    int dialogWidth;
    int dialogHeight;
    QWidget         *m_dialog;
    QWidget         *m_centerWidget;
    QPushButton     *m_closeButton;

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *event);

public:
    void setDialogSize(int w, int h);
    QPixmap pixmap;

private:
    void addButton(QString name, QLabel *button, QLabel *label);
    void doevent(QEvent *, QLabel *label, QString test1, QString test2, int i);

private slots:
    void update();

private:
    bool    m_hasOpenSessions;
    UkuiPower *m_power;

    QHBoxLayout *m_hbox;

    QLabel *m_prompt;
    QLabel *m_suspend;
    QLabel *m_hibernate;
    QLabel *m_reboot;
    QLabel *m_shutdown;
    QLabel *m_logout;
    QLabel *m_suspendLabel;
    QLabel *m_hibernateLabel;
    QLabel *m_rebootLabel;
    QLabel *m_shutdownLabel;
    QLabel *m_logoutLabel;
    QLabel *m_switchuser;
    QLabel *m_switchuserLabel;

    QTimer *qtimer;
    QProgressBar *progressBar;
    QLabel *tip;
};

#endif // POWERWIN_H
