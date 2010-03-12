/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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

#include "KpkLicenseAgreement.h"

#include <KLocale>

#include "KpkStrings.h"

KpkLicenseAgreement::KpkLicenseAgreement( PackageKit::Client::EulaInfo info, bool modal, QWidget *parent )
 : KDialog(parent)
{
    setupUi(mainWidget());
    setModal(modal);

    setButtons(KDialog::Cancel | KDialog::Yes);
    setButtonText(KDialog::Yes, i18n("Accept Agreement"));
    setCaption(i18n("License Agreement Required"));
    title->setText(i18n("License required for %1 by %2", info.package->name(), info.vendorName));

    ktextbrowser->setText(info.licenseAgreement);
}

KpkLicenseAgreement::~KpkLicenseAgreement()
{
}

#include "KpkLicenseAgreement.moc"
