#ifndef SESSIONAPPLICATION_H
#define SESSIONAPPLICATION_H

#include <QApplication>
//#include <QGSettings/QGSettings>
//#include <QTimer>

class ModuleManager;

class SessionApplication : public QApplication
{
    Q_OBJECT
public:
    SessionApplication(int& argc, char** argv);
    ~SessionApplication();

//    bool notify(QObject*, QEvent *);
//    void updateTimemonitor();
//    void getidledelay();
//    QTimer       *idlemonitor;
//    QGSettings   *settings;
//    int           idledelay;

//public slots:
//    void updatekey(const QString &key);
//    void onTimeout();

private Q_SLOTS:
    bool startup();

private:
    ModuleManager* modman;
    QWidget *widget;
};

#endif // SESSIONAPPLICATION_H
