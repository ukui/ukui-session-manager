#ifndef IDLEWATCHER_H
#define IDLEWATCHER_H

#include <QObject>

//class ScreenSaver;

class IdleWatcher : public QObject
{
    Q_OBJECT
public:
    explicit IdleWatcher(int secs, QObject *parent = nullptr);
    virtual ~IdleWatcher();

    void reset(int timeout);

private slots:
    void timeoutReached(int identifier);
    void setup();

signals:
    void StatusChanged(uint status);

private:
    int mSecs;

};

#endif // IDLEWATCHER_H
