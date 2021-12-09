#ifndef LOCKCHECKER_H
#define LOCKCHECKER_H

#include <QFile>
#include <QDBusArgument>

struct Inhibitor {
    QString action;
    QString name;
    QString reason;
    QString mode;
    int uid;
    int pid;
};

QDBusArgument &operator<<(QDBusArgument &argument, const Inhibitor &mystruct);

const QDBusArgument &operator>>(const QDBusArgument &argument, Inhibitor &mystruct);

Q_DECLARE_METATYPE(Inhibitor)

class LockChecker
{
public:
    LockChecker();
    ~LockChecker();

public:
    static int checkLock();

    static QStringList getLoginedUsers();

    static QVector<Inhibitor> getInhibitors();

    static bool isSleepBlocked();

    static bool isShutdownBlocked();

    static void getSleepInhibitors(QStringList &sleepInhibitors, QStringList &sleepInhibitorsReason);

    static void getShutdownInhibitors(QStringList &shutdownInhibitors, QStringList &shutdownInhibitorsReason);

    static int getCachedUsers();

    static bool hasMultipleUsers();

private:
    static QString getName(QFile *a);
};

#endif // LOCKCHECKER_H
