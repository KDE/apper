/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include <KDebug>
#include <PkIcons.h>
#include <KLocale>
#include <KMimeType>
#include <KIconLoader>
#include <KService>

FilesModel::FilesModel(const QStringList &files, const QStringList &mimes, QObject *parent)
: QStandardItemModel(parent),
  m_mimes(mimes)
{
    if (!files.isEmpty()) {
        QList<QUrl> urls;
        foreach (const QString &file, files) {
            urls << file;
        }
        insertFiles(urls);
    } else if (!mimes.isEmpty()) {
        // we are searching for mimetypes
        foreach (const QString &mime, mimes) {
            KMimeType::Ptr mimePtr = KMimeType::mimeType(mime);
            // Make sure we have a pointer
            if (mimePtr && mimePtr->isValid()) {
                QStandardItem *item = new QStandardItem(mime);
                item->setData(mime);
                item->setIcon(KIconLoader::global()->loadMimeTypeIcon(mimePtr->iconName(),
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
    foreach (const QUrl &url, urls) {
        if (files().contains(url.path())) {
            continue;
        }

        QFileInfo fileInfo(url.path());
        QStandardItem *item = 0;
        if (fileInfo.isFile()) {
            KMimeType::Ptr mime = KMimeType::findByFileContent(url.path());
            kDebug() << url << mime->name();
            foreach (const QString &mimeType, m_mimes) {
                if (mime->is(mimeType)) {
                    ret = true;
/*                    kDebug() << "Found Supported Mime" << mimeType << mime->iconName();*/
                    item = new QStandardItem(fileInfo.fileName());
                    item->setData(url.path());
                    item->setToolTip(url.path());
                    item->setIcon(KIconLoader::global()->loadMimeTypeIcon(mime->iconName(),
                                                                        KIconLoader::Desktop));
                    break;
                }
            }

            if (ret == false && m_mimes.isEmpty()) {
                if (mime->name() == "application/x-desktop") {
                    KService *service = new KService(url.path());
                    item = new QStandardItem(service->name());
                    item->setData(true, Qt::UserRole);
                    item->setIcon(KIconLoader::global()->loadMimeTypeIcon(service->icon(),
                                                                          KIconLoader::Desktop));
                } else {
                    item = new QStandardItem(fileInfo.fileName());
                    item->setIcon(KIconLoader::global()->loadMimeTypeIcon(mime->iconName(),
                                                                        KIconLoader::Desktop));
                }
                item->setData(url.path());
                item->setToolTip(url.path());
            } else if (ret == false && !m_mimes.isEmpty()) {
                item = new QStandardItem(fileInfo.fileName());
                item->setData(url.path());
                item->setToolTip(url.path());
                item->setEnabled(false);
                item->setIcon(KIconLoader::global()->loadIcon("dialog-cancel", KIconLoader::Desktop));
            }
        } else if (m_mimes.isEmpty()) {
            // It's not a file but we don't have a mime so it's ok
            item = new QStandardItem(fileInfo.fileName());
            item->setData(url.path());
            item->setToolTip(url.path());
            item->setIcon(KIconLoader::global()->loadIcon("unknown", KIconLoader::Desktop));
        }

        if (item) {
            appendRow(item);
        }
    }
    return ret;
}

QStringList FilesModel::mimeTypes() const
{
    return QStringList() << "text/uri-list";
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

#include "FilesModel.moc"
