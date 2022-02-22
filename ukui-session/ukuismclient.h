/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>
Copyright 2021 KylinSoft Co., Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

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
