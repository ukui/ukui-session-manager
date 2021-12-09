#include <QDBusAbstractAdaptor>
//#include "sessioninhibitcontext.h"
#include "sessionmanagercontext.h"
#include "sessiondbusadaptor.h"

SessionManagerDBusContext::SessionManagerDBusContext(ModuleManager *manager, QObject *parent)
    :QObject(parent)
    , mManager(manager)
    , minhibit(new usminhibit())
    , m_systemdProvider(new SystemdProvider())
    , m_ukuiProvider(new UKUIProvider())
    , m_serviceWatcher(new QDBusServiceWatcher(this))
{
    new SessionDBusAdaptor(this);

    connect(mManager, &ModuleManager::moduleStateChanged, this , &SessionManagerDBusContext::moduleStateChanged);
    connect(mManager, &ModuleManager::finished,this,&SessionManagerDBusContext::emitPrepareForPhase2);
    connect(minhibit, &usminhibit::inhibitRemove,this,&SessionManagerDBusContext::simulateUserActivity);
    connect(minhibit, &usminhibit::inhibitAdd,this,&SessionManagerDBusContext::simulateUserActivity);

    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &SessionManagerDBusContext::on_serviceUnregistered);
}

SessionManagerDBusContext::~SessionManagerDBusContext() = default;


Q_NOREPLY void SessionManagerDBusContext::startupfinished(const QString& appName ,const QString& string)
{
    return mManager->startupfinished(appName,string);
}

bool SessionManagerDBusContext::canSwitch()
{
    //判断能否切换用户只需要用到systemdProvide的功能。
    return m_systemdProvider->canAction(UkuiPower::PowerSwitchUser);
}

bool SessionManagerDBusContext::canHibernate()
{
    return m_systemdProvider->canAction(UkuiPower::PowerHibernate);
}

bool SessionManagerDBusContext::canSuspend()
{
    //睡眠通过systemd判断
    return m_systemdProvider->canAction(UkuiPower::PowerSuspend);
}


bool SessionManagerDBusContext::canLogout()
{
    //暂时都返回true
    return true;
}

bool SessionManagerDBusContext::canReboot()
{
    //界面处已经判断过inhibitor,会有界面提示，此处有待优化
    //判断systemd和ukui-session的inhibitor
    return m_systemdProvider->canAction(UkuiPower::PowerReboot) && m_ukuiProvider->canAction(UkuiPower::PowerReboot);
}

bool SessionManagerDBusContext::canPowerOff()
{
    return m_systemdProvider->canAction(UkuiPower::PowerShutdown) && m_ukuiProvider->canAction(UkuiPower::PowerShutdown);
}

Q_NOREPLY void SessionManagerDBusContext::switchUser()
{
    m_systemdProvider->doAction(UkuiPower::PowerSwitchUser);
}

Q_NOREPLY void SessionManagerDBusContext::hibernate()
{
    m_systemdProvider->doAction(UkuiPower::PowerHibernate);
}

Q_NOREPLY void SessionManagerDBusContext::suspend()
{
    m_systemdProvider->doAction(UkuiPower::PowerSuspend);
}


Q_NOREPLY void SessionManagerDBusContext::logout()
{
    //xsmp协议的退出保存机制
    theServer->performLogout();
    //保证一定会退出
    QTimer::singleShot(20000, [](){
        //判断注销动作是否被取消
        if (!theServer->isCancelLogout()) {
            QDBusInterface face("org.freedesktop.login1",
                                "/org/freedesktop/login1/session/self",
                                "org.freedesktop.login1.Session",
                                QDBusConnection::systemBus());

            face.call("Terminate");
        } else {
            //恢复m_isCancelLogout为false，不影响下一次注销
            theServer->setIsCancelLogout(false);
        }
        //            qDebug() << "calling force logout";
        //            QDBusInterface face("org.freedesktop.login1",
        //                                "/org/freedesktop/login1/user/self",
        //                                "org.freedesktop.login1.User",
        //                                QDBusConnection::systemBus());

        //            face.call("Kill", 15);
    });
}

Q_NOREPLY void SessionManagerDBusContext::reboot()
{
    theServer->performLogout();
    connect(theServer, &UKUISMServer::logoutFinished, [this](){
        this->m_systemdProvider->doAction(UkuiPower::PowerReboot);
    });

}

Q_NOREPLY void SessionManagerDBusContext::powerOff()
{
    theServer->performLogout();
    connect(theServer, &UKUISMServer::logoutFinished, [this](){
        this->m_systemdProvider->doAction(UkuiPower::PowerShutdown);
    });

}

Q_NOREPLY void SessionManagerDBusContext::startModule(const QString& name)
{
    mManager->startProcess(name, true);
}

Q_NOREPLY void SessionManagerDBusContext::stopModule(const QString& name)
{
    mManager->stopProcess(name);
}

uint SessionManagerDBusContext::Inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags)
{
    uint result = minhibit->addInhibit(app_id, toplevel_xid, reason, flags);
    if(result < 0){
        return 0;
    }

    QString dbusService = message().service();
    qDebug() << "SessionManagerDBusContext message().service():" << dbusService;

    QStringList keys = m_hashInhibitionServices.keys();
    if(!keys.contains(dbusService))
    {
        QList<quint32> cookies;
        cookies.append(result);
        m_hashInhibitionServices.insert(dbusService, cookies);
        m_serviceWatcher->addWatchedService(dbusService);
        qDebug() << "m_serviceWatcher services:..." << m_serviceWatcher->watchedServices();
    }
    else{
        for(auto iter = m_hashInhibitionServices.begin(); iter != m_hashInhibitionServices.end(); iter++){
            qDebug() << "Inhibit iter.key()" << iter.key() << dbusService;

            if(iter.key() == dbusService)
            {
                QList<quint32> cookies = iter.value();
                if(!cookies.contains(result)){
                    cookies.append(result);
                    m_hashInhibitionServices[dbusService] = cookies;
                    break;
                }
            }
        }
    }
    qDebug() << "Inhibit m_hashInhibitionServices..." << m_hashInhibitionServices;
    emit this->inhibitadded(flags);
    return result;
}

Q_NOREPLY void SessionManagerDBusContext::Uninhibit(uint cookie){
    uint result = minhibit->unInhibit(cookie);
    if(result > 0){
        emit inhibitremove(result);
    }
    for(auto iter = m_hashInhibitionServices.begin();iter != m_hashInhibitionServices.end(); iter++){
        QList<quint32> cookies = iter.value();
        for(int i = 0; i < cookies.length(); i++)
        {
            if(cookie == cookies[i])
            {
                qDebug() << "Uninhibit cookies:" << cookies << "cookie:" << cookie;
                if(cookies.length() > 1){
                    cookies.removeAt(i);
                    m_hashInhibitionServices[iter.key()] = cookies;
                }
                else{
                    m_serviceWatcher->removeWatchedService(iter.key());
                    m_hashInhibitionServices.remove(iter.key());
                    qDebug() << "Uninhibit m_hashInhibitionServices...:" << m_hashInhibitionServices;
                }
                qDebug() << "Uninhibit m_hashInhibitionServices:" << m_hashInhibitionServices;
                return;
            }
        }
    }
}

QStringList SessionManagerDBusContext::GetInhibitors(){
    return minhibit->getInhibitor();
}

bool SessionManagerDBusContext::IsSessionRunning(){
    QString xdg_session_desktop = qgetenv("XDG_SESSION_DESKTOP").toLower();
    if(xdg_session_desktop == "ukui")
        return true;
    return false;
}

QString SessionManagerDBusContext::GetSessionName(){
    QString xdg_session_desktop = qgetenv("XDG_SESSION_DESKTOP").toLower();
    if(xdg_session_desktop == "ukui")
        return "ukui-session";
    return "error";
}

bool SessionManagerDBusContext::IsInhibited(quint32 flags){
    return minhibit->isInhibited(flags);
}

Q_NOREPLY void SessionManagerDBusContext::emitStartLogout(){
    qDebug()<<"emit  StartLogout";
    emit StartLogout();
}

Q_NOREPLY void SessionManagerDBusContext::emitPrepareForSwitchuser(){
    qDebug()<<"emit  PrepareForSwitchuser";
    emit PrepareForSwitchuser();
}

Q_NOREPLY void SessionManagerDBusContext::emitPrepareForPhase2(){
    qDebug()<<"emit  PrepareForPhase2";
    emit PrepareForPhase2();
}

Q_NOREPLY void SessionManagerDBusContext::simulateUserActivity(){
    qDebug()<<"simulate User Activity";
    KIdleTime::instance()->simulateUserActivity();
}

void SessionManagerDBusContext::on_serviceUnregistered(const QString &serviceName)
{
    qDebug() << "onServiceUnregistered..." << serviceName;

    for(auto iter = m_hashInhibitionServices.begin(); iter != m_hashInhibitionServices.end(); iter++)
    {
        if(iter.key() == serviceName)
        {
            QList<quint32> cookies = iter.value();
            for(auto i = 0; i < cookies.length(); i++){
                quint32 cookie = cookies[i];
                qDebug() << "on_serviceUnregistered cookie:" << cookie;
                Uninhibit(cookie);
            }
            return;
        }
    }
    qDebug() << "on_serviceUnregistered unfind serviceName:" << serviceName;
}
