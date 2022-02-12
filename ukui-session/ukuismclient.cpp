/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>
Copyright 2005 Lubos Lunak <l.lunak@kde.org>
Copyright 2021 KylinSoft Co., Ltd.


relatively small extensions by Oswald Buddenhagen <ob6@inf.tu-dresden.de>

some code taken from the dcopserver (part of the KDE libraries), which is
Copyright 1999 Matthias Ettrich <ettrich@kde.org>
Copyright 1999 Preston Brown <pbrown@kde.org>

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

#include "ukuismclient.h"
#include "ukuismserver.h"

#include <unistd.h>
#include <stdlib.h>


extern UKUISMServer*& getGlobalServer();

/*用于生成客户端的唯一ID*/
Q_GLOBAL_STATIC(QString, my_addr)
char * safeSmsGenerateClientID(SmsConn c)
{
    char *ret = nullptr;
    if (!ret) {
       if (my_addr->isEmpty()) {
           char hostname[256];
           if (gethostname(hostname, 255) != 0) {
               my_addr->sprintf("0%.8x", rand());
           } else {
               int addr[4] = {0, 0, 0, 0};
               int pos = 0;
               for (unsigned int i = 0; i < strlen(hostname); ++i, ++pos) {
                   addr[pos % 4] += hostname[i];
               }
               *my_addr = QStringLiteral("0");
               for (int i = 0; i < 4; ++i) {
                   *my_addr += QString::number(addr[i], 16);
               }
           }
       }

       ret = (char*)malloc(1 + my_addr->length() + 13 + 10 + 4 + 1 + 0);
       static int sequence = 0;
       if (ret == nullptr) {
           return nullptr;
       }

       sprintf(ret, "1%s%.13ld%.10d%.4d", my_addr->toLatin1().constData(), (long)time(nullptr), getpid(), sequence);
       sequence = (sequence + 1) % 10000;
    }
    return ret;
}

UKUISMClient::UKUISMClient(SmsConn conn) : m_smsConn(conn), m_id(nullptr)
{
    resetState();
}

UKUISMClient::~UKUISMClient()
{
}

void UKUISMClient::resetState()
{
    m_saveYourselfDone = false;
    m_pendingInteraction = false;
    m_waitForPhase2 = false;
    m_wasPhase2 = false;
}

void UKUISMClient::registerClient(const char *previousId)
{
    m_id = previousId;
    if (!m_id) {
        m_id = safeSmsGenerateClientID(m_smsConn);
    }

    SmsRegisterClientReply(m_smsConn, (char*)m_id);
    //此处向应用发出保存自身信号，应用收到信号后，调用SmcSetProperties将应用的名称等信息注册到server
    SmsSaveYourself(m_smsConn, SmSaveLocal, false, SmInteractStyleNone, false);
    SmsSaveComplete(m_smsConn);
    getGlobalServer()->clientRegistered(previousId);
}

QString UKUISMClient::program() const
{
    SmProp *p = property(SmProgram);
    if (!p || qstrcmp(p->type, SmARRAY8) || p->num_vals < 1) {
        return QString();
    }

    return QLatin1String((const char*)p->vals[0].value);
}

QStringList UKUISMClient::restartCommand() const
{
    QStringList result;
    SmProp *p = property(SmRestartCommand);
    if (!p || qstrcmp(p->type, SmLISTofARRAY8) || p->num_vals < 1) {
        return result;
    }

    for (int i = 0; i < p->num_vals; i++) {
        result += QLatin1String((const char*)p->vals[i].value);
    }

    return result;
}

QStringList UKUISMClient::discardCommand() const
{
    QStringList result;
    SmProp *p = property(SmDiscardCommand);
    if (!p || qstrcmp(p->type, SmLISTofARRAY8) || p->num_vals < 1) {
        return result;
    }

    for (int i = 0; i < p->num_vals; i++) {
        result += QLatin1String((const char*)p->vals[i].value);
    }

    return result;
}

int UKUISMClient::restartStyleHint() const
{
    SmProp *p = property(SmRestartStyleHint);
    if (!p || qstrcmp(p->type, SmCARD8) || p->num_vals < 1) {
        return SmRestartIfRunning;
    }

    return *((unsigned char*)p->vals[0].value);
}

SmProp *UKUISMClient::property(const char *name) const
{
    foreach (SmProp *prop, m_properties) {
        if (!qstrcmp(prop->name, name)) {
            return prop;
        }
    }
    return nullptr;
}

QString UKUISMClient::userId() const
{
    SmProp *p = property(SmUserID);

    if (!p || qstrcmp(p->type, SmARRAY8) || p->num_vals < 1) {
        return QString();
    }

    return QLatin1String((const char*)p->vals[0].value);
}
