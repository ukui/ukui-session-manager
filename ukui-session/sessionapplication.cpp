#include "sessionapplication.h"
#include "modulemanager.h"
#include "sessiondbusadaptor.h"

#include <QDebug>

SessionApplication::SessionApplication(int& argc, char** argv) :
    QApplication(argc, argv)
{
    modman = new ModuleManager;

    new SessionDBusAdaptor(modman);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (!dbus.registerService(QStringLiteral("org.ukui.Session")))
    {
        qCritical() << "Can't register org.ukui.Session, there is already a session manager!";
    }
    if (!dbus.registerObject(("/org/ukui/Session"), modman))
    {
        qCritical() << "Can't register object, there is already an object registered at "
                    << "/org/ukui/Session";
    }

    // Wait until the event loop starts
    QTimer::singleShot(0, this, SLOT(startup()));
}

SessionApplication::~SessionApplication()
{
    delete modman;
}

bool SessionApplication::startup()
{
    modman->startup();

    return true;
}
