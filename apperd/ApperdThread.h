#ifndef APPERDTHREAD_H
#define APPERDTHREAD_H

#include <QThread>

#include <QTimer>
#include <QDBusConnection>
#include <QDateTime>

class ApperdThread : public QObject
{
    Q_OBJECT
public:
    explicit ApperdThread(QObject *parent = 0);
    ~ApperdThread();

private slots:
    void init();
    void poll();
    void configFileChanged();

    void transactionListChanged(const QStringList &tids);
    void updatesChanged();
    void serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);

private:
    void callApperSentinel(const QString &method);
    QDateTime getTimeSinceRefreshCache() const;
    bool nameHasOwner(const QString &name, const QDBusConnection &connection) const;

    bool m_actRefreshCacheChecked;
    bool m_canRefreshCache;
    bool m_sentinelIsRunning;
    QDateTime m_lastRefreshCache;
    uint m_refreshCacheInterval;
    QTimer *m_qtimer;
    QThread *m_thread;
};

#endif // APPERDTHREAD_H
