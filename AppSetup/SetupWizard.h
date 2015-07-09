/* This file is part of Apper
 *
 * Copyright (C) 2012 Matthias Klumpp <matthias@tenstral.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SetupWizard_H
#define SetupWizard_H

#include <KDialog>

namespace Ui {
    class SetupWizard;
}

class SetupWizardPrivate;

class SetupWizard : public KDialog
{
    Q_OBJECT
public:
    explicit SetupWizard(QWidget *parent = 0);
    virtual ~SetupWizard();

    virtual void slotButtonClicked(int button);

    bool initialize(const QString& ipkFName);

    void setCurrentPage(QWidget *widget);

    SetupWizardPrivate *getPriv() { return d; };

private Q_SLOTS:
    void currentPageChanged(int index);
    void updatePallete();

private:
    bool constructWizardLayout();
    void showError(const QString& details);

    void runInstallation();

    SetupWizardPrivate *const d;
    Ui::SetupWizard *ui;
    QString m_ipkFName;
};

#endif // SetupWizard_H
