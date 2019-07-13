#include "sessionapplication.h"
#include "modulemanager.h"

#include <QTimer>

SessionApplication::SessionApplication(int& argc, char** argv) :
    QApplication(argc, argv)
{
    modman = new ModuleManager;

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
