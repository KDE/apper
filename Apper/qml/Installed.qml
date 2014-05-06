import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.packagekit 1.0

TableView {
    TableViewColumn{ role: "roleName"  ; title: "Package" ; width: 100 }
    TableViewColumn{ role: "roleSummary"  ; title: "Summary" ; width: 100 }
    TableViewColumn{ role: "roleVersion"  ; title: "Version" ; width: 100 }
    TableViewColumn{ role: "roleVersion"  ; title: "Version" ; width: 100 }

    model: ApplicationSortFilterModel {
        sourcePkgModel: PackageModel {
            id: pkgModel
        }
    }

    Component.onCompleted: {
        pkgModel.getInstalled()
    }
}
