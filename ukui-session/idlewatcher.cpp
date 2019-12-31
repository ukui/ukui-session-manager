#include "idlewatcher.h"

#include <KIdleTime>
#include <QDebug>

IdleWatcher::IdleWatcher(QObject *parent) :
    QObject(parent)
{
    qDebug() << "Starting idlewatcher";

    connect(KIdleTime::instance(),
            static_cast<void (KIdleTime::*)(int)>(&KIdleTime::timeoutReached),
            this,
            &IdleWatcher::timeoutReached);

    setup();
}

IdleWatcher::~IdleWatcher()
{
}

void IdleWatcher::setup()
{
    int timeout = 1000 * 600;
    KIdleTime::instance()->addIdleTimeout(timeout);
}

void IdleWatcher::timeoutReached(int identifier)
{
    qDebug() << "Timeout Reached, emit StatusChanged signal!";
    emit StatusChanged(3);
}
