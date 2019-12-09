#include "sessionapplication.h"
#include "modulemanager.h"
#include "sessiondbusadaptor.h"

SessionApplication::SessionApplication(int& argc, char** argv) :
    QApplication(argc, argv)
{
    modman = new ModuleManager;

    new SessionDBusAdaptor(modman);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService(QStringLiteral("org.ukui.Session"));
    dbus.registerObject(("/org/ukui/Session"), modman);

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
