#include "daemonhelper.h"

#include <Daemon>

using namespace PackageKit;

DaemonHelper::DaemonHelper(QObject *parent) :
    QObject(parent)
{
}

uint DaemonHelper::getTimeSinceLastRefresh()
{
    return Daemon::global()->getTimeSinceAction(Transaction::RoleRefreshCache);
}
