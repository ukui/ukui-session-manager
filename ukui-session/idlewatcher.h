#ifndef IDLEWATCHER_H
#define IDLEWATCHER_H

#include <QObject>

//class ScreenSaver;

class IdleWatcher : public QObject
{
    Q_OBJECT
public:
    explicit IdleWatcher(QObject *parent = nullptr);
    virtual ~IdleWatcher();

private slots:
    void timeoutReached(int identifier);
    void setup();

signals:
    void StatusChanged(uint status);
private:
//    ScreenSaver mScreensaver;

};

#endif // IDLEWATCHER_H
