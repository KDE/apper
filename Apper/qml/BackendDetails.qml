import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1

import PackageKit 1.0 as Pk

ApplicationWindow {
    visible: true
    width: 500
    height: 600
    title: qsTr("Backend Details")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        GroupBox {
            Layout.fillWidth: true
            title: qsTr("About PackageKit")
            GridLayout {
                Label {
                    text: qsTr("Version:")
                }
                Label {
                    text: Pk.Daemon.versionMajor + "." +  Pk.Daemon.versionMinor + "." +  Pk.Daemon.versionMicro
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("About the Package Manager Integration")
            GridLayout {
                columns: 2
                Label {
                    text: qsTr("Backend name:")
                }
                Label {
                    text: Pk.Daemon.backendName
                }

                Label {
                    text: qsTr("Backend description:")
                }
                Label {
                    text: Pk.Daemon.backendDescription
                }

                Label {
                    text: qsTr("Backend author:")
                }
                Label {
                    text: Pk.Daemon.backendAuthor
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Supported Methods")
            GridLayout {
                columns: 3
                CheckBox{
                    text: qsTr("Accept EULA")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleAcceptEula)
                }

                CheckBox{
                    text: qsTr("Cancel transaction")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleCancel)
                }

                CheckBox{
                    text: qsTr("Depends on")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleDependsOn)
                }
                CheckBox{
                    text: qsTr("Download packages")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleDownloadPackages)
                }

                CheckBox{
                    text: qsTr("Get categories")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetCategories)
                }
                CheckBox{
                    text: qsTr("Get details")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetDetails)
                }
                CheckBox{
                    text: qsTr("Get details local")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetDetailsLocal)
                }
                CheckBox{
                    text: qsTr("Get distro upgrades")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetDistroUpgrades)
                }
                CheckBox{
                    text: qsTr("Get files")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetFiles)
                }
                CheckBox{
                    text: qsTr("Get files local")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetFilesLocal)
                }
                CheckBox{
                    text: qsTr("Get old transactions")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetOldTransactions)
                }
                CheckBox{
                    text: qsTr("Get packages")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetPackages)
                }
                CheckBox{
                    text: qsTr("Get repository list")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetRepoList)
                }
                CheckBox{
                    text: qsTr("Get update detail")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetUpdateDetail)
                }
                CheckBox{
                    text: qsTr("Get updates")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleGetUpdates)
                }

                CheckBox{
                    text: qsTr("Install files")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleInstallFiles)
                }
                CheckBox{
                    text: qsTr("Install packages")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleInstallPackages)
                }
                CheckBox{
                    text: qsTr("Install signature")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleInstallSignature)
                }

                CheckBox{
                    text: qsTr("Refresh cache")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleRefreshCache)
                }
                CheckBox{
                    text: qsTr("Remove packages")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleRemovePackages)
                }
                CheckBox{
                    text: qsTr("Repair system")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleRepairSystem)
                }
                CheckBox{
                    text: qsTr("Enable repository")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleRepoEnable)
                }
                CheckBox{
                    text: qsTr("Remove repository")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleRepoRemove)
                }
                CheckBox{
                    text: qsTr("Repository set data")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleRepoSetData)
                }
                CheckBox{
                    text: qsTr("Required by")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleRequiredBy)
                }
                CheckBox{
                    text: qsTr("Resolve package name")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleResolve)
                }

                CheckBox{
                    text: qsTr("Search details")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleSearchDetails)
                }
                CheckBox{
                    text: qsTr("Search file")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleSearchFile)
                }
                CheckBox{
                    text: qsTr("Search group")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleSearchGroup)
                }
                CheckBox{
                    text: qsTr("Search name")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleSearchName)
                }

                CheckBox{
                    text: qsTr("Update packages")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleUpdatePackages)
                }

                CheckBox{
                    text: qsTr("What provides")
                    enabled: false
                    checked: PkHelper.supportRole(Pk.Daemon.roles, Pk.Transaction.RoleWhatProvides)
                }

            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Supported Query Filters")
            GridLayout {
                columns: 4
                CheckBox{
                    text: qsTr("Application")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterApplication)
                }

                CheckBox{
                    text: qsTr("Architecture")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterArch)
                }

                CheckBox{
                    text: qsTr("Basename")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterBasename)
                }

                CheckBox{
                    text: qsTr("Collections")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterCollections)
                }

                CheckBox{
                    text: qsTr("Development")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterDevel)
                }

                CheckBox{
                    text: qsTr("Free")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterFree)
                }

                CheckBox{
                    text: qsTr("Graphical")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterGui)
                }

                CheckBox{
                    text: qsTr("Installed")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterInstalled)
                }

                CheckBox{
                    text: qsTr("Newest")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterNewest)
                }

                CheckBox{
                    text: qsTr("Source")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterSource)
                }

                CheckBox{
                    text: qsTr("Supported")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterSupported)
                }

                CheckBox{
                    text: qsTr("Visible")
                    enabled: false
                    checked: PkHelper.supportFilter(Pk.Daemon.filters, Pk.Transaction.FilterVisible)
                }
            }
        }

        Item { Layout.fillHeight: true }

        Button {
            Layout.alignment: Qt.AlignRight
            text: qsTr("Close")
            onClicked: Qt.quit()
        }

    }
}
