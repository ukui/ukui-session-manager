#ifndef PW_H
#define PW_H
#include <QWidget>
#include <QPixmap>

class QPushButton;

class PW : public QWidget
{
private:
    void initUI();

private:
    int dialogWidth;
    int dialogHeight;

    QWidget         *m_dialog;
    QWidget         *m_centerWidget;
    QPushButton     *m_closeButton;

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

public:
    PW();
    void setDialogSize(int w, int h);
    QWidget *centerWidget();
    QWidget *dialog();
    QPixmap pixmap;
};

#endif // PW_H
