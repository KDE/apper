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
    KpkPackageDetails(QWidget *parent = 0);
    ~KpkPackageDetails();

    void setPackage(const QSharedPointer<PackageKit::Package> &package,
                    const QModelIndex &index);

    void setDisplayDetails(bool display);

private slots:
    void description(const QSharedPointer<PackageKit::Package> &package);
    void files(QSharedPointer<PackageKit::Package> package, const QStringList &files);

    void finished(PackageKit::Enum::Exit status);

    void on_descriptionTB_clicked();
    void on_fileListTB_clicked();
    void on_dependsOnTB_clicked();
    void on_requiredByTB_clicked();

    void actionActivated(QAction *action);

    void resultJob(KJob *);

    void display();
    void fadeOut();
    void setupDescription();

private:
    QActionGroup *m_actionGroup;
    QSharedPointer<PackageKit::Package> m_package;
    KTemporaryFile *m_tempFile;

    KpkSimplePackageModel *m_pkg_model_dep;
    KpkSimplePackageModel *m_pkg_model_req;

    QStackedLayout *m_viewLayout;
    KTextBrowser   *descriptionKTB;
    QPlainTextEdit *filesPTE;
    QListView      *dependsOnLV;
    QListView      *requiredByLV;

    KPixmapSequenceOverlayPainter *m_busySeqDetails;
    KPixmapSequenceOverlayPainter *m_busySeqFiles;
    KPixmapSequenceOverlayPainter *m_busySeqDepends;
    KPixmapSequenceOverlayPainter *m_busySeqRequires;

    QPropertyAnimation *m_fadeDetails;
    QPropertyAnimation *m_fadeScreenshot;
    bool m_display;

    // We need a copy of prety much every thing
    // we have, so that we update only when we are
    // totaly trasnparent this way the user
    // does not see the ui flicker
    Transaction *m_transaction;
    bool         m_hasDetails;
    QString      m_currentText;
    QPixmap      m_currentIcon;

    // KIO don't get the same file if it store
    QString      m_currentScreenshot;
    QHash<QString, QString> m_screenshotPath;

    void setupSequence(Transaction *transaction,
                       KPixmapSequenceOverlayPainter **sequence,
                       QWidget *widget);
};

#endif
