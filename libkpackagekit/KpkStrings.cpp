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
    case Transaction::StatusSetup :
        return i18nc("The transaction state", "Waiting for service to start");
    case Transaction::StatusWait :
        return i18nc("The transaction state", "Waiting for other tasks");
    case Transaction::StatusRunning :
        return i18nc("The transaction state", "Running task");
    case Transaction::StatusQuery :
        return i18nc("The transaction state", "Querying");
    case Transaction::StatusInfo :
        return i18nc("The transaction state", "Getting information");
    case Transaction::StatusRemove :
        return i18nc("The transaction state", "Removing packages");
    case Transaction::StatusDownload :
        return i18nc("The transaction state", "Downloading packages");
    case Transaction::StatusInstall :
        return i18nc("The transaction state", "Installing packages");
    case Transaction::StatusRefreshCache :
        return i18nc("The transaction state", "Refreshing software list");
    case Transaction::StatusUpdate :
        return i18nc("The transaction state", "Updating packages");
    case Transaction::StatusCleanup :
        return i18nc("The transaction state", "Cleaning up packages");
    case Transaction::StatusObsolete :
        return i18nc("The transaction state", "Obsoleting packages");
    case Transaction::StatusDepResolve :
        return i18nc("The transaction state", "Resolving dependencies");
    case Transaction::StatusSigCheck :
        return i18nc("The transaction state", "Checking signatures");
    case Transaction::StatusRollback :
        return i18nc("The transaction state", "Rolling back");
    case Transaction::StatusTestCommit :
        return i18nc("The transaction state", "Testing changes");
    case Transaction::StatusCommit :
        return i18nc("The transaction state", "Committing changes");
    case Transaction::StatusRequest :
        return i18nc("The transaction state", "Requesting data");
    case Transaction::StatusFinished :
        return i18nc("The transaction state", "Finished");
    case Transaction::StatusCancel :
        return i18nc("The transaction state", "Cancelling");
    case Transaction::StatusDownloadRepository :
        return i18nc("The transaction state", "Downloading repository information");
    case Transaction::StatusDownloadPackagelist :
        return i18nc("The transaction state", "Downloading list of packages");
    case Transaction::StatusDownloadFilelist :
        return i18nc("The transaction state", "Downloading file lists");
    case Transaction::StatusDownloadChangelog :
        return i18nc("The transaction state", "Downloading lists of changes");
    case Transaction::StatusDownloadGroup :
        return i18nc("The transaction state", "Downloading groups");
    case Transaction::StatusDownloadUpdateinfo :
        return i18nc("The transaction state", "Downloading update information");
    case Transaction::StatusRepackaging :
        return i18nc("The transaction state", "Repackaging files");
    case Transaction::StatusLoadingCache :
        return i18nc("The transaction state", "Loading cache");
    case Transaction::StatusScanApplications :
        return i18nc("The transaction state", "Scanning installed applications");
    case Transaction::StatusGeneratePackageList :
        return i18nc("The transaction state", "Generating package lists");
    case Transaction::StatusWaitingForLock :
        return i18nc("The transaction state", "Waiting for package manager lock");
    }
    kWarning() << "status unrecognised: " << status;
    return QString();
}

QString KpkStrings::statusPast(PackageKit::Transaction::Status status)
{
    switch (status) {
    case Transaction::StatusDownload:
        return i18nc("The action of the package, in past tense", "Downloaded");
    case Transaction::StatusUpdate:
        return i18nc("The action of the package, in past tense", "Updated");
    case Transaction::StatusInstall:
        return i18nc("The action of the package, in past tense", "Installed");
    case Transaction::StatusRemove:
        return i18nc("The action of the package, in past tense", "Removed");
    case Transaction::StatusCleanup:
        return i18nc("The action of the package, in past tense", "Cleaned Up");
    case Transaction::StatusObsolete:
        return i18nc("The action of the package, in past tense", "Obsoleted");
    default : // In this case we don't want to map all enums
        kWarning() << "status unrecognised: " << status;
        return QString();
    }
}

QString KpkStrings::action(Client::Action action)
{
    switch (action) {
    case Client::UnknownAction :
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
    case Client::ActionGetOldTransactions :
        return i18nc("The role of the transaction, in present tense", "Getting old transactions");
    }
    kWarning() << "action unrecognised: " << action;
    return QString();
}

QString KpkStrings::actionPast(Client::Action action)
{
    switch (action) {
    case Client::UnknownAction :
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
    case Client::ActionGetOldTransactions :
        return i18nc("The role of the transaction, in past tense", "Got old transactions");
    }
    kWarning() << "action unrecognised: " << action;
    return QString();
}

QString KpkStrings::error(PackageKit::Client::ErrorType error)
{
    switch (error) {
    case Client::ErrorNoNetwork :
        return i18n("No network connection available");
    case Client::ErrorNoCache :
        return i18n("No package cache is available");
    case Client::ErrorOom :
        return i18n("Out of memory");
    case Client::ErrorCreateThreadFailed :
        return i18n("Failed to create a thread");
    case Client::ErrorNotSupported :
        return i18n("Not supported by this backend");
    case Client::ErrorInternalError :
        return i18n("An internal system error has occurred");
    case Client::ErrorGpgFailure :
        return i18n("A security trust relationship is not present");
    case Client::ErrorPackageNotInstalled :
        return i18n("The package is not installed");
    case Client::ErrorPackageNotFound :
        return i18n("The package was not found");
    case Client::ErrorPackageAlreadyInstalled :
        return i18n("The package is already installed");
    case Client::ErrorPackageDownloadFailed :
        return i18n("The package download failed");
    case Client::ErrorGroupNotFound :
        return i18n("The group was not found");
    case Client::ErrorGroupListInvalid :
        return i18n("The group list was invalid");
    case Client::ErrorDepResolutionFailed :
        return i18n("Dependency resolution failed");
    case Client::ErrorFilterInvalid :
        return i18n("Search filter was invalid");
    case Client::ErrorPackageIdInvalid :
        return i18n("The package identifier was not well formed");
    case Client::ErrorTransactionError :
        return i18n("Transaction error");
    case Client::ErrorRepoNotFound :
        return i18n("Repository name was not found");
    case Client::ErrorCannotRemoveSystemPackage :
        return i18n("Could not remove a protected system package");
    case Client::ErrorTransactionCancelled :
        return i18n("The task was canceled");
    case Client::ErrorProcessKill :
        return i18n("The task was forcibly canceled");
    case Client::ErrorFailedConfigParsing :
        return i18n("Reading the config file failed");
    case Client::ErrorCannotCancel :
        return i18n("The task cannot be cancelled");
    case Client::ErrorCannotInstallSourcePackage :
        return i18n("Source packages cannot be installed");
    case Client::ErrorNoLicenseAgreement :
        return i18n("The license agreement failed");
    case Client::ErrorFileConflicts :
        return i18n("Local file conflict between packages");
    case Client::ErrorPackageConflicts :
        return i18n("Packages are not compatible");
    case Client::ErrorRepoNotAvailable :
        return i18n("Problem connecting to a software orign");
    case Client::ErrorFailedInitialization :
        return i18n("Failed to initialize");
    case Client::ErrorFailedFinalise :
        return i18n("Failed to finalize");
    case Client::ErrorCannotGetLock :
        return i18n("Cannot get lock");
    case Client::ErrorNoPackagesToUpdate :
        return i18n("No packages to update");
    case Client::ErrorCannotWriteRepoConfig :
        return i18n("Cannot write repository configuration");
    case Client::ErrorLocalInstallFailed :
        return i18n("Local install failed");
    case Client::ErrorBadGpgSignature :
        return i18n("Bad GPG signature");
    case Client::ErrorMissingGpgSignature :
        return i18n("Missing GPG signature");
    case Client::ErrorRepoConfigurationError :
        return i18n("Repository configuration invalid");
    case Client::ErrorInvalidPackageFile :
        return i18n("Invalid package file");
    case Client::ErrorPackageInstallBlocked :
        return i18n("Package install blocked");
    case Client::ErrorPackageCorrupt :
        return i18n("Package is corrupt");
    case Client::ErrorAllPackagesAlreadyInstalled :
        return i18n("All packages are already installed");
    case Client::ErrorFileNotFound :
        return i18n("The specified file could not be found");
    case Client::ErrorNoMoreMirrorsToTry :
        return i18n("No more mirrors are available");
    case Client::ErrorNoDistroUpgradeData :
        return i18n("No distribution upgrade data is available");
    case Client::ErrorIncompatibleArchitecture :
        return i18n("Package is incompatible with this system");
    case Client::ErrorNoSpaceOnDevice :
        return i18n("No space is left on the disk");
    case Client::ErrorMediaChangeRequired :
        return i18n("A media change is required");
    case Client::ErrorNotAuthorized :
        return i18n("Authorization failed");
    case Client::ErrorUpdateNotFound :
        return i18n("Update not found");
    case Client::ErrorCannotInstallRepoUnsigned :
        return i18n("Cannot install from untrusted orign");
    case Client::ErrorCannotUpdateRepoUnsigned :
        return i18n("Cannot update from untrusted orign");
    case Client::ErrorCannotGetFilelist :
        return i18n("Cannot get the file list");
    case Client::ErrorCannotGetRequires :
        return i18n("Cannot get package requires");
    case Client::ErrorCannotDisableRepository :
        return i18n("Cannot disable orign");
    case Client::ErrorRestrictedDownload :
        return i18n("The download failed");
    case Client::ErrorPackageFailedToConfigure :
        return i18n("Package failed to configure");
    case Client::ErrorPackageFailedToBuild :
        return i18n("Package failed to build");
    case Client::ErrorPackageFailedToInstall :
        return i18n("Package failed to install");
    case Client::UnknownErrorType :
        return i18n("Unknown error");
    }
    kWarning() << "error unrecognised: " << error;
    return QString();
}

QString KpkStrings::errorMessage(PackageKit::Client::ErrorType error)
{
    switch (error) {
    case Client::ErrorNoNetwork :
        return i18n("There is no network connection available.\n"
                    "Please check your connection settings and try again");
    case Client::ErrorNoCache :
        return i18n("The package list needs to be rebuilt.\n"
                    "This should have been done by the backend automatically.");
    case Client::ErrorOom :
        return i18n("The service that is responsible for handling user requests is out of memory.\n"
                    "Please close some programs or restart your computer.");
    case Client::ErrorCreateThreadFailed :
        return i18n("A thread could not be created to service the user request.");
    case Client::ErrorNotSupported :
        return i18n("The action is not supported by this backend.\n"
                    "Please report a bug as this should not have happened.");
    case Client::ErrorInternalError :
        return i18n("A problem that we were not expecting has occurred.\n"
                    "Please report this bug with the error description.");
    case Client::ErrorGpgFailure :
        return i18n("A security trust relationship could not be made with the software orign.\n"
                    "Please check your software signature settings.");
    case Client::ErrorPackageNotInstalled :
        return i18n("The package that is trying to be removed or updated is not already installed.");
    case Client::ErrorPackageNotFound :
        return i18n("The package that is being modified was not found on your system or in any software orign.");
    case Client::ErrorPackageAlreadyInstalled :
        return i18n("The package that is trying to be installed is already installed.");
    case Client::ErrorPackageDownloadFailed :
        return i18n("The package download failed.\n"
                    "Please check your network connectivity.");
    case Client::ErrorGroupNotFound :
        return i18n("The group type was not found.\n"
                    "Please check your group list and try again.");
    case Client::ErrorGroupListInvalid :
        return i18n("The group list could not be loaded.\n"
                    "Refreshing your cache may help, although this is normally a software "
                    "orign error.");
    case Client::ErrorDepResolutionFailed :
        return i18n("A package dependency could not be found.\n"
                    "More information is available in the detailed report.");
    case Client::ErrorFilterInvalid :
        return i18n("The search filter was not correctly formed.");
    case Client::ErrorPackageIdInvalid :
        return i18n("The package identifier was not well formed when sent to the system daemon.\n"
                    "This normally indicates an internal bug and should be reported.");
    case Client::ErrorTransactionError :
        return i18n("An error occurred while running the transaction.\n"
                    "More information is available in the detailed report.");
    case Client::ErrorRepoNotFound :
        return i18n("The remote software orign name was not found.\n"
                    "You may need to enable an item in Software Origns.");
    case Client::ErrorCannotRemoveSystemPackage :
        return i18n("Removing a protected system package is not allowed.");
    case Client::ErrorTransactionCancelled :
        return i18n("The task was canceled successfully and no packages were changed.");
    case Client::ErrorProcessKill :
        return i18n("The task was canceled successfully and no packages were changed.\n"
                    "The backend did not exit cleanly.");
    case Client::ErrorFailedConfigParsing :
        return i18n("The native package configuration file could not be opened.\n"
                    "Please make sure your system's configuration is valid.");
    case Client::ErrorCannotCancel :
        return i18n("The task is not safe to be cancelled at this time.");
    case Client::ErrorCannotInstallSourcePackage :
        return i18n("Source packages are not normally installed this way.\n"
                    "Check the extension of the file you are trying to install.");
    case Client::ErrorNoLicenseAgreement :
        return i18n("The license agreement was not agreed to.\n"
                    "To use this software you have to accept the license.");
    case Client::ErrorFileConflicts :
        return i18n("Two packages provide the same file.\n"
                    "This is usually due to mixing packages for different software origns.");
    case Client::ErrorPackageConflicts :
        return i18n("Multiple packages exist that are not compatible with each other.\n"
                    "This is usually due to mixing packages from different software origns.");
    case Client::ErrorRepoNotAvailable :
        return i18n("There was a (possibly temporary) problem connecting to a software origns.\n"
                    "Please check the detailed error for further details.");
    case Client::ErrorFailedInitialization :
        return i18n("Failed to initialize packaging backend.\n"
                    "This may occur if other packaging tools are being used simultaneously.");
    case Client::ErrorFailedFinalise :
        return i18n("Failed to close down the backend instance.\n"
                    "This error can normally be ignored.");
    case Client::ErrorCannotGetLock :
        return i18n("Cannot get the exclusive lock on the packaging backend.\n"
                    "Please close any other legacy packaging tools that may be open.");
    case Client::ErrorNoPackagesToUpdate :
        return i18n("None of the selected packages could be updated.");
    case Client::ErrorCannotWriteRepoConfig :
        return i18n("The repository configuration could not be modified.");
    case Client::ErrorLocalInstallFailed :
        return i18n("Installing the local file failed.\n"
                    "More information is available in the detailed report.");
    case Client::ErrorBadGpgSignature :
        return i18n("The package signature could not be verified.");
    case Client::ErrorMissingGpgSignature :
        return i18n("The package signature was missing and this package is untrusted.\n"
                    "This package was not signed with a GPG key when created.");
    case Client::ErrorRepoConfigurationError :
        return i18n("Repository configuration was invalid and could not be read.");
    case Client::ErrorInvalidPackageFile :
        return i18n("The package you are attempting to install is not valid.\n"
                    "The package file could be corrupt, or not a proper package.");
    case Client::ErrorPackageInstallBlocked :
        return i18n("Installation of this package was prevented by your packaging system's configuration.");
    case Client::ErrorPackageCorrupt :
        return i18n("The package that was downloaded is corrupt and needs to be downloaded again.");
    case Client::ErrorAllPackagesAlreadyInstalled :
        return i18n("All of the packages selected for install are already installed on the system.");
    case Client::ErrorFileNotFound :
        return i18n("The specified file could not be found on the system.\n"
                    "Check that the file still exists and has not been deleted.");
    case Client::ErrorNoMoreMirrorsToTry :
        return i18n("Required data could not be found on any of the configured software origns.\n"
                    "There were no more download mirrors that could be tried.");
    case Client::ErrorNoDistroUpgradeData :
        return i18n("Required upgrade data could not be found in any of the configured software origns.\n"
                    "The list of distribution upgrades will be unavailable.");
    case Client::ErrorIncompatibleArchitecture :
        return i18n("The package that is trying to be installed is incompatible with this system.");
    case Client::ErrorNoSpaceOnDevice :
        return i18n("There is insufficient space on the device.\n"
                    "Free some space on the system disk to perform this operation.");
    case Client::ErrorMediaChangeRequired :
        return i18n("Additional media is required to complete the transaction.");
    case Client::ErrorNotAuthorized :
        return i18n("You have failed to provide correct authentication.\n"
                    "Please check any passwords or account settings.");
    case Client::ErrorUpdateNotFound :
        return i18n("The specified update could not be found.\n"
                    "It could have already been installed or no longer available on the remote server.");
    case Client::ErrorCannotInstallRepoUnsigned :
        return i18n("The package could not be installed from untrusted orign.");
    case Client::ErrorCannotUpdateRepoUnsigned :
        return i18n("The package could not be updated from untrusted orign.");
    case Client::ErrorCannotGetFilelist :
        return i18n("The file list is not available for this package.");
    case Client::ErrorCannotGetRequires :
        return i18n("The information about what requires this package could not be obtained.");
    case Client::ErrorCannotDisableRepository :
        return i18n("The specified software orign could not be disabled.");
    case Client::ErrorRestrictedDownload :
        return i18n("The download could not be done automatically and should be done manually.\n"
                    "More information is available in the detailed report.");
    case Client::ErrorPackageFailedToConfigure :
        return i18n("One of the selected packages failed to configure correctly.\n"
                    "More information is available in the detailed report.");
    case Client::ErrorPackageFailedToBuild :
        return i18n("One of the selected packages failed to build correctly.\n"
                    "More information is available in the detailed report.");
    case Client::ErrorPackageFailedToInstall :
        return i18n("One of the selected packages failed to install correctly.\n"
                    "More information is available in the detailed report.");
    case Client::UnknownErrorType :
        return i18n("Unknown error, please report a bug.\n"
                    "More information is available in the detailed report.");
    }
    kWarning() << "error unrecognised: " << error;
    return QString();
}

QString KpkStrings::groups(Client::Group group)
{
    switch (group) {
    case Client::GroupAccessibility :
        return i18nc("The group type", "Accessibility");
    case Client::GroupAccessories :
        return i18nc("The group type", "Accessories");
    case Client::GroupEducation :
        return i18nc("The group type", "Education");
    case Client::GroupGames :
        return i18nc("The group type", "Games");
    case Client::GroupGraphics :
        return i18nc("The group type", "Graphics");
    case Client::GroupInternet :
        return i18nc("The group type", "Internet");
    case Client::GroupOffice :
        return i18nc("The group type", "Office");
    case Client::GroupOther :
        return i18nc("The group type", "Other");
    case Client::GroupProgramming :
        return i18nc("The group type", "Development");
    case Client::GroupMultimedia :
        return i18nc("The group type", "Multimedia");
    case Client::GroupSystem :
        return i18nc("The group type", "System");
    case Client::GroupDesktopGnome :
        return i18nc("The group type", "GNOME desktop");
    case Client::GroupDesktopKde :
        return i18nc("The group type", "KDE desktop");
    case Client::GroupDesktopXfce :
        return i18nc("The group type", "XFCE desktop");
    case Client::GroupDesktopOther :
        return i18nc("The group type", "Other desktops");
    case Client::GroupPublishing :
        return i18nc("The group type", "Publishing");
    case Client::GroupServers :
        return i18nc("The group type", "Servers");
    case Client::GroupFonts :
        return i18nc("The group type", "Fonts");
    case Client::GroupAdminTools :
        return i18nc("The group type", "Admin tools");
    case Client::GroupLegacy :
        return i18nc("The group type", "Legacy");
    case Client::GroupLocalization :
        return i18nc("The group type", "Localization");
    case Client::GroupVirtualization :
        return i18nc("The group type", "Virtualization");
    case Client::GroupSecurity :
        return i18nc("The group type", "Security");
    case Client::GroupPowerManagement :
        return i18nc("The group type", "Power management");
    case Client::GroupCommunication :
        return i18nc("The group type", "Communication");
    case Client::GroupNetwork :
        return i18nc("The group type", "Network");
    case Client::GroupMaps :
        return i18nc("The group type", "Maps");
    case Client::GroupRepos :
        return i18nc("The group type", "Software sources");
    case Client::GroupScience :
        return i18nc("The group type", "Science");
    case Client::GroupDocumentation :
        return i18nc("The group type", "Documentation");
    case Client::GroupElectronics :
        return i18nc("The group type", "Electronics");
    case Client::GroupCollections ://TODO check this one
        return i18nc("The group type", "Package collections");
    case Client::GroupVendor :
        return i18nc("The group type", "Vendor");
    case Client::GroupNewest :
        return i18nc("The group type", "Newest packages");
    case Client::UnknownGroup :
        return i18nc("The group type", "Unknown group");
    }
    kWarning() << "group unrecognised: " << group;
    return QString();
}

QString KpkStrings::info(Package::State state)
{
    switch (state) {
    case Package::StateLow :
        return i18nc("The type of update", "Trivial update");
    case Package::StateNormal :
        return i18nc("The type of update", "Normal update");
    case Package::StateImportant :
        return i18nc("The type of update", "Important update");
    case Package::StateSecurity :
        return i18nc("The type of update", "Security update");
    case Package::StateBugfix :
        return i18nc("The type of update", "Bug fix update");
    case Package::StateEnhancement :
        return i18nc("The type of update", "Enhancement update");
    case Package::StateBlocked :
        return i18nc("The type of update", "Blocked update");
    case Package::StateInstalled :
    case Package::StateCollectionInstalled :
        return i18nc("The type of update", "Installed");
    case Package::StateAvailable :
    case Package::StateCollectionAvailable :
        return i18nc("The type of update", "Available");
    case Package::UnknownState :
        return i18nc("The type of update", "Unknown update");
    default : // In this case we don't want to map all enums
        kWarning() << "info unrecognised: " << state;
        return QString();
    }
}

QString KpkStrings::infoUpdate(Package::State state, int number)
{
    switch (state) {
    case Package::StateLow :
        return i18np("1 trivial update", "%1 trivial updates", number);
    case Package::StateNormal :
        return i18ncp("Type of update, in the case it's just an update", "1 update", "%1 updates", number);
    case Package::StateImportant :
        return i18np("1 important update", "%1 important updates", number);
    case Package::StateSecurity :
        return i18np("1 security update", "%1 security updates", number);
    case Package::StateBugfix :
        return i18np("1 bug fix update", "%1 bug fix updates", number);
    case Package::StateEnhancement :
        return i18np("1 enhancement update", "%1 enhancement updates", number);
    case Package::StateBlocked :
        return i18np("1 blocked update", "%1 blocked updates", number);
    case Package::StateInstalled :
        return i18np("1 installed package", "%1 installed packages", number);
    case Package::StateAvailable :
        return i18np("1 available package", "%1 available packages", number);
    default : // In this case we don't want to map all enums
        kWarning() << "update info unrecognised: " << state;
        return i18np("1 unknown update", "%1 unknown updates", number);
    }
}

QString KpkStrings::infoUpdate(Package::State state, int updates, int selected)
{
    if (updates == selected) {
        switch (state) {
        case Package::StateLow :
            return i18np("1 trivial update selected", "%1 trivial updates selected", updates);
        case Package::StateNormal :
            return i18ncp("Type of update, in the case it's just an update",
                          "1 update selected",
                          "%1 updates selected", updates);
        case Package::StateImportant :
            return i18np("1 important update selected", "%1 important updates selected", updates);
        case Package::StateSecurity :
            return i18np("1 security update selected", "%1 security updates selected", updates);
        case Package::StateBugfix :
            return i18np("1 bug fix update selected", "%1 bug fix updates selected", updates);
        case Package::StateEnhancement :
            return i18np("1 enhancement update selected", "%1 enhancement updates selected", updates);
        case Package::StateInstalled :
            return i18np("1 installed package selected to be removed",
                         "%1 installed packages selected to be removed", updates);
        case Package::StateAvailable :
            return i18np("1 available package selected to be installed",
                         "%1 available packages selected to be installed", updates);
        default : // In this case we don't want to map all enums
            kWarning() << "update info unrecognised: " << state;
            return i18np("1 unknown update", "%1 unknown updates", updates);
        }
    } else if (selected == 0) {
        return infoUpdate(state, updates);
    } else {
        switch (state) {
        case Package::StateLow :
            return i18np("%1 trivial update", "%1 trivial updates, %2 selected", updates, selected);
        case Package::StateNormal :
            return i18ncp("Type of update, in the case it's just an update",
                          "%1 update", "%1 updates, %2 selected", updates, selected);
        case Package::StateImportant :
            return i18np("%1 important update", "%1 important updates, %2 selected", updates, selected);
        case Package::StateSecurity :
            return i18np("%1 security update", "%1 security updates, %2 selected", updates, selected);
        case Package::StateBugfix :
            return i18np("%1 bug fix update", "%1 bug fix updates, %2 selected", updates, selected);
        case Package::StateEnhancement :
            return i18np("%1 enhancement update", "%1 enhancement updates, %2 selected", updates, selected);
        case Package::StateBlocked :
            // Blocked updates aren't selectable
            return i18np("%1 blocked update", "%1 blocked updates", updates);
        case Package::StateInstalled :
            return i18np("%1 installed package", "%1 installed packages, %2 selected to be removed", updates, selected);
        case Package::StateAvailable :
            return i18np("%1 available package", "%1 available packages, %2 selected to be installed", updates, selected);
        default : // In this case we don't want to map all enums
            kWarning() << "update info unrecognised: " << state;
            return i18np("%1 unknown update", "%1 unknown updates", updates);
        }
    }
}

QString KpkStrings::restartTypeFuture(Client::RestartType value)
{
    switch (value) {
    case Client::RestartNone :
        return i18n("No restart is necessary");
    case Client::RestartApplication :
        return i18n("You will be required to restart this application");
    case Client::RestartSession :
        return i18n("You will be required to log out and back in");
    case Client::RestartSystem :
        return i18n("A restart will be required");
    case Client::RestartSecuritySession :
        return i18n("You will be required to log out and back in due to a security update.");
    case Client::RestartSecuritySystem :
        return i18n("A restart will be required due to a security update.");
    case Client::UnknownRestartType :
        kWarning() << "restartTypeFuture(Client::UnknownRestartType)";
        return QString();
    }
    kWarning() << "restart unrecognised: " << value;
    return QString();
}

QString KpkStrings::restartType(Client::RestartType value)
{
    switch (value) {
    case Client::RestartNone :
        return i18n("No restart is required");
    case Client::RestartSystem :
        return i18n("A restart is required");
    case Client::RestartSession :
        return i18n("You need to log out and log back in");
    case Client::RestartApplication :
        return i18n("You need to restart the application");
    case Client::RestartSecuritySession :
        return i18n("You need to log out and log back in to remain secure.");
    case Client::RestartSecuritySystem :
        return i18n("A restart is required to remain secure.");
    case Client::UnknownRestartType :
        kWarning() << "restartType(Client::UnknownRestartType)";
        return QString();
    }
    kWarning() << "restart unrecognised: " << value;
    return QString();
}

QString KpkStrings::updateState(Client::UpdateState value)
{
    switch (value) {
    case Client::UpdateStable :
        return i18n("Stable");
    case Client::UpdateUnstable :
        return i18n("Unstable");
    case Client::UpdateTesting :
        return i18n("Testing");
    case Client::UnknownUpdateState :
        kWarning() << "updateState(Client::UnknownUpdateState)";
        return QString();
    }
    kWarning() << "value unrecognised: " << value;
    return QString();
}

QString KpkStrings::mediaMessage(Transaction::MediaType value, const QString &text)
{
    switch (value) {
    case Transaction::MediaCd :
        return i18n("Please insert the CD labeled '%1', and press continue.", text);
    case Transaction::MediaDvd :
        return i18n("Please insert the DVD labeled '%1', and press continue.", text);
    case Transaction::MediaDisc :
        return i18n("Please insert the disc labeled '%1', and press continue.", text);
    case Transaction::UnknownMediaType :
        return i18n("Please insert the medium labeled '%1', and press continue.", text);
    }
    kWarning() << "value unrecognised: " << value;
    return i18n("Please insert the medium labeled '%1', and press continue.", text);
}

QString KpkStrings::message(PackageKit::Client::MessageType value)
{
    switch (value) {
    case Client::MessageBrokenMirror :
        return i18n("A mirror is possibly broken");
    case Client::MessageConnectionRefused :
        return i18n("The connection was refused");
    case Client::MessageParameterInvalid :
        return i18n("The parameter was invalid");
    case Client::MessagePriorityInvalid :
        return i18n("The priority was invalid");
    case Client::MessageBackendError :
        return i18n("Backend warning");
    case Client::MessageDaemonError :
        return i18n("Daemon warning");
    case Client::MessageCacheBeingRebuilt :
        return i18n("The package list cache is being rebuilt");
    case Client::MessageUntrustedPackage :
        return i18n("An untrusted package was installed");
    case Client::MessageNewerPackageExists :
        return i18n("A newer package exists");
    case Client::MessageCouldNotFindPackage :
        return i18n("Could not find package");
    case Client::MessageConfigFilesChanged :
        return i18n("Configuration files were changed");
    case Client::MessagePackageAlreadyInstalled :
        return i18n("Package is already installed");
    case Client::MessageAutoremoveIgnored :
        return i18n("Automatic cleanup is being ignored");
    case Client::UnknownMessageType :
        kWarning() << "message(Client::UnknownMessageType)";
        return QString();
    }
    kWarning() << "value unrecognised: " << value;
    return QString();
}

QString KpkStrings::daemonError(PackageKit::Client::DaemonError value)
{
    switch (value) {
    case Client::ErrorFailedAuth :
        return i18n("You don't have the necessary privileges to perform this action.");
    case Client::ErrorNoTid :
        return i18n("Could not get a transaction id from packagekitd.");
    case Client::ErrorAlreadyTid :
        return i18n("Cannot connect to this transaction id.");
    case Client::ErrorRoleUnkown :
        return i18n("This action is unknown.");
    case Client::ErrorCannotStartDaemon :
        return i18n("The packagekitd service could not be started.");
    case Client::ErrorInvalidInput :
        return i18n("The query is not valid.");
    case Client::ErrorInvalidFile :
        return i18n("The file is not valid.");
    case Client::ErrorFunctionNotSupported :
        return i18n("This function is not yet supported.");
    case Client::ErrorDaemonUnreachable :
        return i18n("Could not talk to packagekitd.");
    case Client::NoError :
    case Client::ErrorFailed :
    case Client::UnkownError :
        return i18n("An unknown error happened.");
    }
    kWarning() << "value unrecognised: " << value;
    return i18n("An unknown error happened.");
}
