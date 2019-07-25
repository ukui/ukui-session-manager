#ifndef TOOLSWINDOW_H
#define TOOLSWINDOW_H

#include <QWidget>
#include "ukuipower.h"

namespace Ui {
class ToolsWindow;
}

class ToolsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ToolsWindow(QWidget *parent = nullptr);
    ~ToolsWindow();

private slots:
    void on_reboot_button_clicked();

    void on_logout_button_clicked();

    void on_shutdown_button_clicked();

private:
    Ui::ToolsWindow *ui;
    UkuiPower *mPower;
};

#endif // TOOLSWINDOW_H
