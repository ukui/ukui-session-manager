#include "sessionapplication.h"

int main(int argc, char **argv)
{
    SessionApplication app(argc, argv);

    return app.exec();
}