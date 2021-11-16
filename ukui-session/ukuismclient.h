#ifndef UKUISMCLIENT_H
#define UKUISMCLIENT_H

extern "C" {
#include <X11/SM/SMlib.h>
}
#include <fixx11h.h>
#include <QString>
#include <QList>

class UKUISMClient
{
public:
    explicit UKUISMClient(SmsConn conn);
    ~UKUISMClient();

    void resetState();
    void registerClient(const char *previousId = nullptr);
    SmsConn connection() const { return m_smsConn; }

    QString program() const;
    QStringList restartCommand() const;
    QStringList discardCommand() const;
    int restartStyleHint() const;
    SmProp* property( const char* name ) const;
    const char* clientId() const { return m_id ? m_id : ""; }
    QString userId() const;


public:
    QList<SmProp*> m_properties;
    bool m_saveYourselfDone;
    bool m_pendingInteraction;
    bool m_waitForPhase2;
    bool m_wasPhase2;

private:
    const char *m_id;
    SmsConn m_smsConn;
};

#endif // UKUISMCLIENT_H
