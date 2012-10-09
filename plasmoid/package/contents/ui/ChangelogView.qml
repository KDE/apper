/*
 * Copyright 2012  Daniel Nicoletti <dantti12@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import org.packagekit 0.1 as PackageKit

Item {
    id: root

    width: parent.width
    height: changelog.paintedHeight

    property string packageID: ""

    PackageKit.Transaction {
        id: transaction
        onPackageUpdateDetails: {
            console.debug("onPackageUpdateDetails: ");
            console.debug(pkgUpdateDetails.packageId);
            changelog.text = pkgUpdateDetails.detail;
        }
    }

    Text {
        id: changelog
        wrapMode: Text.WordWrap
    }

    Component.onCompleted: {
        transaction.getUpdateDetail(packageID);
    }
}
