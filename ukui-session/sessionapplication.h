#ifndef SESSIONAPPLICATION_H
#define SESSIONAPPLICATION_H

#include <QApplication>

class ModuleManager;
class IdleWatcher;

class SessionApplication : public QApplication
{
    Q_OBJECT
public:
    SessionApplication(int& argc, char** argv);
    ~SessionApplication();

private Q_SLOTS:
    bool startup();

private:
    ModuleManager* modman;
    IdleWatcher* mIdleWatcher;
    QWidget *widget;
};

#endif // SESSIONAPPLICATION_H
