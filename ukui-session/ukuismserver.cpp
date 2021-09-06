#include "ukuismserver.h"
#include "ukuismclient.h"
#include "ukuismconnection.h"
#include "ukuikwinsession_interface.h"

//#include <X11/ICE/ICElib.h>
extern "C" {
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
#include <X11/SM/SM.h>
}

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


#include <QApplication>
#include <QPalette>
#include <QDesktopWidget>

#include <KProcess>
#include <KSharedConfig>
#include <KConfigGroup>

#define VendorString "ukuismserver"
#define ReleaseString "1.0"

#define SESSION_PREVIOUS_LOGOUT "saved at previous logout"

enum KWinSessionState {
    Normal = 0,
    Saving = 1,
    Quitting = 2
};

UKUISMServer *the_server = nullptr;

IceAuthDataEntry *authDataEntries = nullptr;

static QTemporaryFile *remTempFile = nullptr;

static IceListenObj *listenObjs = nullptr;
int numTransports = 0;
static bool only_local = false;

extern "C" int _IceTransNoListen(const char *protocol);

static Bool HostBasedAuthProc(char *hostname)
{
    if (only_local)
        return true;
    else
        return false;
}

Status RegisterClientProc(SmsConn smsConn, SmPointer managerData, char *previousId)
{
    UKUISMClient *client = (UKUISMClient*)managerData;
    client->registerClient(previousId);
    qDebug() << "client " << client->program() << " " << client->clientId() << " registered.";
    return 1;
}

void InteractRequestProc(SmsConn smsConn, SmPointer managerData, int dialogType)
{
    the_server->interactRequest((UKUISMClient*)managerData, dialogType );
}

void InteractDoneProc(SmsConn smsConn, SmPointer managerData, Bool cancelShutdown)
{
    the_server->interactDone((UKUISMClient*)managerData, cancelShutdown);
}

void SaveYourselfRequestProc(SmsConn smsConn, SmPointer managerData, int saveType, Bool shutdown, int interactStyle, Bool fast, Bool global)
{
    //如果shutdown为true,则执行关机流程
    if (shutdown) {
        the_server->shutdown();
    } else if (!global) {
        //如果global为false,则只向发送请求的客户端发送save yourself
        SmsSaveYourself(smsConn, saveType, false, interactStyle, fast);
        SmsSaveComplete(smsConn);
    }
}

void KSMSaveYourselfPhase2RequestProc(SmsConn smsConn, SmPointer managerData)
{
    the_server->phase2Request((UKUISMClient*)managerData);
}

void KSMSaveYourselfDoneProc(SmsConn smsConn, SmPointer managerData, Bool success)
{
    the_server->saveYourselfDone((UKUISMClient*)managerData, success);
}

void KSMCloseConnectionProc(SmsConn smsConn, SmPointer managerData, int count, char **reasonMsgs)
{
    the_server->deleteClient((UKUISMClient*)managerData);
    if (count)
        SmFreeReasons(count, reasonMsgs);

    IceConn iceConn = SmsGetIceConnection(smsConn);
    SmsCleanUp(smsConn);
    IceSetShutdownNegotiation(iceConn, False);
    IceCloseConnection(iceConn);
}

void KSMSetPropertiesProc(SmsConn smsConn, SmPointer managerData, int numProps, SmProp **props)
{
    UKUISMClient *client = (UKUISMClient*)managerData;
    for (int i = 0; i < numProps; i++) {
        SmProp *p = client->property(props[i]->name);
        if (p) {
            client->m_properties.removeAll( p );
            SmFreeProperty(p);
        }
        client->m_properties.append(props[i]);
        if (!qstrcmp(props[i]->name, SmProgram))
            the_server->clientSetProgram(client);
    }

    if (numProps)
        free(props);

}

void KSMDeletePropertiesProc(SmsConn smsConn, SmPointer managerData, int numProps, char **propNames)
{
    UKUISMClient *client = (UKUISMClient*)managerData;
    for (int i = 0; i < numProps; i++) {
        SmProp *p = client->property(propNames[i]);
        if (p) {
            client->m_properties.removeAll(p);
            SmFreeProperty(p);
        }
    }
}

void KSMGetPropertiesProc(SmsConn smsConn, SmPointer managerData)
{
    UKUISMClient *client = (UKUISMClient*)managerData;
    SmProp** props = new SmProp*[client->m_properties.count()];
    int i = 0;
    foreach(SmProp *prop, client->m_properties)
        props[i++] = prop;

    SmsReturnProperties(smsConn, i, props);
    delete[] props;
}

static Status NewClientProc(SmsConn conn, SmPointer manager_data, unsigned long *mask_ret, SmsCallbacks *cb, char **failure_reason_ret)
{
    //根据XSMP协议的要求，出错时，这个函数应该返回状态0,并且错误原因要包含在failure_reason_ret中
    *failure_reason_ret = nullptr;

    void* client = ((UKUISMServer*)manager_data)->newClient(conn);

    cb->register_client.callback = RegisterClientProc;
    cb->register_client.manager_data = client;
    cb->interact_request.callback = InteractRequestProc;
    cb->interact_request.manager_data = client;
    cb->interact_done.callback = InteractDoneProc;
    cb->interact_done.manager_data = client;
    cb->save_yourself_request.callback = SaveYourselfRequestProc;
    cb->save_yourself_request.manager_data = client;
    cb->save_yourself_phase2_request.callback = KSMSaveYourselfPhase2RequestProc;
    cb->save_yourself_phase2_request.manager_data = client;
    cb->save_yourself_done.callback = KSMSaveYourselfDoneProc;
    cb->save_yourself_done.manager_data = client;
    cb->close_connection.callback = KSMCloseConnectionProc;
    cb->close_connection.manager_data = client;
    cb->set_properties.callback = KSMSetPropertiesProc;
    cb->set_properties.manager_data = client;
    cb->delete_properties.callback = KSMDeletePropertiesProc;
    cb->delete_properties.manager_data = client;
    cb->get_properties.callback = KSMGetPropertiesProc;
    cb->get_properties.manager_data = client;

    *mask_ret = SmsRegisterClientProcMask |
                SmsInteractRequestProcMask |
                SmsInteractDoneProcMask |
                SmsSaveYourselfRequestProcMask |
                SmsSaveYourselfP2RequestProcMask |
                SmsSaveYourselfDoneProcMask |
                SmsCloseConnectionProcMask |
                SmsSetPropertiesProcMask |
                SmsDeletePropertiesProcMask |
                SmsGetPropertiesProcMask;
    return 1;
}

/*用于打印16进制数据*/
static void fprintfhex(FILE *fp, unsigned int len, char *cp)
{
    static const char hexchars[] = "0123456789abcdef";

    for (; len > 0; len--, cp++) {
        unsigned char s = *cp;
        putc(hexchars[s >> 4], fp);
        putc(hexchars[s & 0x0f], fp);
    }
}

static void write_iceauth(FILE *addfp, FILE *removefp, IceAuthDataEntry *entry)
{
    fprintf(addfp, "add %s \"\" %s %s ", entry->protocol_name, entry->network_id, entry->auth_name);
    fprintfhex(addfp, entry->auth_data_length, entry->auth_data);
    fprintf (addfp, "\n");

    fprintf (removefp,
             "remove protoname=%s protodata=\"\" netid=%s authname=%s\n",
             entry->protocol_name,
             entry->network_id,
             entry->auth_name);
}

#define COOKIE_LEN 16

Status SetAuthentication_local (int count, IceListenObj *listenObjs)
{
    int i;
    for (i = 0; i < count; i ++) {
        char *prot = IceGetListenConnectionString(listenObjs[i]);
        if (!prot) continue;
        char *host = strchr(prot, '/');
        char *sock = nullptr;
        if (host) {
            *host=0;
            host++;
            sock = strchr(host, ':');
            if (sock) {
                *sock = 0;
                sock++;
            }
        }
        if (sock && !strcmp(prot, "local")) {
            chmod(sock, 0700);
        }
        IceSetHostBasedAuthProc(listenObjs[i], HostBasedAuthProc);
        free(prot);
    }
    return 1;
}

Status SetAuthentication (int count, IceListenObj *listenObjs, IceAuthDataEntry **authDataEntries)
{
    QTemporaryFile addTempFile;
    remTempFile = new QTemporaryFile;

    if (!addTempFile.open() || !remTempFile->open())
        return 0;

    if ((*authDataEntries = (IceAuthDataEntry *) malloc(count * 2 * sizeof(IceAuthDataEntry))) == nullptr)
        return 0;

    FILE *addAuthFile = fopen(QFile::encodeName(addTempFile.fileName()).constData(), "r+");
    FILE *remAuthFile = fopen(QFile::encodeName(remTempFile->fileName()).constData(), "r+");

    for (int i = 0; i < numTransports * 2; i += 2) {
        (*authDataEntries)[i].network_id = IceGetListenConnectionString(listenObjs[i/2]);
        (*authDataEntries)[i].protocol_name = (char*)"ICE";
        (*authDataEntries)[i].auth_name = (char*)"MIT-MAGIC-COOKIE-1";

        (*authDataEntries)[i].auth_data = IceGenerateMagicCookie(COOKIE_LEN);
        (*authDataEntries)[i].auth_data_length = COOKIE_LEN;

        (*authDataEntries)[i+1].network_id = IceGetListenConnectionString(listenObjs[i/2]);
        (*authDataEntries)[i+1].protocol_name = (char*)"XSMP";
        (*authDataEntries)[i+1].auth_name = (char*)"MIT-MAGIC-COOKIE-1";

        (*authDataEntries)[i+1].auth_data = IceGenerateMagicCookie(COOKIE_LEN);
        (*authDataEntries)[i+1].auth_data_length = COOKIE_LEN;

        write_iceauth(addAuthFile, remAuthFile, &(*authDataEntries)[i]);
        write_iceauth(addAuthFile, remAuthFile, &(*authDataEntries)[i+1]);

        IceSetPaAuthData (2, &(*authDataEntries)[i]);

        IceSetHostBasedAuthProc(listenObjs[i/2], HostBasedAuthProc);
    }
    fclose(addAuthFile);
    fclose(remAuthFile);

    QString iceAuth = QStandardPaths::findExecutable(QStringLiteral("iceauth"));
    if (iceAuth.isEmpty())
    {
        qDebug() << "KSMServer: could not find iceauth";
        return 0;
    }

    KProcess p;
    p << iceAuth << QStringLiteral("source") << addTempFile.fileName();
    p.execute();

    return (1);
}

void FreeAuthenticationData(int count, IceAuthDataEntry *authDataEntries)
{
    /* Each transport has entries for ICE and XSMP */
    if (only_local)
        return;

    for (int i = 0; i < count * 2; i++) {
        free(authDataEntries[i].network_id);
        free(authDataEntries[i].auth_data);
    }

    free (authDataEntries);

    QString iceAuth = QStandardPaths::findExecutable(QStringLiteral("iceauth"));
    if (iceAuth.isEmpty())
    {
        qDebug() << "KSMServer: could not find iceauth";
        return;
    }

    if (remTempFile)
    {
        KProcess p;
        p << iceAuth << QStringLiteral("source") << remTempFile->fileName();
        p.execute();
    }

    delete remTempFile;
    remTempFile = nullptr;
}

void UKUISMWatchProc(IceConn iceConn, IcePointer client_data, Bool opening, IcePointer *watch_data)
{
    UKUISMServer *ds = (UKUISMServer*)client_data;

    if (opening) {
        *watch_data = (IcePointer)ds->watchConnection(iceConn);
    }
    else  {
        ds->removeConnection((UKUISMConnection*)*watch_data);
    }
}


UKUISMServer::UKUISMServer() : m_kwinInterface(new OrgKdeKWinSessionInterface(QStringLiteral("org.ukui.KWin"), QStringLiteral("/Session"), QDBusConnection::sessionBus(), this))
                             , m_state(Idle), m_saveSession(false), m_wmPhase1WaitingCount(0), m_clientInteracting(nullptr), m_sessionGroup(QStringLiteral(""))
                             , m_wm(QStringLiteral("ukui-kwin_x11"))
{
    m_wmCommands = QStringList({m_wm});
    the_server = this;

    only_local = true;

    if (only_local) {
        _IceTransNoListen("tcp");//这个函数到底是哪个库里面的？还是需要自己定义？
    }

    char errormsg[256];
    if (!SmsInitialize((char*)VendorString, (char*)ReleaseString, NewClientProc, (SmPointer) this, HostBasedAuthProc, 256, errormsg)) {
        qDebug() << "xsmpserver: could not register XSM protocol";
    }

    if (!IceListenForConnections(&numTransports, &listenObjs, 256, errormsg)) {
        qDebug() << "KSMServer: Error listening for connections: " << errormsg;
        qDebug() << "KSMServer: Aborting.";
        exit(1);
    }

    {
        QByteArray fName = QFile::encodeName(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation)
                                             + QDir::separator()
                                             + QStringLiteral("xsmpserver"));

        QString display = QString::fromLocal8Bit(::getenv("DISPLAY"));
        display.remove(QRegExp(QStringLiteral("\\.[0-9]+$")));
        int i = 0;
        while((i = display.indexOf(QLatin1Char(':'))) >= 0) {
            display[i] = '_';
        }

        while((i = display.indexOf(QLatin1Char('/'))) >= 0) {
            display[i] = '_';
        }

        fName += '_'+display.toLocal8Bit();
        FILE *f;
        f = ::fopen(fName.data(), "w+");
        if (!f) {
            QString str = QString(QStringLiteral("UKUISMServer: cannot open %s: %s")).arg(fName.data()).arg(strerror(errno));
            qDebug() << str;
            qDebug() << "UKUISMServer: Aborting.";
            exit(1);
        }
        char *session_manager = IceComposeNetworkIdList(numTransports, listenObjs);
        fprintf(f, "%s\n%i\n", session_manager, getpid());
        fclose(f);
        setenv("SESSION_MANAGER", session_manager, true);

        //这里要将session_manager变量传递给其他需要的进程

        free(session_manager);
    }

    if (only_local) {
        if (!SetAuthentication_local(numTransports, listenObjs))
            qFatal("ukuismserver : authentication setup failed.");
    } else {
        if (!SetAuthentication(numTransports, listenObjs, &authDataEntries))
            qFatal("ukuismserver : authentication setup failed.");
    }


    IceAddConnectionWatch(UKUISMWatchProc, (IcePointer)this);

    UKUISMListener *con;
    for (int i = 0; i < numTransports; i++) {
        fcntl(IceGetListenConnectionNumber(listenObjs[i]), F_SETFD, FD_CLOEXEC);
        con = new UKUISMListener(listenObjs[i]);
        m_listener.append(con);
        connect(con, &UKUISMListener::activated, this, &UKUISMServer::newConnection);
    }


//    connect(qApp, &QApplication::aboutToQuit, this, &UKUISMServer::cleanUp);
    connect(&m_restoreTimer, &QTimer::timeout, this, &UKUISMServer::tryRestoreNext);

    qDebug() << "finish construct ukuismserver";
}

UKUISMServer::~UKUISMServer()
{
    qDeleteAll(m_listener);
    the_server = nullptr;
    cleanUp();
}

UKUISMClient *UKUISMServer::newClient(SmsConn conn)
{
    UKUISMClient *client = new UKUISMClient(conn);
    m_clients.append(client);
    return client;
}

void UKUISMServer::deleteClient(UKUISMClient *client)
{
    m_clients.removeAll(client);
    qDebug() << "m_clients remove client " << client->clientId();

    if (client == m_clientInteracting) {
        m_clientInteracting = nullptr;
        handlePendingInteractions();
    }

    delete client;

    if (m_state == Shutdown)
        completeShutdownOrCheckpoint();

    if (m_state == Killing)
        completeKilling();

    if (m_state == KillingWM )
        completeKillingWM();
}

void UKUISMServer::interactRequest(UKUISMClient *client, int dialogType)
{
    qDebug() << client->clientId() << "ask for interact";

    //如果是关机阶段，该客户端的请求就要暂时挂起
    if (m_state == Shutdown) {
        //将pendingInteraction属性设置为true,以便在handlePendingInteractions中处理
        qDebug() << "pending client " << client->clientId();
        client->m_pendingInteraction = true;
    } else {
        //非关机阶段，则直接授予客户端交互权限
        qDebug() << "sending save yourself to client " << client->clientId();
        SmsInteract(client->connection());
    }
    //处理被挂起的客户端请求
    handlePendingInteractions();
}

void UKUISMServer::interactDone(UKUISMClient *client, bool cancelShutdown_)
{
    //如果交互完成的客户端与服务器保存的m_clientInteraction信息不一致，则返回，一般不会发生，因为退出时的交互对话框是模态的
    if (client != m_clientInteracting)
        return;
    //重置m_clientInteraciton，以便处理下一个挂起的客户端
    qDebug() << m_clientInteracting->clientId() << "interact done";
    m_clientInteracting = nullptr;
    //如果客户端取消关机，则向所有客户端发送取消关机信号
    if (cancelShutdown_) {
        cancelShutdown(client);
    } else {
        //处理下一个挂起的客户端
        handlePendingInteractions();
    }
}

void UKUISMServer::phase2Request(UKUISMClient *client)
{
    qDebug() << "wm ask for phase2";
    //这两个成员变量对于窗管才有用
    client->m_waitForPhase2 = true;
    client->m_wasPhase2 = true;

    completeShutdownOrCheckpoint();
    if (isWM(client) && m_wmPhase1WaitingCount > 0) {
        --m_wmPhase1WaitingCount;
        //窗管完成phase1保存，请求phase2，服务器先向所有其他客户端发送保存命令
        if(m_wmPhase1WaitingCount == 0) {
            foreach (UKUISMClient *c, m_clients) {
                if (!isWM(c)) {
                    qDebug() << "wm done phase1, sending saveyourself to " << c->clientId();
                    SmsSaveYourself(c->connection(), m_saveType, m_saveType != SmSaveLocal,
                        m_saveType != SmSaveLocal ? SmInteractStyleAny : SmInteractStyleNone,
                        false);
                }
            }
        }
    }
}

void UKUISMServer::saveYourselfDone(UKUISMClient *client, bool success)
{
    if (m_state == Idle) {
        QStringList discard = client->discardCommand();
        if( !discard.isEmpty())
            executeCommand( discard );
        return;
    }

    if (success) {
        qDebug() << client->clientId() << "done save";
        client->m_saveYourselfDone = true;
        completeShutdownOrCheckpoint();
    } else {
        //即使保存不成功也要按照成功的方式进行下一步，否则无法保存下一个客户端
        client->m_saveYourselfDone = true;
        completeShutdownOrCheckpoint();
    }
}

void UKUISMServer::clientSetProgram(UKUISMClient *client)
{
    if(isWM(client)) {
        qDebug() << "windowManager loaded";
    }
}

void *UKUISMServer::watchConnection(IceConn iceConn)
{
    UKUISMConnection* conn = new UKUISMConnection(iceConn);
    connect(conn, &UKUISMConnection::activated, this, &UKUISMServer::processData);
    return (void*)conn;
}

void UKUISMServer::restoreSession(const QString &sessionName)
{
    if(m_state != Idle)
        return;

    m_state = LaunchingWM;

    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    m_sessionGroup = QLatin1String("Session: ") + sessionName;
    KConfigGroup configSessionGroup(config, m_sessionGroup);

    int count = configSessionGroup.readEntry("count", 0);
    m_appsToStart = count;

    QList<QStringList> wmStartCommands;
    if (!m_wm.isEmpty()) {
        for (int i = 1; i <= count; i++) {
            QString n = QString::number(i);
            if (isWM(configSessionGroup.readEntry(QStringLiteral("program") + n, QString()))) {
                wmStartCommands << configSessionGroup.readEntry(QStringLiteral("restartCommand") + n, QStringList());
            }
        }
    }

    if (wmStartCommands.isEmpty()) {
        wmStartCommands << m_wmCommands;
    }

    launchWM(wmStartCommands);
}

void UKUISMServer::startDefaultSession()
{
    if(m_state != Idle ) return;
    m_state = LaunchingWM;

    m_sessionGroup = QString();
    launchWM(QList<QStringList>() << m_wmCommands);
}

void UKUISMServer::clientRegistered(const char *previousId)
{
    if (previousId && m_lastIdRestore == QString::fromLocal8Bit(previousId)) {
        tryRestoreNext();
    }
}

void UKUISMServer::shutdown()
{
    //保存关机
    //是否需要设置m_state?
    qDebug() << "begin performlogout";
    performLogout();
}

void UKUISMServer::performLogout()
{
    //已经在执行关机，直接返回
    if (m_state >= Shutdown) {
        return;
    }

    //暂时注释此处
//    if (m_state != Idle) {
//        QTimer::singleShot(1000, this, &UKUISMServer::performLogout);
//    }

    m_kwinInterface->setState(KWinSessionState::Saving);
    m_state = Shutdown;
    m_saveSession = true;

    if (m_saveSession)
        m_sessionGroup = QStringLiteral("Session: ") + QString::fromLocal8Bit(SESSION_PREVIOUS_LOGOUT);

    //将桌面背景设置为黑色
//    QPalette palette;
//    palette.setColor(QApplication::desktop()->backgroundRole(), Qt::black);
//    QApplication::setPalette(palette);
    m_wmPhase1WaitingCount = 0;
    m_saveType = SmSaveBoth;

    foreach (UKUISMClient *c, m_clients) {
        c->resetState();
        if(isWM(c))
            ++m_wmPhase1WaitingCount;
    }
    if (m_wmPhase1WaitingCount > 0) {
        foreach (UKUISMClient *c, m_clients) {
            //先向窗管发送保存自身的信号
            if(isWM(c)) {
                qDebug() << "sending save signal to wm first";
                SmsSaveYourself(c->connection(), m_saveType, true, SmInteractStyleAny, false);
            }

        }
    } else {
        foreach (UKUISMClient *c, m_clients) {
            qDebug() << "sending saveourself to client " << c->program() << " " << c->clientId();
            SmsSaveYourself(c->connection(), m_saveType, true, SmInteractStyleAny, false);
        }

    }

    if (m_clients.isEmpty())
        completeShutdownOrCheckpoint();
}

void UKUISMServer::cleanUp()
{
    if (clean) return;
    clean = true;
    IceFreeListenObjs (numTransports, listenObjs);


    QByteArray fName = QFile::encodeName(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + QLatin1Char('/') + QStringLiteral("KSMserver"));
    QString display  = QString::fromLocal8Bit(::getenv("DISPLAY"));

    display.remove(QRegExp(QStringLiteral("\\.[0-9]+$")));
    int i;
    while( (i = display.indexOf(QLatin1Char(':'))) >= 0)
         display[i] = '_';
    while( (i = display.indexOf(QLatin1Char('/'))) >= 0)
         display[i] = '_';

    fName += '_'+display.toLocal8Bit();
    ::unlink(fName.data());

    FreeAuthenticationData(numTransports, authDataEntries);
}

void UKUISMServer::newConnection(int socket)
{
    IceAcceptStatus status;
    IceConn iceConn = IceAcceptConnection(((UKUISMListener*)sender())->listenObj, &status);
    if(iceConn == nullptr)
        return;
    IceSetShutdownNegotiation(iceConn, False);
    IceConnectStatus cstatus;
    while ((cstatus = IceConnectionStatus(iceConn)) == IceConnectPending) {
        (void)IceProcessMessages(iceConn, nullptr, nullptr);
    }

    if (cstatus != IceConnectAccepted) {
        if (cstatus == IceConnectIOError) {
            qDebug() << "IO error opening ICE Connection!";
        }
        else {
            qDebug() << "ICE Connection rejected!";
        }

        (void)IceCloseConnection(iceConn);
        return;
    }

    fcntl(IceConnectionNumber(iceConn), F_SETFD, FD_CLOEXEC);
}

void UKUISMServer::processData(int socket)
{
    IceConn iceConn = ((UKUISMConnection*)sender())->iceConn;
    IceProcessMessagesStatus status = IceProcessMessages(iceConn, nullptr, nullptr);
    if (status == IceProcessMessagesIOError) {
        qDebug() << "processData called and status is IOError";
        IceSetShutdownNegotiation(iceConn, False);
        QList<UKUISMClient*>::iterator it = m_clients.begin();
        QList<UKUISMClient*>::iterator const itEnd = m_clients.end();
        while ((it != itEnd) && *it && (SmsGetIceConnection((*it)->connection()) != iceConn)) {
            ++it;
        }

        if ((it != itEnd) && *it) {
            SmsConn smsConn = (*it)->connection();
            deleteClient(*it);
            SmsCleanUp(smsConn);
        }

        (void)IceCloseConnection(iceConn);
    }
}

void UKUISMServer::wmProcessChange()
{
    if(m_state != LaunchingWM)
    {
        m_wmProcess = nullptr;
        return;
    }

    if(m_wmProcess->state() == QProcess::NotRunning)
    {
        if(m_wm == QLatin1String("ukui-kwin_x11") )
            return;

        m_wm = QStringLiteral("ukui-kwin_x11");
        m_wmCommands = (QStringList() << QStringLiteral("ukui-kwin_x11"));

        launchWM(QList<QStringList>() << m_wmCommands);
        return;
    }
}

void UKUISMServer::timeoutQuit()
{
    killWM();
}

void UKUISMServer::timeoutWMQuit()
{
    if (m_state == KillingWM) {
        qDebug() << "SmsDie WM timeout";
    }
    killingCompleted();
}

void UKUISMServer::completeShutdownOrCheckpoint()
{
    //只能在shutdown阶段调用
    if (m_state != Shutdown)
        return;

    QList<UKUISMClient*> pendingClients;
    pendingClients = m_clients;
    //此处判断除窗管之外的客户端是否全部完成保存，没有的话就返回
    foreach(UKUISMClient *c, pendingClients ) {
        if (!c->m_saveYourselfDone && !c->m_waitForPhase2)
            qDebug() << "there are none-wm client haven't save";
            return;
    }
    //窗管正在等待phase2阶段的保存，则向其发送保存phase2的信号
    bool waitForPhase2 = false;
    foreach(UKUISMClient *c, pendingClients) {
        if (!c->m_saveYourselfDone && c->m_waitForPhase2) {
            c->m_waitForPhase2 = false;
            qDebug() << "sending saveyourselfphase2 to " << c->clientId();
            SmsSaveYourselfPhase2(c->connection());
            waitForPhase2 = true;
        }
    }

    if (waitForPhase2)
        return;
    //运行到这里说明窗管和普通客户端都完成了保存，开始保存会话信息到磁盘中
    if (m_saveSession) {
        qDebug() << "store session informantion in rcfile";
        storeSession();
    }

    //会话信息保存完毕后开始退出，杀死客户端
    if (m_state == Shutdown) {
        m_state = WaitingForKNotify;
        if (m_state == WaitingForKNotify) {
            qDebug() << "begin killint client";
            startKilling();
        }
    }
}

void UKUISMServer::storeSession()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    config->reparseConfiguration();//更新配置文件

    KConfigGroup configSessionGroup(config, m_sessionGroup);
    int count =  configSessionGroup.readEntry("count", 0);
    for (int i = 1; i <= count; i++) {
        //读取discardcommand
        QStringList discardCommand = configSessionGroup.readPathEntry(QLatin1String("discardCommand") + QString::number(i), QStringList());
        if (discardCommand.isEmpty())
            continue;

        //这一步的目的是寻找是否有重复的discardcommand， 如果没有就直接执行discardcommand
        QList<UKUISMClient*>::iterator it = m_clients.begin();
        QList<UKUISMClient*>::iterator const itEnd = m_clients.end();
        while ((it != itEnd) && *it && (discardCommand != (*it)->discardCommand()))
            ++it;

        if ((it != itEnd)&& *it)
            continue;

        executeCommand(discardCommand);
    }
    //删除上一次保存的会话信息
    config->deleteGroup(m_sessionGroup);
    KConfigGroup cg(config, m_sessionGroup);
    count =  0;
    //将wm移动到客户端列表的第一个
    foreach (UKUISMClient *c, m_clients)
        if (isWM(c)) {
            m_clients.removeAll(c);
            m_clients.prepend(c);
            break;
        }

    //遍历每一个客户端，存储客户端的信息到文件中
    foreach (UKUISMClient *c, m_clients) {
        int restartHint = c->restartStyleHint();
        if (restartHint == SmRestartNever)
            continue;

        QString program = c->program();
        QStringList restartCommand = c->restartCommand();
        if (program.isEmpty() && restartCommand.isEmpty())
           continue;

        count++;
        QString n = QString::number(count);
        cg.writeEntry(QStringLiteral("program") + n, program);
        cg.writeEntry(QStringLiteral("clientId") + n, c->clientId());
        cg.writeEntry(QStringLiteral("restartCommand") + n, restartCommand);
        cg.writePathEntry(QStringLiteral("discardCommand") + n, c->discardCommand());
        cg.writeEntry(QStringLiteral("restartStyleHint") + n, restartHint);
        cg.writeEntry(QStringLiteral("userId") + n, c->userId());
        cg.writeEntry(QStringLiteral("wasWm") + n, isWM(c));
    }
    cg.writeEntry( "count", count );

    config->sync();
}

void UKUISMServer::completeKilling()
{
    if (m_state == Killing) {
        //这一段的含义是只要客户端列表中还有非窗管的客户端存在，则等待，直到客户端中只有一个窗管，则开始杀死窗管
        bool wait = false;
        foreach(UKUISMClient *c, m_clients) {
            if(isWM(c))
                continue;
            wait = true;
        }

        if(wait)
            return;

        killWM();
    }
}

void UKUISMServer::startKilling()
{
    if (m_state == Killing) {
        return;
    }

    m_state = Killing;

    m_kwinInterface->setState(KWinSessionState::Quitting);

    foreach(UKUISMClient *c, m_clients) {
        if(isWM(c))//最后再杀死窗管
            continue;
        qDebug() << c->clientId() << "kill connection";
        SmsDie(c->connection());
    }

    completeKilling();
    QTimer::singleShot(10000, this, &UKUISMServer::timeoutQuit);
}

void UKUISMServer::killWM()
{
    if(m_state != Killing )
        return;

    m_state = KillingWM;
    bool iswm = false;
    foreach(UKUISMClient *c, m_clients) {
        if (isWM(c)) {
            iswm = true;
            qDebug() << "wm kill connection";
            SmsDie(c->connection());
        }
    }

    if (iswm) {
        completeKillingWM();
        //5秒后窗管没有杀死，就直接终止程序
        QTimer::singleShot(5000, this, &UKUISMServer::timeoutWMQuit);
    } else {
        killingCompleted();
    }

}

void UKUISMServer::completeKillingWM()
{
    if(m_state == KillingWM) {
        if(m_clients.isEmpty())
            killingCompleted();
    }
}

void UKUISMServer::killingCompleted()
{
    qDebug() << "done killing, exit";
    qApp->quit();
}

void UKUISMServer::cancelShutdown(UKUISMClient *c)
{
    m_clientInteracting = nullptr;

    foreach(UKUISMClient *c, m_clients) {
        SmsShutdownCancelled(c->connection());
        if(c->m_saveYourselfDone) {
            QStringList discard = c->discardCommand();
            if(!discard.isEmpty())
                executeCommand( discard );
        }
    }

    m_state = Idle;

    m_kwinInterface->setState(KWinSessionState::Normal);
}

KProcess *UKUISMServer::startApplication(const QStringList &command, bool wm)
{
    if (wm) {
        KProcess *process = new KProcess(this);
        *process << command;
        connect(process, static_cast<void (KProcess::*)(QProcess::ProcessError)>(&KProcess::error), process, &KProcess::deleteLater);
        connect(process, static_cast<void (KProcess::*)(int, QProcess::ExitStatus)>(&KProcess::finished), process, &KProcess::deleteLater);
        process->start();
        return process;
    } else {
        int n = command.count();
        QString app = command[0];
        QStringList argList;
        for (int i = 1; i < n; i++) {
            argList.append(command[i]);
        }

        QProcess appProcess;
        appProcess.start(app, argList);
        return nullptr;
    }
}

void UKUISMServer::executeCommand(const QStringList &command)
{
    if (command.isEmpty())
        return;

    KProcess::execute(command);
}

void UKUISMServer::handlePendingInteractions()
{
    //该函数在保存退出阶段，第一次进入该函数时，clientInteracting一定是nullptr
    if (m_clientInteracting)
        return;
    //遍历客户端，找到第一个正在挂起的客户端，将其初始化为m_clientInteracting
    foreach(UKUISMClient *c, m_clients) {
        if (c->m_pendingInteraction) {
            m_clientInteracting = c;
            c->m_pendingInteraction = false;
            break;
        }
    }
    //向m_clientInteracting授予交互权限
    if (m_clientInteracting) {
        qDebug() << "sending interact to " << m_clientInteracting->clientId();
        SmsInteract(m_clientInteracting->connection());
    }
}

void UKUISMServer::launchWM(const QList<QStringList> &wmStartCommands)
{
    assert(m_state == LaunchingWM);

    if (!(qEnvironmentVariableIsSet("WAYLAND_DISPLAY") || qEnvironmentVariableIsSet("WAYLAND_SOCKET"))) {
        qDebug() << "smserver launch wm";
        m_wmProcess = startApplication(wmStartCommands[0], true);
        connect(m_wmProcess, SIGNAL(error(QProcess::ProcessError)), SLOT(wmProcessChange()));
        connect(m_wmProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(wmProcessChange()));
    }
}

void UKUISMServer::tryRestoreNext()
{
    if(m_state != Restoring)
        return;

    m_restoreTimer.stop();
    KConfigGroup config(KSharedConfig::openConfig(), m_sessionGroup);

    while (m_appRestored < m_appsToStart) {
        QString n = QString::number(++m_appRestored);
        QString clientId = config.readEntry(QLatin1String("clientId") + n, QString());

        bool alreadyStarted = false;
        foreach (UKUISMClient *c, m_clients) {
            if (QString::fromLocal8Bit(c->clientId()) == clientId) {
                alreadyStarted = true;
                break;
            }
        }

        if (alreadyStarted)
            continue;

        QStringList restartCommand = config.readEntry(QLatin1String("restartCommand") + n, QStringList());

        if (restartCommand.isEmpty() || (config.readEntry(QStringLiteral("restartStyleHint") + n, 0) == SmRestartNever)) {
            continue;
        }

        if (isWM(config.readEntry( QStringLiteral("program") + n, QString())))
            continue;
        if (config.readEntry(QStringLiteral("wasWm") + n, false))
            continue;

        startApplication(restartCommand);

        m_lastIdRestore = clientId;
        if (!m_lastIdRestore.isEmpty()) {
            m_restoreTimer.setSingleShot(true);
            m_restoreTimer.start(2000);
            return;
        }
    }

    m_appRestored = 0;
    m_lastIdRestore.clear();

    m_state = Idle;
}

bool UKUISMServer::isWM(const UKUISMClient *client) const
{
    return isWM(client->program());
}

bool UKUISMServer::isWM(const QString &program) const
{
    QString wmName = m_wm.mid(m_wm.lastIndexOf(QDir::separator()) + 1);
    QString programName = program.mid(program.lastIndexOf(QDir::separator()) + 1);
    return programName == wmName;
}

void UKUISMServer::removeConnection(UKUISMConnection *conn)
{
    delete conn;
}

void UKUISMServer::restoreSession()
{
    m_appRestored = 0;
    m_lastIdRestore.clear();
    m_state = Restoring;

    tryRestoreNext();
}
