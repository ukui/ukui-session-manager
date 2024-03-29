/*****************************************************************
ukuismserver - the UKUI session management server

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

#ifndef UKUISMSERVER_H
#define UKUISMSERVER_H

extern "C" {
#include <X11/SM/SMlib.h>
}
#include <fixx11h.h> //该头文件来自kwindowsystem，主要作用是处理和Qt的头文件一起编译时的冲突
#include <QObject>
#include <QTimer>

class UKUISMClient;
class UKUISMListener;
class UKUISMConnection;
class KProcess;
class OrgKdeKWinSessionInterface;

class UKUISMServer : public QObject
{
    Q_OBJECT
public:
    UKUISMServer();
    ~UKUISMServer();

    UKUISMClient* newClient(SmsConn conn);
    void  deleteClient(UKUISMClient *client);

    //callback
    void clientRegistered(const char *previousId);
    void interactRequest(UKUISMClient *client, int dialogType);
    void interactDone(UKUISMClient *client, bool cancelShutdown_);
    void phase2Request(UKUISMClient *client);
    void saveYourselfDone(UKUISMClient *client, bool success);
    void clientSetProgram(UKUISMClient *client);

    // error handling
    void ioError(IceConn iceConn);

    void startLogoutTimer();
    void startShutdownTimer();
    void startRebootTimer();
public:
    void* watchConnection(IceConn iceConn );
    void removeConnection(UKUISMConnection *conn );
    void restoreSession();
    void restoreWM(const QString &sessionName);
    void startDefaultSession();

    void shutdown();
    bool performLogout();

Q_SIGNALS:
    void logoutFinished();

public Q_SLOTS:
    void cleanUp();//smserver析构前的清理工作


private Q_SLOTS:
    void newConnection(int socket);
    void processData(int socket);
    void wmProcessChange();
    void protectionTimeout();
    void timeoutQuit();
    void timeoutWMQuit();

private:
    void completeShutdownOrCheckpoint();
    void storeSession();
    void completeKilling();
    void startKilling();
    void killWM();
    void completeKillingWM();
    void killingCompleted();
    void cancelShutdown(UKUISMClient *c);

    KProcess* startApplication(const QStringList &command, bool wm = false);
    void executeCommand(const QStringList& command);
    void handlePendingInteractions();

    void startProtection();
    void endProtection();

    void launchWM(const QList<QStringList> &wmStartCommands);

    void tryRestoreNext();
    bool isWM(const UKUISMClient *client) const;
    bool isWM(const QString &program) const;

    //改变客户端的顺序
    void changeClientOrder();
    bool syncDBusEnvironment();
    inline void executeBoxadm();
private:
    enum State {
        Idle,
        LaunchingWM, Restoring,
        Shutdown, Killing, KillingWM, WaitingForKNotify, // shutdown
    };
    State m_state;

    bool clean;
    bool m_saveSession;
    int m_wmPhase1WaitingCount;
    int m_appsToStart;
    int m_appRestored;
    int m_saveType;

    KProcess *m_wmProcess;
    OrgKdeKWinSessionInterface *m_kwinInterface;
    UKUISMClient *m_clientInteracting;

    QList<UKUISMListener*> m_listener;
    QList<UKUISMClient*> m_clients;
    QTimer m_restoreTimer;
    QTimer m_protectionTimer;
    QString m_wm;
    QString m_lastIdRestore;
    QString m_sessionGroup;
    QStringList m_wmCommands;

    QTimer m_systemdLogoutTimer;
    QTimer m_systemdShutdownTimer;
    QTimer m_systemdRebootTimer;

public:
    bool prepareForShutdown();

    bool isCancelLogout() const;
    void setIsCancelLogout(bool isCancelLogout);
    bool isCancelShutdown() const;
    void setIsCancelShutdown(bool isCancelShutdown);
    bool isCancelReboot() const;
    void setIsCancelReboot(bool isCancelReboot);
};

#endif // UKUISMSERVER_H
