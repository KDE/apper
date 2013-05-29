#ifndef DAEMONHELPER_H
#define DAEMONHELPER_H

#include <QObject>

class DaemonHelper : public QObject
{
    Q_OBJECT
public:
    explicit DaemonHelper(QObject *parent = 0);

public slots:
    uint getTimeSinceLastRefresh();
};

#endif // DAEMONHELPER_H
