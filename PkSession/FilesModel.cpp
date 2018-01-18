/***************************************************************************
 *   Copyright (C) 2009-2018 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "FilesModel.h"

#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

#include <QLoggingCategory>
#include <PkIcons.h>
#include <KLocalizedString>
#include <QMimeType>
#include <QMimeDatabase>
#include <KIconLoader>
#include <KService>

Q_DECLARE_LOGGING_CATEGORY(APPER_SESSION)

FilesModel::FilesModel(const QStringList &files, const QStringList &mimes, QObject *parent)
    : QStandardItemModel(parent)
    ,  m_mimes(mimes)
{
    if (!files.isEmpty()) {
        QList<QUrl> urls;
        for (const QString &file : files) {
            urls << QUrl(file);
        }
        insertFiles(urls);
    } else if (!mimes.isEmpty()) {
        QMimeDatabase db;
        // we are searching for mimetypes
        for (const QString &mimeName : mimes) {
            QMimeType mime = db.mimeTypeForName(mimeName);
            if (mime.isValid()) {
                auto item = new QStandardItem(mimeName);
                item->setData(mimeName);
                item->setIcon(KIconLoader::global()->loadMimeTypeIcon(mime.iconName(),
                                                                      KIconLoader::Desktop));
                appendRow(item);
            }
        }
    }
}

bool FilesModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    bool ret = false;
    if (data->hasUrls()) {
        ret = insertFiles(data->urls());
    }
    return ret;
}

bool FilesModel::insertFiles(const QList<QUrl> &urls)
{
    bool ret = false;
    for (const QUrl &url : urls) {
        const QString path = QUrl::fromPercentEncoding(url.path().toLatin1());
        if (files().contains(path)) {
            continue;
        }

        QFileInfo fileInfo(path);
        QStandardItem *item = 0;
        if (fileInfo.isFile()) {
            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForFile(path, QMimeDatabase::MatchContent);
            qCDebug(APPER_SESSION) << url << mime.name();
            for (const QString &mimeType : qAsConst(m_mimes)) {
                if (mime.name() == mimeType) {
                    ret = true;
                    qCDebug(APPER_SESSION) << "Found Supported Mime" << mimeType << mime.iconName();
                    item = new QStandardItem(fileInfo.fileName());
                    item->setData(path);
                    item->setToolTip(path);
                    item->setIcon(KIconLoader::global()->loadMimeTypeIcon(mime.iconName(),
                                                                          KIconLoader::Desktop));
                    break;
                }
            }

            if (ret == false && m_mimes.isEmpty()) {
                if (mime.name() == QLatin1String("application/x-desktop")) {
                    auto service = new KService(path);
                    item = new QStandardItem(service->name());
                    item->setData(true, Qt::UserRole);
                    item->setIcon(KIconLoader::global()->loadMimeTypeIcon(service->icon(),
                                                                          KIconLoader::Desktop));
                } else {
                    item = new QStandardItem(fileInfo.fileName());
                    item->setIcon(KIconLoader::global()->loadMimeTypeIcon(mime.iconName(),
                                                                        KIconLoader::Desktop));
                }
                item->setData(path);
                item->setToolTip(path);
            } else if (ret == false && !m_mimes.isEmpty()) {
                item = new QStandardItem(fileInfo.fileName());
                item->setData(path);
                item->setToolTip(path);
                item->setEnabled(false);
                item->setIcon(KIconLoader::global()->loadIcon(QLatin1String("dialog-cancel"), KIconLoader::Desktop));
            }
        } else if (m_mimes.isEmpty()) {
            // It's not a file but we don't have a mime so it's ok
            item = new QStandardItem(fileInfo.fileName());
            item->setData(path);
            item->setToolTip(path);
            item->setIcon(KIconLoader::global()->loadIcon(QLatin1String("unknown"), KIconLoader::Desktop));
        }

        if (item) {
            appendRow(item);
        }
    }
    return ret;
}

QStringList FilesModel::mimeTypes() const
{
    return { QStringLiteral("text/uri-list") };
}

Qt::DropActions FilesModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::ItemFlags FilesModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

QStringList FilesModel::files() const
{
    QStringList ret;
    for (int i = 0; i < rowCount(); ++i) {
        if (item(i)->isEnabled()) {
            ret << item(i)->data().toString();
        }
    }
    return ret;
}

bool FilesModel::onlyApplications() const
{
    for (int i = 0; i < rowCount(); ++i) {
        // UserRole is true if it is an application
        if (item(i)->isEnabled() == true &&
            item(i)->data(Qt::UserRole).toBool() == false) {
            return false; // there is something that is not an app
        }
    }
    return true;
}

#include "moc_FilesModel.cpp"
