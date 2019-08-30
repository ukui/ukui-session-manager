#ifndef POWERWINDOW_H
#define POWERWINDOW_H

#include <QWidget>
#include <QProgressBar>
#include <QHBoxLayout>
//#include <QLightDM/Power>

#include "powerprovider.h"
#include "pw.h"


class QLabel;
class PowerWindow : public PW
{
    Q_OBJECT
public:
    PowerWindow(bool hasOpenSessions = false);
    void initUI();
    int temp;

protected:
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *event);

private:
    void addButton(QString name, QLabel *button, QLabel *label);

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

    QTimer *qtimer;
    QProgressBar *progressBar;
    QLabel *tip;
};

#endif // POWERWINDOW_H
