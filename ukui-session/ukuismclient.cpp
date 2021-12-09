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
