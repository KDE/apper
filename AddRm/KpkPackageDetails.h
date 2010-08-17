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
    KpkPackageDetails(const QSharedPointer<PackageKit::Package> &package,
                      const Enum::Roles &roles,
                      QWidget *parent = 0);
    ~KpkPackageDetails();

private slots:
    void description(QSharedPointer<PackageKit::Package> package);
    void files(QSharedPointer<PackageKit::Package> package, const QStringList &files);

    void finished(PackageKit::Enum::Exit status);

    void on_descriptionTB_clicked();
    void on_fileListTB_clicked();
    void on_dependsOnTB_clicked();
    void on_requiredByTB_clicked();

private:
    QSharedPointer<PackageKit::Package> m_package;

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

    void setupSequence(Transaction *transaction,
                       KPixmapSequenceOverlayPainter **sequence,
                       QWidget *widget);
};

#endif
