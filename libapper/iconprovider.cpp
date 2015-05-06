#include "iconprovider.h"

#include <QIcon>
#include <QDebug>

IconProvider::IconProvider() :
    QQuickImageProvider(Pixmap)
{
    QIcon::setThemeSearchPaths(QStringList() << "/usr/share/icons");
}

QPixmap IconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QPixmap ret;
    QIcon  icon = QIcon::fromTheme(id);
    if (!icon.isNull()) {
        // Load the requested image, scalling to the
        // requested size
        QSize scaleSize = requestedSize;
        if (scaleSize.width() <= 0 && scaleSize.height() <= 0) {
            if (!icon.availableSizes().isEmpty()) {
                scaleSize = icon.availableSizes().last();
            } else {
                scaleSize = QSize(128, 128);
            }
        } else if (scaleSize.height() == 0) {
            scaleSize.setHeight(scaleSize.width());
        } else if (scaleSize.width() == 0) {
            scaleSize.setWidth(scaleSize.height());
        }

        ret = icon.pixmap(scaleSize);
    }

    if (size) {
        *size = requestedSize;
    }

    return ret;
}
