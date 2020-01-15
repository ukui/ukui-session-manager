#include "idlewatcher.h"

#include <KIdleTime>
#include <QDebug>

IdleWatcher::IdleWatcher(int secs, QObject *parent) :
    QObject(parent),
    mSecs(secs)
{
    connect(KIdleTime::instance(),
            static_cast<void (KIdleTime::*)(int)>(&KIdleTime::timeoutReached),
            this,
            &IdleWatcher::timeoutReached);

    setup();
}

IdleWatcher::~IdleWatcher()
{
    KIdleTime::instance()->removeAllIdleTimeouts();
}

void IdleWatcher::setup()
{
    KIdleTime::instance()->addIdleTimeout(1000 * mSecs);
}

void IdleWatcher::timeoutReached(int identifier)
{
    qDebug() << "Timeout Reached, emit StatusChanged signal!";
    emit StatusChanged(3);
}

void IdleWatcher::reset(int timeout)
{
    qDebug() << "Idle timeout reset to " << timeout;
    KIdleTime::instance()->removeAllIdleTimeouts();
    mSecs = timeout;
    setup();
}
