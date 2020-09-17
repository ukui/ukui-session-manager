#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>

class mainwindow : public QMainWindow
{
    Q_OBJECT
public:
    mainwindow();
    QPixmap pix;

protected:
    void paintEvent(QPaintEvent *e);
};

#endif // MAINWINDOW_H
