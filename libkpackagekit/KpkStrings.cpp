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

#include "KpkStrings.h"

#include <KLocale>

#include <KDebug>

QString KpkStrings::status(PackageKit::Transaction::Status status)
{
    switch (status) {
    case Transaction::UnknownStatus :
        return i18nc("The transaction state", "Unknown state");
    case Transaction::Setup :
        return i18nc("The transaction state", "Waiting for service to start");
    case Transaction::Wait :
        return i18nc("The transaction state", "Waiting for other tasks");
    case Transaction::Running :
        return i18nc("The transaction state", "Running task");
    case Transaction::Query :
        return i18nc("The transaction state", "Querying");
    case Transaction::Info :
        return i18nc("The transaction state", "Getting information");
    case Transaction::Remove :
        return i18nc("The transaction state", "Removing packages");
    case Transaction::Download :
        return i18nc("The transaction state", "Downloading packages");
    case Transaction::Install :
        return i18nc("The transaction state", "Installing packages");
    case Transaction::RefreshCache :
        return i18nc("The transaction state", "Refreshing software list");
    case Transaction::Update :
        return i18nc("The transaction state", "Updating packages");
    case Transaction::Cleanup :
        return i18nc("The transaction state", "Cleaning up packages");
    case Transaction::Obsolete :
        return i18nc("The transaction state", "Obsoleting packages");
    case Transaction::DepResolve :
        return i18nc("The transaction state", "Resolving dependencies");
    case Transaction::SigCheck :
        return i18nc("The transaction state", "Checking signatures");
    case Transaction::Rollback :
        return i18nc("The transaction state", "Rolling back");
    case Transaction::TestCommit :
        return i18nc("The transaction state", "Testing changes");
    case Transaction::Commit :
        return i18nc("The transaction state", "Committing changes");
    case Transaction::Request :
        return i18nc("The transaction state", "Requesting data");
    case Transaction::Finished :
        return i18nc("The transaction state", "Finished");
    case Transaction::Cancel :
        return i18nc("The transaction state", "Cancelling");
    case Transaction::DownloadRepository :
        return i18nc("The transaction state", "Downloading repository information");
    case Transaction::DownloadPackagelist :
        return i18nc("The transaction state", "Downloading list of packages");
    case Transaction::DownloadFilelist :
        return i18nc("The transaction state", "Downloading file lists");
    case Transaction::DownloadChangelog :
        return i18nc("The transaction state", "Downloading lists of changes");
    case Transaction::DownloadGroup :
        return i18nc("The transaction state", "Downloading groups");
    case Transaction::DownloadUpdateinfo :
        return i18nc("The transaction state", "Downloading update information");
    case Transaction::Repackaging :
        return i18nc("The transaction state", "Repackaging files");
    case Transaction::LoadingCache :
        return i18nc("The transaction state", "Loading cache");
    case Transaction::ScanApplications :
        return i18nc("The transaction state", "Scanning installed applications");
    case Transaction::GeneratePackageList :
        return i18nc("The transaction state", "Generating package lists");
//  case Transaction::WaitingForLock :
//      return i18nc("The transaction state", "Waiting for package manager lock");
    }
    kDebug() << "status unrecognised: " << status;
    return QString();
}

QString KpkStrings::statusPast(PackageKit::Transaction::Status status)
{
    switch (status) {
    case Transaction::Download:
        return i18nc("The action of the package, in past tense", "Downloaded");
    case Transaction::Update:
        return i18nc("The action of the package, in past tense", "Updated");
    case Transaction::Install:
        return i18nc("The action of the package, in past tense", "Installed");
    case Transaction::Remove:
        return i18nc("The action of the package, in past tense", "Removed");
    case Transaction::Cleanup:
        return i18nc("The action of the package, in past tense", "Cleaned Up");
    case Transaction::Obsolete:
        return i18nc("The action of the package, in past tense", "Obsoleted");
    default : // In this case we don't want to map all enums
        kDebug() << "status unrecognised: " << status;
        return QString();
    }
}

QString KpkStrings::action(Client::Action action)
{
    switch (action) {
    case Client::UnkownAction :
        return i18nc("The role of the transaction, in present tense", "Unknown role type");
    case Client::ActionGetDepends :
        return i18nc("The role of the transaction, in present tense", "Getting dependencies");
    case Client::ActionGetUpdateDetail :
        return i18nc("The role of the transaction, in present tense", "Getting update detail");
    case Client::ActionGetDetails :
        return i18nc("The role of the transaction, in present tense", "Getting details");
    case Client::ActionGetRequires :
        return i18nc("The role of the transaction, in present tense", "Getting requires");
    case Client::ActionGetUpdates :
        return i18nc("The role of the transaction, in present tense", "Getting updates");
    case Client::ActionSearchDetails :
        return i18nc("The role of the transaction, in present tense", "Searching details");
    case Client::ActionSearchFile :
        return i18nc("The role of the transaction, in present tense", "Searching for file");
    case Client::ActionSearchGroup :
        return i18nc("The role of the transaction, in present tense", "Searching groups");
    case Client::ActionSearchName :
        return i18nc("The role of the transaction, in present tense", "Searching by package name");
    case Client::ActionServicePack :
        return i18nc("The role of the transaction, in present tense", "Applying service pack");
    case Client::ActionRemovePackages :
        return i18nc("The role of the transaction, in present tense", "Removing");
    case Client::ActionInstallPackages :
        return i18nc("The role of the transaction, in present tense", "Installing");
    case Client::ActionInstallFiles :
        return i18nc("The role of the transaction, in present tense", "Installing file");
    case Client::ActionRefreshCache :
        return i18nc("The role of the transaction, in present tense", "Refreshing package cache");
    case Client::ActionUpdatePackages :
        return i18nc("The role of the transaction, in present tense", "Updating packages");
    case Client::ActionUpdateSystem :
        return i18nc("The role of the transaction, in present tense", "Updating system");
    case Client::ActionCancel :
        return i18nc("The role of the transaction, in present tense", "Canceling");
    case Client::ActionRollback :
        return i18nc("The role of the transaction, in present tense", "Rolling back");
    case Client::ActionGetRepoList :
        return i18nc("The role of the transaction, in present tense", "Getting list of repositories");
    case Client::ActionRepoEnable :
        return i18nc("The role of the transaction, in present tense", "Enabling repository");
    case Client::ActionRepoSetData :
        return i18nc("The role of the transaction, in present tense", "Setting repository data");
    case Client::ActionResolve :
        return i18nc("The role of the transaction, in present tense", "Resolving");
    case Client::ActionGetFiles :
        return i18nc("The role of the transaction, in present tense", "Getting file list");
    case Client::ActionWhatProvides :
        return i18nc("The role of the transaction, in present tense", "Getting what provides");
    case Client::ActionInstallSignature :
        return i18nc("The role of the transaction, in present tense", "Installing signature");
    case Client::ActionGetPackages :
        return i18nc("The role of the transaction, in present tense", "Getting package lists");
    case Client::ActionAcceptEula :
        return i18nc("The role of the transaction, in present tense", "Accepting EULA");
    case Client::ActionDownloadPackages :
        return i18nc("The role of the transaction, in present tense", "Downloading packages");
    case Client::ActionGetDistroUpgrades :
        return i18nc("The role of the transaction, in present tense", "Getting distribution upgrade information");
    case Client::ActionGetCategories :
        return i18nc("The role of the transaction, in present tense", "Getting categories");
//  case Client::ActionGetOldTransactions :
//      return i18nc("The role of the transaction, in present tense", "Getting old transactions");
    }
    kDebug() << "action unrecognised: " << action;
    return QString();
}

QString KpkStrings::actionPast(Client::Action action)
{
    switch (action) {
    case Client::UnkownAction :
        return i18nc("The role of the transaction, in past tense", "Unknown role type");
    case Client::ActionGetDepends :
        return i18nc("The role of the transaction, in past tense", "Got dependencies");
    case Client::ActionGetUpdateDetail :
        return i18nc("The role of the transaction, in past tense", "Got update detail");
    case Client::ActionGetDetails :
        return i18nc("The role of the transaction, in past tense", "Got details");
    case Client::ActionGetRequires :
        return i18nc("The role of the transaction, in past tense", "Got requires");
    case Client::ActionGetUpdates :
        return i18nc("The role of the transaction, in past tense", "Got updates");
    case Client::ActionSearchDetails :
        return i18nc("The role of the transaction, in past tense", "Searched for package details");
    case Client::ActionSearchFile :
        return i18nc("The role of the transaction, in past tense", "Searched for file");
    case Client::ActionSearchGroup :
        return i18nc("The role of the transaction, in past tense", "Searched groups");
    case Client::ActionSearchName :
        return i18nc("The role of the transaction, in past tense", "Searched for package name");
    case Client::ActionServicePack :
        return i18nc("The role of the transaction, in past tense", "Applied service pack");
    case Client::ActionRemovePackages :
        return i18nc("The role of the transaction, in past tense", "Removed packages");
    case Client::ActionInstallPackages :
        return i18nc("The role of the transaction, in past tense", "Installed packages");
    case Client::ActionInstallFiles :
        return i18nc("The role of the transaction, in past tense", "Installed local files");
    case Client::ActionRefreshCache :
        return i18nc("The role of the transaction, in past tense", "Refreshed package cache");
    case Client::ActionUpdatePackages :
        return i18nc("The role of the transaction, in past tense", "Updated packages");
    case Client::ActionUpdateSystem :
        return i18nc("The role of the transaction, in past tense", "Updated system");
    case Client::ActionCancel :
        return i18nc("The role of the transaction, in past tense", "Canceled");
    case Client::ActionRollback :
        return i18nc("The role of the transaction, in past tense", "Rolled back");
    case Client::ActionGetRepoList :
        return i18nc("The role of the transaction, in past tense", "Got list of repositories");
    case Client::ActionRepoEnable :
        return i18nc("The role of the transaction, in past tense", "Enabled repository");
    case Client::ActionRepoSetData :
        return i18nc("The role of the transaction, in past tense", "Set repository data");
    case Client::ActionResolve :
        return i18nc("The role of the transaction, in past tense", "Resolved");
    case Client::ActionGetFiles :
        return i18nc("The role of the transaction, in past tense", "Got file list");
    case Client::ActionWhatProvides :
        return i18nc("The role of the transaction, in past tense", "Got what provides");
    case Client::ActionInstallSignature :
        return i18nc("The role of the transaction, in past tense", "Installed signature");
    case Client::ActionGetPackages :
        return i18nc("The role of the transaction, in past tense", "Got package lists");
    case Client::ActionAcceptEula :
        return i18nc("The role of the transaction, in past tense", "Accepted EULA");
    case Client::ActionDownloadPackages :
        return i18nc("The role of the transaction, in past tense", "Downloaded packages");
    case Client::ActionGetDistroUpgrades :
        return i18nc("The role of the transaction, in past tense", "Got distribution upgrades");
    case Client::ActionGetCategories :
        return i18nc("The role of the transaction, in past tense", "Got categories");
//  case Client::ActionGetOldTransactions :
//      return i18nc("The role of the transaction, in past tense", "Got old transactions");
    }
    kDebug() << "action unrecognised: " << action;
    return QString();
}

QString KpkStrings::error(PackageKit::Client::ErrorType error)
{
    switch (error) {
    case Client::NoNetwork :
        return i18n("No network connection available");
    case Client::NoCache :
        return i18n("No package cache is available");
    case Client::Oom :
        return i18n("Out of memory");
    case Client::CreateThreadFailed :
        return i18n("Failed to create a thread");
    case Client::NotSupported :
        return i18n("Not supported by this backend");
    case Client::InternalError :
        return i18n("An internal system error has occurred");
    case Client::GpgFailure :
        return i18n("A security trust relationship is not present");
    case Client::PackageNotInstalled :
        return i18n("The package is not installed");
    case Client::PackageNotFound :
        return i18n("The package was not found");
    case Client::PackageAlreadyInstalled :
        return i18n("The package is already installed");
    case Client::PackageDownloadFailed :
        return i18n("The package download failed");
    case Client::GroupNotFound :
        return i18n("The group was not found");
    case Client::GroupListInvalid :
        return i18n("The group list was invalid");
    case Client::DepResolutionFailed :
        return i18n("Dependency resolution failed");
    case Client::FilterInvalid :
        return i18n("Search filter was invalid");
    case Client::PackageIdInvalid :
        return i18n("The package identifier was not well formed");
    case Client::TransactionError :
        return i18n("Transaction error");
    case Client::RepoNotFound :
        return i18n("Repository name was not found");
    case Client::CannotRemoveSystemPackage :
        return i18n("Could not remove a protected system package");
    case Client::TransactionCancelled :
        return i18n("The task was canceled");
    case Client::ProcessKill :
        return i18n("The task was forcibly canceled");
    case Client::FailedConfigParsing :
        return i18n("Reading the config file failed");
    case Client::CannotCancel :
        return i18n("The task cannot be cancelled");
    case Client::CannotInstallSourcePackage :
        return i18n("Source packages cannot be installed");
    case Client::NoLicenseAgreement :
        return i18n("The license agreement failed");
    case Client::FileConflicts :
        return i18n("Local file conflict between packages");
    case Client::PackageConflicts :
        return i18n("Packages are not compatible");
    case Client::RepoNotAvailable :
        return i18n("Problem connecting to a software source");
    case Client::FailedInitialization :
        return i18n("Failed to initialize");
    case Client::FailedFinalise :
        return i18n("Failed to finalize");
    case Client::CannotGetLock :
        return i18n("Cannot get lock");
    case Client::NoPackagesToUpdate :
        return i18n("No packages to update");
    case Client::CannotWriteRepoConfig :
        return i18n("Cannot write repository configuration");
    case Client::LocalInstallFailed :
        return i18n("Local install failed");
    case Client::BadGpgSignature :
        return i18n("Bad GPG signature");
    case Client::MissingGpgSignature :
        return i18n("Missing GPG signature");
    case Client::RepoConfigurationError :
        return i18n("Repository configuration invalid");
    case Client::InvalidPackageFile :
        return i18n("Invalid package file");
    case Client::PackageInstallBlocked :
        return i18n("Package install blocked");
    case Client::PackageCorrupt :
        return i18n("Package is corrupt");
    case Client::AllPackagesAlreadyInstalled :
        return i18n("All packages are already installed");
    case Client::FileNotFound :
        return i18n("The specified file could not be found");
    case Client::NoMoreMirrorsToTry :
        return i18n("No more mirrors are available");
//  case Client::NoDistroUpgradeData :
//      return i18n("No distribution upgrade data is available");
//  case Client::IncompatibleArchitecture :
//      return i18n("Package is incompatible with this system");
//  case Client::NoSpaceOnDevice :
//      return i18n("No space is left on the disk");
    case Client::UnknownErrorType :
        return i18n("Unknown error");
    }
    kDebug() << "error unrecognised: " << error;
    return QString();
}

QString KpkStrings::errorMessage(PackageKit::Client::ErrorType error)
{
    switch (error) {
    case Client::NoNetwork :
        return i18n("There is no network connection available.\n"
                    "Please check your connection settings and try again");
    case Client::NoCache :
        return i18n("The package list needs to be rebuilt.\n"
                    "This should have been done by the backend automatically.");
    case Client::Oom :
        return i18n("The service that is responsible for handling user requests is out of memory.\n"
                    "Please close some programs or restart your computer.");
    case Client::CreateThreadFailed :
        return i18n("A thread could not be created to service the user request.");
    case Client::NotSupported :
        return i18n("The action is not supported by this backend.\n"
                    "Please report a bug as this should not have happened.");
    case Client::InternalError :
        return i18n("A problem that we were not expecting has occurred.\n"
                    "Please report this bug with the error description.");
    case Client::GpgFailure :
        return i18n("A security trust relationship could not be made with the software source.\n"
                    "Please check your software signature settings.");
    case Client::PackageNotInstalled :
        return i18n("The package that is trying to be removed or updated is not already installed.");
    case Client::PackageNotFound :
        return i18n("The package that is being modified was not found on your system or in any software source.");
    case Client::PackageAlreadyInstalled :
        return i18n("The package that is trying to be installed is already installed.");
    case Client::PackageDownloadFailed :
        return i18n("The package download failed.\n"
                    "Please check your network connectivity.");
    case Client::GroupNotFound :
        return i18n("The group type was not found.\n"
                    "Please check your group list and try again.");
    case Client::GroupListInvalid :
        return i18n("The group list could not be loaded.\n"
                    "Refreshing your cache may help, although this is normally a software "
                    "source error.");
    case Client::DepResolutionFailed :
        return i18n("A package dependency could not be found.\n"
                    "More information is available in the detailed report.");
    case Client::FilterInvalid :
        return i18n("The search filter was not correctly formed.");
    case Client::PackageIdInvalid :
        return i18n("The package identifier was not well formed when sent to the system daemon.\n"
                    "This normally indicates an internal bug and should be reported.");
    case Client::TransactionError :
        return i18n("An error occurred while running the transaction.\n"
                    "More information is available in the detailed report.");
    case Client::RepoNotFound :
        return i18n("The remote software source name was not found.\n"
                    "You may need to enable an item in Software Sources.");
    case Client::CannotRemoveSystemPackage :
        return i18n("Removing a protected system package is not allowed.");
    case Client::TransactionCancelled :
        return i18n("The task was canceled successfully and no packages were changed.");
    case Client::ProcessKill :
        return i18n("The task was canceled successfully and no packages were changed.\n"
                    "The backend did not exit cleanly.");
    case Client::FailedConfigParsing :
        return i18n("The native package configuration file could not be opened.\n"
                    "Please make sure your system's configuration is valid.");
    case Client::CannotCancel :
        return i18n("The task is not safe to be cancelled at this time.");
    case Client::CannotInstallSourcePackage :
        return i18n("Source packages are not normally installed this way.\n"
                    "Check the extension of the file you are trying to install.");
    case Client::NoLicenseAgreement :
        return i18n("The license agreement was not agreed to.\n"
                    "To use this software you have to accept the license.");
    case Client::FileConflicts :
        return i18n("Two packages provide the same file.\n"
                    "This is usually due to mixing packages for different software sources.");
    case Client::PackageConflicts :
        return i18n("Multiple packages exist that are not compatible with each other.\n"
                    "This is usually due to mixing packages from different software sources.");
    case Client::RepoNotAvailable :
        return i18n("There was a (possibly temporary) problem connecting to a software source.\n"
                    "Please check the detailed error for further details.");
    case Client::FailedInitialization :
        return i18n("Failed to initialize packaging backend.\n"
                    "This may occur if other packaging tools are being used simultaneously.");
    case Client::FailedFinalise :
        return i18n("Failed to close down the backend instance.\n"
                    "This error can normally be ignored.");
    case Client::CannotGetLock :
        return i18n("Cannot get the exclusive lock on the packaging backend.\n"
                    "Please close any other legacy packaging tools that may be open.");
    case Client::NoPackagesToUpdate :
        return i18n("None of the selected packages could be updated.");
    case Client::CannotWriteRepoConfig :
        return i18n("The repository configuration could not be modified.");
    case Client::LocalInstallFailed :
        return i18n("Installing the local file failed.\n"
                    "More information is available in the detailed report.");
    case Client::BadGpgSignature :
        return i18n("The package signature could not be verified.");
    case Client::MissingGpgSignature :
        return i18n("The package signature was missing and this package is untrusted.\n"
                    "This package was not signed with a GPG key when created.");
    case Client::RepoConfigurationError :
        return i18n("Repository configuration was invalid and could not be read.");
    case Client::InvalidPackageFile :
        return i18n("The package you are attempting to install is not valid.\n"
                    "The package file could be corrupt, or not a proper package.");
    case Client::PackageInstallBlocked :
        return i18n("Installation of this package was prevented by your packaging system's configuration.");
    case Client::PackageCorrupt :
        return i18n("The package that was downloaded is corrupt and needs to be downloaded again.");
    case Client::AllPackagesAlreadyInstalled :
        return i18n("All of the packages selected for install are already installed on the system.");
    case Client::FileNotFound :
        return i18n("The specified file could not be found on the system.\n"
                    "Check that the file still exists and has not been deleted.");
    case Client::NoMoreMirrorsToTry :
        return i18n("Required data could not be found on any of the configured software sources.\n"
                    "There were no more download mirrors that could be tried.");
//  case Client::NoDistroUpgradeData :
//      return i18n("Required upgrade data could not be found in any of the configured software sources.\n"
//                  "The list of distribution upgrades will be unavailable.");
//  case Client::IncompatibleArchitecture :
//      return i18n("The package that is trying to be installed is incompatible with this system.");
//  case Client::NoSpaceOnDevice :
//      return i18n("There is insufficient space on the device.\n"
//                  "Free some space on the system disk to perform this operation.");
    case Client::UnknownErrorType :
        return i18n("Unknown error, please report a bug.\n"
                    "More information is available in the detailed report.");
    }
    kDebug() << "error unrecognised: " << error;
    return QString();
}

QString KpkStrings::groups(Client::Group group)
{
    switch (group) {
    case Client::Accessibility :
        return i18nc("The group type", "Accessibility");
    case Client::Accessories :
        return i18nc("The group type", "Accessories");
    case Client::Education :
        return i18nc("The group type", "Education");
    case Client::Games :
        return i18nc("The group type", "Games");
    case Client::Graphics :
        return i18nc("The group type", "Graphics");
    case Client::Internet :
        return i18nc("The group type", "Internet");
    case Client::Office :
        return i18nc("The group type", "Office");
    case Client::Other :
        return i18nc("The group type", "Other");
    case Client::Programming :
        return i18nc("The group type", "Development");
    case Client::Multimedia :
        return i18nc("The group type", "Multimedia");
    case Client::System :
        return i18nc("The group type", "System");
    case Client::DesktopGnome :
        return i18nc("The group type", "GNOME desktop");
    case Client::DesktopKde :
        return i18nc("The group type", "KDE desktop");
    case Client::DesktopXfce :
        return i18nc("The group type", "XFCE desktop");
    case Client::DesktopOther :
        return i18nc("The group type", "Other desktops");
    case Client::Publishing :
        return i18nc("The group type", "Publishing");
    case Client::Servers :
        return i18nc("The group type", "Servers");
    case Client::Fonts :
        return i18nc("The group type", "Fonts");
    case Client::AdminTools :
        return i18nc("The group type", "Admin tools");
    case Client::Legacy :
        return i18nc("The group type", "Legacy");
    case Client::Localization :
        return i18nc("The group type", "Localization");
    case Client::Virtualization :
        return i18nc("The group type", "Virtualization");
    case Client::Security :
        return i18nc("The group type", "Security");
    case Client::PowerManagement :
        return i18nc("The group type", "Power management");
    case Client::Communication :
        return i18nc("The group type", "Communication");
    case Client::Network :
        return i18nc("The group type", "Network");
    case Client::Maps :
        return i18nc("The group type", "Maps");
    case Client::Repos :
        return i18nc("The group type", "Software sources");
    case Client::Science :
        return i18nc("The group type", "Science");
    case Client::Documentation :
        return i18nc("The group type", "Documentation");
    case Client::Electronics :
        return i18nc("The group type", "Electronics");
    case Client::Collections ://TODO check this one
        return i18nc("The group type", "Package collections");
    case Client::Vendor :
        return i18nc("The group type", "Vendor");
    case Client::Newest :
        return i18nc("The group type", "Newest packages");
    case Client::UnknownGroup :
        return i18nc("The group type", "Unknown group");
    }
    kDebug() << "group unrecognised: " << group;
    return QString();
}

QString KpkStrings::info(Package::State state)
{
    switch (state) {
    case Package::Low :
        return i18nc("The type of update", "Trivial update");
    case Package::Normal :
        return i18nc("The type of update", "Normal update");
    case Package::Important :
        return i18nc("The type of update", "Important update");
    case Package::Security :
        return i18nc("The type of update", "Security update");
    case Package::Bugfix :
        return i18nc("The type of update", "Bug fix update");
    case Package::Enhancement :
        return i18nc("The type of update", "Enhancement update");
    case Package::Blocked :
        return i18nc("The type of update", "Blocked update");
    case Package::Installed :
//  case Package::CollectionInstalled :
        return i18nc("The type of update", "Installed");
    case Package::Available :
//  case Package::CollectionAvailable :
        return i18nc("The type of update", "Available");
    case Package::UnknownState :
        return i18nc("The type of update", "Unknown update");
    default : // In this case we don't want to map all enums
        kDebug() << "info unrecognised: " << state;
        return QString();
    }
}

QString KpkStrings::infoUpdate(Package::State state, int number)
{
    switch (state) {
    case Package::Low :
        return i18np("1 trivial update", "%1 trivial updates", number);
    case Package::Normal :
        return i18ncp("Type of update, in the case it's just an update", "1 update", "%1 updates", number);
    case Package::Important :
        return i18np("1 important update", "%1 important updates", number);
    case Package::Security :
        return i18np("1 security update", "%1 security updates", number);
    case Package::Bugfix :
        return i18np("1 bug fix update", "%1 bug fix updates", number);
    case Package::Enhancement :
        return i18np("1 enhancement update", "%1 enhancement updates", number);
    case Package::Blocked :
        return i18np("1 blocked update", "%1 blocked updates", number);
    case Package::Installed :
        return i18np("1 installed package", "%1 installed packages", number);
    case Package::Available :
        return i18np("1 available package", "%1 available packages", number);
    default : // In this case we don't want to map all enums
        kDebug() << "update info unrecognised: " << state;
        return i18np("1 unknown update", "%1 unknown updates", number);
    }
}

QString KpkStrings::restartType(Client::RestartType value)
{
    switch (value) {
    case Client::RestartNone :
        return i18n("No restart is required");
    case Client::RestartSystem :
        return i18n("A system restart is required");
    case Client::RestartSession :
        return i18n("You will need to log off and log back on");
    case Client::RestartApplication :
        return i18n("You need to restart the application");
    case Client::UnknownRestartType :
        return QString();
    }
    kDebug() << "restart unrecognised: " << value;
    return QString();
}

QString KpkStrings::restartTypeFuture(Client::RestartType value)
{
    switch (value) {
    case Client::RestartNone :
        return i18n("No restart is necessary for this update");
    case Client::RestartApplication :
        return i18n("An application restart is required after this update");
    case Client::RestartSession :
        return i18n("You will be required to log off and back on after this update");
    case Client::RestartSystem :
        return i18n("A system restart is required after this update");
    case Client::UnknownRestartType :
        return QString();
    }
    kDebug() << "restart unrecognised: " << value;
    return QString();
}

QString KpkStrings::updateState(Client::UpgradeType value)
{
    switch (value) {
    case Client::UpgradeStable :
        return i18n("Stable");
    case Client::UpgradeUnstable :
        return i18n("Unstable");
    case Client::UpgradeTesting :
        return i18n("Testing");
    case Client::UnknownUpgradeType :
        return QString();
    }
    kDebug() << "value unrecognised: " << value;
    return QString();
}
