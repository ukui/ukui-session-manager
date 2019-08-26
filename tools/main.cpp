#include "toolswindow.h"
#include <QApplication>
#include "powerwindow.h"
#include <QWidget>
#include <QDebug>
#include <QDesktopWidget>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    PowerWindow *pw;
    pw = new PowerWindow(false);
    a.setStyleSheet("QPushButton{"
                    "text-align:center;"
                    "color: rgb(255, 255, 255, 200);"
                    "border: none;"
                    "outline: none;}"
                    "QPushButton::hover{"
                        "background-color: rgb(255, 255, 255, 20%);}"
                    "QPushButton::pressed {"
                        "background-color: rgba(0, 0, 0, 30%);}"
                    "QPushButton#closeButton{"
                        "background:url(resource/dialog_close.png);"
                        "background-repeat:no-repeat}"
                    "QPushButton::hover#closeButton{"
                        "background:url(resource/dialog_close_highlight.png);"
                        "background-repeat:no-repeat}"
                    "QWidget#pwCenterWidget{"
                        "background: rgb(0, 0, 0, 0)}"
                    "QWidget#pw{"
                        "background: rgb(0, 0, 0, 150);}");

    pw->setWindowFlag(Qt::WindowStaysOnTopHint);//设置为顶层窗口，无法被切屏
    pw->showFullScreen();
    pw->move((QApplication::desktop()->width() - pw->width())/2,(QApplication::desktop()->height()- pw->height())/2);

    return a.exec();
}
