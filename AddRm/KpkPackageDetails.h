/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KPKPACKAGEDETAILS_H
#define KPKPACKAGEDETAILS_H

#include <QWidget>
#include <QPackageKit>

#include <KTextBrowser>
#include <QPlainTextEdit>
#include <QListView>

#include "KpkPackageModel.h"

#include "ui_KpkPackageDetails.h"

using namespace PackageKit;

class KpkPackageDetails : public QWidget, Ui::KpkPackageDetails
{
Q_OBJECT
public:
    KpkPackageDetails(PackageKit::Package *package, const Client::Actions &actions, QWidget *parent = 0);
    ~KpkPackageDetails();

private slots:
    void description(PackageKit::Package *package);
    void files(PackageKit::Package *package, const QStringList &files);

    void getDetailsFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void getFilesFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void getDependsFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void getRequiresFinished(PackageKit::Transaction::ExitStatus status, uint runtime);

    void on_descriptionTB_clicked();
    void on_fileListTB_clicked();
    void on_dependsOnTB_clicked();
    void on_requiredByTB_clicked();

private:
    KpkPackageModel *m_pkg_model_dep;
    KpkPackageModel *m_pkg_model_req;

    KTextBrowser   *descriptionKTB;
    QPlainTextEdit *filesPTE;
    QListView      *dependsOnLV;
    QListView      *requiredByLV;
    QWidget        *currentWidget;

    void setCurrentWidget(QWidget *widget);

    void getDetails(PackageKit::Package *p);
    void getFiles(PackageKit::Package *p);
    void getDepends(PackageKit::Package *p);
    void getRequires(PackageKit::Package *p);
};

#endif
