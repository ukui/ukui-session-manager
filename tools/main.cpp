#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QDesktopWidget>

#include "powerwin.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    PowerWin *pw;
    pw = new PowerWin(false);

    //加载qss文件
    QFile qss(":/powerwin.qss");
    qss.open(QFile::ReadOnly);
    a.setStyleSheet(qss.readAll());
    qss.close();

    QRect screen = QApplication::desktop()->screenGeometry(QCursor::pos());
    int xx = screen.x();
    int yy = screen.y();//取得当前鼠标所在屏幕的最左，上坐标。

    pw->setWindowFlag(Qt::WindowStaysOnTopHint);//设置为顶层窗口，无法被切屏
    pw->showFullScreen();
    pw->move(xx+(screen.width() - pw->width())/2,yy+(screen.height()- pw->height())/2);

    return a.exec();
}
