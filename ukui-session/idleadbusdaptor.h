#ifndef IDLEDBUSADAPTOR_H
#define IDLEDBUSADAPTOR_H

#include <QtDBus>
#include "idlewatcher.h"

class IdleDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.gnome.SessionManager.Presence")

public:
    IdleDBusAdaptor(IdleWatcher *mIdleWatch)
        : QDBusAbstractAdaptor(mIdleWatch)
    {
        connect(mIdleWatch, SIGNAL(StatusChanged(uint)), SIGNAL(StatusChanged(uint)));
    }

signals:
    void StatusChanged(uint status);

private:
    IdleWatcher *mIdleWatch;
};

#endif // IDLEDBUSADAPTOR_H
