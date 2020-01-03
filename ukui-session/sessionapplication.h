#ifndef SESSIONAPPLICATION_H
#define SESSIONAPPLICATION_H

#include <QApplication>
#include <QFileSystemWatcher>
#include <QSettings>

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
    void settingsChanged(QString path);

private:
    void InitSettings();

    ModuleManager* modman;
    IdleWatcher* mIdleWatcher;
    QFileSystemWatcher *mSettingsWatcher;
    QSettings *mSettings;
    QWidget *widget;
};

#endif // SESSIONAPPLICATION_H
