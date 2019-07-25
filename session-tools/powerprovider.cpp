#include "powerprovider.h"

#include <QDBusInterface>
#include <QDebug>

#define UPOWER_SERVICE        "org.freedesktop.UPOWER"
#define UPOWER_PATH           "/org/freedesktop/UPower"
#define UPOWER_INTERFACE      UPOWER_SERVICE

#define CONSOLEKIT_SERVICE      "org.freedesktop.ConsoleKit"
#define CONSOLEKIT_PATH         "/org/freedesktop/ConsoleKit/Manager"
#define CONSOLEKIT_INTERFACE    "org.freedesktop.ConsoleKit.Manager"

#define SYSTEMD_SERVICE         "org.freedesktop.login1"
#define SYSTEMD_PATH            "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE       "org.freedesktop.login1.Manager"

#define UKUI_SERVICE      "org.ukui.session"
#define UKUI_PATH         "/UkuiSession"
#define UKUI_INTERFACE    "org.ukui.session"

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

UPowerProvider::UPowerProvider(QObject *parent) : PowerProvider(parent)
{
}

UPowerProvider::~UPowerProvider()
{
}

bool UPowerProvider::canAction(UkuiPower::Action action) const
{
    QString command;
    QString property;
    switch (action) {
      case UkuiPower::PowerHibernate:
        property = QLatin1String("CanHihernate");
        command = QLatin1String("HibernateAllowed");
        break;
      case UkuiPower::PowerSuspend:
        property = QLatin1String("CanSuspend");
        command = QLatin1String("SuspendAllowed");
        break;
      default:
        return false;
    }

    return dbusGetProperty(QLatin1String(UPOWER_SERVICE),
                           QLatin1String(UPOWER_PATH),
                           QLatin1String(PROPERTIES_INTERFACE),
                           QDBusConnection::systemBus(),
                           property)
            &&
            dbusCall(QLatin1String(UPOWER_SERVICE),
                     QLatin1String(UPOWER_PATH),
                     QLatin1String(UPOWER_INTERFACE),
                     QDBusConnection::systemBus(),
                     command);
}

bool UPowerProvider::doAction(UkuiPower::Action action)
{
    QString command;

    switch (action) {
      case UkuiPower::PowerHibernate:
        command = QLatin1String("Hibernate");
        break;
      case UkuiPower::PowerSuspend:
        command = QLatin1String("Suspend");
        break;

      default:
        return false;
    }

    return dbusCall(QLatin1String(UPOWER_SERVICE),
                    QLatin1String(UPOWER_PATH),
                    QLatin1String(UPOWER_INTERFACE),
                    QDBusConnection::systemBus(),
                    command);
}

ConsoleKitProvider::ConsoleKitProvider(QObject *parent): PowerProvider(parent)
{
}


ConsoleKitProvider::~ConsoleKitProvider()
{
}


bool ConsoleKitProvider::canAction(UkuiPower::Action action) const
{
    QString command;
    switch (action) {
      case UkuiPower::PowerReboot:
        command = QLatin1String("CanReboot");
        break;

      case UkuiPower::PowerShutdown:
        command = QLatin1String("CanPowerOff");
        break;

      case UkuiPower::PowerHibernate:
        command  = QLatin1String("CanHibernate");
        break;

      case UkuiPower::PowerSuspend:
        command  = QLatin1String("CanSuspend");
        break;

      default:
        return false;
    }

    // canAction should be always silent because it can freeze
    // g_main_context_iteration Qt event loop in QMessageBox
    // on panel startup if there is no DBUS running.
    return dbusCallSystemd(QLatin1String(CONSOLEKIT_SERVICE),
                           QLatin1String(CONSOLEKIT_PATH),
                           QLatin1String(CONSOLEKIT_INTERFACE),
                           QDBusConnection::systemBus(),
                           command,
                           false);
}


bool ConsoleKitProvider::doAction(UkuiPower::Action action)
{
    QString command;
    switch (action) {
      case UkuiPower::PowerReboot:
        command = QLatin1String("Reboot");
        break;

      case UkuiPower::PowerShutdown:
        command = QLatin1String("PowerOff");
        break;

      case UkuiPower::PowerHibernate:
        command = QLatin1String("Hibernate");
        break;

      case UkuiPower::PowerSuspend:
        command = QLatin1String("Suspend");
        break;

      default:
        return false;
    }

    return dbusCallSystemd(QLatin1String(CONSOLEKIT_SERVICE),
                           QLatin1String(CONSOLEKIT_PATH),
                           QLatin1String(CONSOLEKIT_INTERFACE),
                           QDBusConnection::systemBus(),
                           command,
                           true);
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







