#ifndef WINDOW_H
#define WINDOW_H


#include <QMainWindow>
#include <QObject>

class window : public QMainWindow
{
    Q_OBJECT
public:
    window();
    QPixmap pix;

protected:
    void paintEvent(QPaintEvent *e);
};

#endif // WINDOW_H
