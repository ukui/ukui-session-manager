#include "toolswindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ToolsWindow* T = new ToolsWindow();
    T->showFullScreen();

    return a.exec();
}
