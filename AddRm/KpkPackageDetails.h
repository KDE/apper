/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
 *   dantti85-pk@yahoo.com.br                                              *
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

#ifndef KPK_PACKAGE_DETAILS_H
#define KPK_PACKAGE_DETAILS_H

#include <QPackageKit>

#include <KTextBrowser>
#include <KPixmapSequenceOverlayPainter>
#include <KTemporaryFile>
#include <KJob>

#include <QWidget>
#include <QPlainTextEdit>
#include <QListView>
#include <QStackedLayout>

#include "ui_KpkPackageDetails.h"

using namespace PackageKit;

class KpkSimplePackageModel;
class KpkPackageDetails : public QWidget, Ui::KpkPackageDetails
{
Q_OBJECT
public:
    enum FadeWidget {
        FadeNone       = 0x0,
        FadeStacked    = 0x1,
        FadeScreenshot = 0x2
    };
    Q_DECLARE_FLAGS(FadeWidgets, FadeWidget)
    KpkPackageDetails(QWidget *parent = 0);
    ~KpkPackageDetails();

    void setPackage(const QModelIndex &index);
    void setDisplayDetails(bool display);

private slots:
    void actionActivated(QAction *action);
    void description(const QSharedPointer<PackageKit::Package> &package);
    void files(QSharedPointer<PackageKit::Package> package, const QStringList &files);
    void finished();
    void resultJob(KJob *);

    void display();

private:
    void fadeOut(FadeWidgets widgets);
    void setupDescription();
    QVector<QPair<QString, QString> > locateApplication(const QString &_relPath, const QString &menuId) const;

    QActionGroup *m_actionGroup;
    QSharedPointer<PackageKit::Package> m_package;
    KTemporaryFile *m_tempFile;

    QStackedLayout *m_viewLayout;
    KTextBrowser   *descriptionKTB;
    QPlainTextEdit *filesPTE;
    QListView      *dependsOnLV;
    QListView      *requiredByLV;

    KPixmapSequenceOverlayPainter *m_busySeq;

    QPropertyAnimation *m_fadeStacked;
    QPropertyAnimation *m_fadeScreenshot;
    bool m_display;

    // We need a copy of prety much every thing
    // we have, so that we update only when we are
    // totaly transparent this way the user
    // does not see the ui flicker
    Transaction *m_transaction;
    bool         m_hasDetails;
    QString      m_currentText;
    QPixmap      m_currentIcon;
    QString      m_appId;

    // file list buffer
    bool         m_hasFileList;
    QStringList  m_currentFileList;

    // GetDepends buffer
    bool m_hasDepends;
    KpkSimplePackageModel *m_dependsModel;

    // GetRequires buffer
    bool m_hasRequires;
    KpkSimplePackageModel *m_requiresModel;

    // Screen shot buffer
    QString      m_currentScreenshot;
    QHash<QString, QString> m_screenshotPath;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KpkPackageDetails::FadeWidgets)

#endif
