#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QAbstractNativeEventFilter>
#include <QPushButton>
class PushButton : public QPushButton
{
    Q_OBJECT

public:
    PushButton(QWidget *parent = nullptr);
    ~PushButton();

private:
protected:
    void keyPressEvent(QKeyEvent *event);
    //void enterEvent(QEvent *event);
    //void leaveEvent(QEvent *event);
public:

signals:
};
#endif // PUSHBUTTON_H
