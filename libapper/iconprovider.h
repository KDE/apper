#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QtQuick/QQuickImageProvider>

class IconProvider : public QQuickImageProvider
{
public:
    IconProvider();

    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // ICONPROVIDER_H
