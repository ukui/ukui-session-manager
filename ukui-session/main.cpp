#include "sessionapplication.h"

int main(int argc, char **argv)
{
    SessionApplication app(argc, argv);

    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
