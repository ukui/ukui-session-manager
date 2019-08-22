#include "powerprovider.h"

#include <QDBusInterface>
#include <QDebug>

#define SYSTEMD_SERVICE         "org.freedesktop.login1"
#define SYSTEMD_PATH            "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE       "org.freedesktop.login1.Manager"

#define UKUI_SERVICE      "org.ukui.Session"
#define UKUI_PATH         "/org/ukui/Session"
#define UKUI_INTERFACE    "org.ukui.Session"

#define PROPERTIES_INTERFACE    "org.freedesktop.DBus.Properties"

static bool dbusCall(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString &method)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        return false;
    }

    QDBusMessage msg = dbus.call(method);

    if (!msg.errorName().isEmpty()) {
        qWarning() << "Dbus error: " << msg;
    }

    return msg.arguments().isEmpty() ||
           msg.arguments().constFirst().isNull() ||
           msg.arguments().constFirst().toBool();
}

static bool dbusCallSystemd(const QString &service,
                            const QString &path,
                            const QString &interface,
                            const QDBusConnection &connection,
                            const QString &method,
                            bool needBoolArg)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        return false;
    }

    QDBusMessage msg = dbus.call(method, needBoolArg ? QVariant(true) : QVariant());

    if (!msg.errorName().isEmpty()) {
        qWarning() << "Debus error: " << msg;
    }

    if (msg.arguments().isEmpty() || msg.arguments().constFirst().isNull())
        return true;

    QString response = msg.arguments().constFirst().toString();
    qDebug() << "systemd:" << method << "=" << response;
    return response == QLatin1String("yes") || response == QLatin1String("challenge");
}

bool dbusGetProperty(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString &property)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusGetProperty: QDBusinterface is invalid" << service << path << interface << property;
        return false;
    }

    QDBusMessage msg = dbus.call(QLatin1String("Get"), dbus.interface(), property);

    if (!msg.errorName().isEmpty()) {
        qWarning() << "Dbus error: " << msg;
    }

    return !msg.arguments().isEmpty() &&
            msg.arguments().constFirst().value<QDBusVariant>().variant().toBool();
}

PowerProvider::PowerProvider(QObject *parent) : QObject(parent)
{
}

PowerProvider::~PowerProvider()
{
}

/************************************************
  SystemdProvider

  http://www.freedesktop.org/wiki/Software/systemd/logind
 ************************************************/

SystemdProvider::SystemdProvider(QObject *parent): PowerProvider(parent)
{
}


SystemdProvider::~SystemdProvider()
{
}


bool SystemdProvider::canAction(UkuiPower::Action action) const
{
    QString command;

    switch (action) {
      case UkuiPower::PowerReboot:
        command = QLatin1String("CanReboot");
        break;

      case UkuiPower::PowerShutdown:
        command = QLatin1String("CanPowerOff");
        break;

      case UkuiPower::PowerSuspend:
        command = QLatin1String("CanSuspend");
        break;

      case UkuiPower::PowerHibernate:
        command = QLatin1String("CanHibernate");
        break;

      default:
        return false;
    }

    // canAction should be always silent because it can freeze
    // g_main_context_iteration Qt event loop in QMessageBox
    // on panel startup if there is no DBUS running.
    return dbusCallSystemd(QLatin1String(SYSTEMD_SERVICE),
                           QLatin1String(SYSTEMD_PATH),
                           QLatin1String(SYSTEMD_INTERFACE),
                           QDBusConnection::systemBus(),
                           command,
                           false);
}


bool SystemdProvider::doAction(UkuiPower::Action action)
{
    QString command;

    switch (action)
    {
    case UkuiPower::PowerReboot:
        command = QLatin1String("Reboot");
        break;

    case UkuiPower::PowerShutdown:
        command = QLatin1String("PowerOff");
        break;

    case UkuiPower::PowerSuspend:
        command = QLatin1String("Suspend");
        break;

    case UkuiPower::PowerHibernate:
        command = QLatin1String("Hibernate");
        break;

    default:
        return false;
    }

    return dbusCallSystemd(QLatin1String(SYSTEMD_SERVICE),
                           QLatin1String(SYSTEMD_PATH),
                           QLatin1String(SYSTEMD_INTERFACE),
                           QDBusConnection::systemBus(),
                           command,
                           true);
}


UKUIProvider::UKUIProvider(QObject *parent): PowerProvider (parent)
{
}

UKUIProvider::~UKUIProvider()
{}

bool UKUIProvider::canAction(UkuiPower::Action action) const
{
    QString command;
    switch (action) {
    case UkuiPower::PowerLogout:
        command = QLatin1String("canLogout");
        break;
    case UkuiPower::PowerReboot:
        command = QLatin1String("canReboot");
        break;
    case UkuiPower::PowerShutdown:
        command = QLatin1String("canPowerOff");
        break;
    default:
        return false;
    }

    return dbusCall(QLatin1String(UKUI_SERVICE),
                    QLatin1String(UKUI_PATH),
                    QLatin1String(UKUI_INTERFACE),
                    QDBusConnection::sessionBus(),
                    command);
}

bool UKUIProvider::doAction(UkuiPower::Action action)
{
    QString command;
    switch (action) {
    case UkuiPower::PowerLogout:
        command = QLatin1String("logout");
        break;
    case UkuiPower::PowerReboot:
        command = QLatin1String("reboot");
        break;
    case UkuiPower::PowerShutdown:
        command = QLatin1String("powerOff");
        break;
    default:
        return false;
    }

    return dbusCall(QLatin1String(UKUI_SERVICE),
             QLatin1String(UKUI_PATH),
             QLatin1String(UKUI_INTERFACE),
             QDBusConnection::sessionBus(),
             command);
}


