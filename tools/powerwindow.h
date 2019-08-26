#ifndef POWERWINDOW_H
#define POWERWINDOW_H

#include <QWidget>
//#include <QLightDM/Power>
#include "powerprovider.h"
#include "pw.h"
#include <QProgressBar>

class QLabel;
class PowerWindow : public PW
{
    Q_OBJECT
public:
    PowerWindow(bool hasOpenSessions = false);
    void initUI();
    int temp;

private slots:
    void update();

protected:
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *event);

private:
    bool    m_hasOpenSessions;
    UkuiPower *m_power;

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

    QTimer *qtimer;
    QProgressBar *progressBar;
    QLabel *tip;
};

#endif // POWERWINDOW_H
