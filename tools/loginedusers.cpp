#include "loginedusers.h"

QDBusArgument &operator<<(QDBusArgument &argument, const LoginedUsers &mystruct)
{
    argument.beginStructure();
    argument << mystruct.uid << mystruct.userName << mystruct.objpath;//< mystruct.usergroup;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LoginedUsers &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.uid >> mystruct.userName >> mystruct.objpath;// >> mystruct.usergroup;
    argument.endStructure();
    return argument;
}
