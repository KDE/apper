/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
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

QString KpkStrings::status(PackageKit::Enum::Status status)
{
    switch (status) {
    case Enum::LastStatus :
    case Enum::UnknownStatus :
        return i18nc("This is when the transaction status is not known",
                     "Unknown state");
    case Enum::StatusSetup :
        return i18nc("transaction state, the daemon is in the process of starting",
                     "Waiting for service to start");
    case Enum::StatusWait :
        return i18nc("transaction state, the transaction is waiting for another to complete",
                     "Waiting for other tasks");
    case Enum::StatusRunning :
        return i18nc("transaction state, just started",
                     "Running task");
    case Enum::StatusQuery :
        return i18nc("transaction state, is querying data",
                     "Querying");
    case Enum::StatusInfo :
        return i18nc("transaction state, getting data from a server",
                     "Getting information");
    case Enum::StatusRemove :
        return i18nc("transaction state, removing packages",
                     "Removing packages");
    case Enum::StatusDownload :
        return i18nc("transaction state, downloading package files",
                     "Downloading packages");
    case Enum::StatusInstall :
        return i18nc("transaction state, installing packages",
                     "Installing packages");
    case Enum::StatusRefreshCache :
        return i18nc("transaction state, refreshing internal lists",
                     "Refreshing software list");
    case Enum::StatusUpdate :
        return i18nc("transaction state, installing updates",
                     "Updating packages");
    case Enum::StatusCleanup :
        return i18nc("transaction state, removing old packages, and cleaning config files",
                     "Cleaning up packages");
    case Enum::StatusObsolete :
        return i18nc("transaction state, obsoleting old packages",
                     "Obsoleting packages");
    case Enum::StatusDepResolve :
        return i18nc("transaction state, checking the transaction before we do it",
                     "Resolving dependencies");
    case Enum::StatusSigCheck :
        return i18nc("transaction state, checking if we have all the security keys for the operation",
                     "Checking signatures");
    case Enum::StatusRollback :
        return i18nc("transaction state, when we return to a previous system state",
                     "Rolling back");
    case Enum::StatusTestCommit :
        return i18nc("transaction state, when we're doing a test transaction",
                     "Testing changes");
    case Enum::StatusCommit :
        return i18nc("transaction state, when we're writing to the system package database",
                     "Committing changes");
    case Enum::StatusRequest :
        return i18nc("transaction state, requesting data from a server",
                     "Requesting data");
    case Enum::StatusFinished :
        return i18nc("transaction state, all done!",
                     "Finished");
    case Enum::StatusCancel :
        return i18nc("transaction state, in the process of cancelling",
                     "Cancelling");
    case Enum::StatusDownloadRepository :
        return i18nc("transaction state, downloading metadata",
                     "Downloading repository information");
    case Enum::StatusDownloadPackagelist :
        return i18nc("transaction state, downloading metadata",
                     "Downloading list of packages");
    case Enum::StatusDownloadFilelist :
        return i18nc("transaction state, downloading metadata",
                     "Downloading file lists");
    case Enum::StatusDownloadChangelog :
        return i18nc("transaction state, downloading metadata",
                     "Downloading lists of changes");
    case Enum::StatusDownloadGroup :
        return i18nc("transaction state, downloading metadata",
                     "Downloading groups");
    case Enum::StatusDownloadUpdateinfo :
        return i18nc("transaction state, downloading metadata",
                     "Downloading update information");
    case Enum::StatusRepackaging :
        return i18nc("transaction state, repackaging delta files",
                     "Repackaging files");
    case Enum::StatusLoadingCache :
        return i18nc("transaction state, loading databases",
                     "Loading cache");
    case Enum::StatusScanApplications :
        return i18nc("transaction state, scanning for running processes",
                     "Scanning installed applications");
    case Enum::StatusGeneratePackageList :
        return i18nc("transaction state, generating a list of packages installed on the system",
                     "Generating package lists");
    case Enum::StatusWaitingForLock :
        return i18nc("transaction state, when we're waiting for the native tools to exit",
                     "Waiting for package manager lock");
    case Enum::StatusWaitingForAuth :
        return i18nc("waiting for user to type in a password",
                     "Waiting for authentication");
    case Enum::StatusScanProcessList :
        return i18nc("we are updating the list of processes",
                     "Updating the list of running applications");
    case Enum::StatusCheckExecutableFiles :
        return i18nc("we are checking executable files in use",
                     "Checking for applications currently in use");
    case Enum::StatusCheckLibraries :
        return i18nc("we are checking for libraries in use",
                     "Checking for libraries currently in use");
    case Enum::StatusCopyFiles :
        return i18nc("we are copying package files to prepare to install",
                     "Copying files");
    }
    kWarning() << "status unrecognised: " << status;
    return QString();
}

QString KpkStrings::statusPast(PackageKit::Enum::Status status)
{
    switch (status) {
    case Enum::StatusDownload:
        return i18nc("The action of the package, in past tense", "Downloaded");
    case Enum::StatusUpdate:
        return i18nc("The action of the package, in past tense", "Updated");
    case Enum::StatusInstall:
        return i18nc("The action of the package, in past tense", "Installed");
    case Enum::StatusRemove:
        return i18nc("The action of the package, in past tense", "Removed");
    case Enum::StatusCleanup:
        return i18nc("The action of the package, in past tense", "Cleaned Up");
    case Enum::StatusObsolete:
        return i18nc("The action of the package, in past tense", "Obsoleted");
    default : // In this case we don't want to map all enums
        kWarning() << "status unrecognised: " << status;
        return QString();
    }
}

QString KpkStrings::action(Enum::Role action)
{
    switch (action) {
    case Enum::LastRole :
    case Enum::UnknownRole :
        return i18nc("The role of the transaction, in present tense", "Unknown role type");
    case Enum::RoleGetDepends :
        return i18nc("The role of the transaction, in present tense", "Getting dependencies");
    case Enum::RoleGetUpdateDetail :
        return i18nc("The role of the transaction, in present tense", "Getting update detail");
    case Enum::RoleGetDetails :
        return i18nc("The role of the transaction, in present tense", "Getting details");
    case Enum::RoleGetRequires :
        return i18nc("The role of the transaction, in present tense", "Getting requires");
    case Enum::RoleGetUpdates :
        return i18nc("The role of the transaction, in present tense", "Getting updates");
    case Enum::RoleSearchDetails :
        return i18nc("The role of the transaction, in present tense", "Searching details");
    case Enum::RoleSearchFile :
        return i18nc("The role of the transaction, in present tense", "Searching for file");
    case Enum::RoleSearchGroup :
        return i18nc("The role of the transaction, in present tense", "Searching groups");
    case Enum::RoleSearchName :
        return i18nc("The role of the transaction, in present tense", "Searching by package name");
    case Enum::RoleRemovePackages :
        return i18nc("The role of the transaction, in present tense", "Removing");
    case Enum::RoleInstallPackages :
        return i18nc("The role of the transaction, in present tense", "Installing");
    case Enum::RoleInstallFiles :
        return i18nc("The role of the transaction, in present tense", "Installing file");
    case Enum::RoleRefreshCache :
        return i18nc("The role of the transaction, in present tense", "Refreshing package cache");
    case Enum::RoleUpdatePackages :
        return i18nc("The role of the transaction, in present tense", "Updating packages");
    case Enum::RoleUpdateSystem :
        return i18nc("The role of the transaction, in present tense", "Updating system");
    case Enum::RoleCancel :
        return i18nc("The role of the transaction, in present tense", "Canceling");
    case Enum::RoleRollback :
        return i18nc("The role of the transaction, in present tense", "Rolling back");
    case Enum::RoleGetRepoList :
        return i18nc("The role of the transaction, in present tense", "Getting list of repositories");
    case Enum::RoleRepoEnable :
        return i18nc("The role of the transaction, in present tense", "Enabling repository");
    case Enum::RoleRepoSetData :
        return i18nc("The role of the transaction, in present tense", "Setting repository data");
    case Enum::RoleResolve :
        return i18nc("The role of the transaction, in present tense", "Resolving");
    case Enum::RoleGetFiles :
        return i18nc("The role of the transaction, in present tense", "Getting file list");
    case Enum::RoleWhatProvides :
        return i18nc("The role of the transaction, in present tense", "Getting what provides");
    case Enum::RoleInstallSignature :
        return i18nc("The role of the transaction, in present tense", "Installing signature");
    case Enum::RoleGetPackages :
        return i18nc("The role of the transaction, in present tense", "Getting package lists");
    case Enum::RoleAcceptEula :
        return i18nc("The role of the transaction, in present tense", "Accepting EULA");
    case Enum::RoleDownloadPackages :
        return i18nc("The role of the transaction, in present tense", "Downloading packages");
    case Enum::RoleGetDistroUpgrades :
        return i18nc("The role of the transaction, in present tense", "Getting distribution upgrade information");
    case Enum::RoleGetCategories :
        return i18nc("The role of the transaction, in present tense", "Getting categories");
    case Enum::RoleGetOldTransactions :
        return i18nc("The role of the transaction, in present tense", "Getting old transactions");
    case Enum::RoleSimulateInstallFiles :
        return i18nc("The role of the transaction, in present tense", "Simulating the install of files");
    case Enum::RoleSimulateInstallPackages :
        return i18nc("The role of the transaction, in present tense", "Simulating the install");
    case Enum::RoleSimulateRemovePackages :
        return i18nc("The role of the transaction, in present tense", "Simulating the remove");
    case Enum::RoleSimulateUpdatePackages :
        return i18nc("The role of the transaction, in present tense", "Simulating the update");
    }
    kWarning() << "action unrecognised: " << action;
    return QString();
}

QString KpkStrings::actionPast(Enum::Role action)
{
    switch (action) {
    case Enum::LastRole :
    case Enum::UnknownRole :
        return i18nc("The role of the transaction, in past tense", "Unknown role type");
    case Enum::RoleGetDepends :
        return i18nc("The role of the transaction, in past tense", "Got dependencies");
    case Enum::RoleGetUpdateDetail :
        return i18nc("The role of the transaction, in past tense", "Got update detail");
    case Enum::RoleGetDetails :
        return i18nc("The role of the transaction, in past tense", "Got details");
    case Enum::RoleGetRequires :
        return i18nc("The role of the transaction, in past tense", "Got requires");
    case Enum::RoleGetUpdates :
        return i18nc("The role of the transaction, in past tense", "Got updates");
    case Enum::RoleSearchDetails :
        return i18nc("The role of the transaction, in past tense", "Searched for package details");
    case Enum::RoleSearchFile :
        return i18nc("The role of the transaction, in past tense", "Searched for file");
    case Enum::RoleSearchGroup :
        return i18nc("The role of the transaction, in past tense", "Searched groups");
    case Enum::RoleSearchName :
        return i18nc("The role of the transaction, in past tense", "Searched for package name");
    case Enum::RoleRemovePackages :
        return i18nc("The role of the transaction, in past tense", "Removed packages");
    case Enum::RoleInstallPackages :
        return i18nc("The role of the transaction, in past tense", "Installed packages");
    case Enum::RoleInstallFiles :
        return i18nc("The role of the transaction, in past tense", "Installed local files");
    case Enum::RoleRefreshCache :
        return i18nc("The role of the transaction, in past tense", "Refreshed package cache");
    case Enum::RoleUpdatePackages :
        return i18nc("The role of the transaction, in past tense", "Updated packages");
    case Enum::RoleUpdateSystem :
        return i18nc("The role of the transaction, in past tense", "Updated system");
    case Enum::RoleCancel :
        return i18nc("The role of the transaction, in past tense", "Canceled");
    case Enum::RoleRollback :
        return i18nc("The role of the transaction, in past tense", "Rolled back");
    case Enum::RoleGetRepoList :
        return i18nc("The role of the transaction, in past tense", "Got list of repositories");
    case Enum::RoleRepoEnable :
        return i18nc("The role of the transaction, in past tense", "Enabled repository");
    case Enum::RoleRepoSetData :
        return i18nc("The role of the transaction, in past tense", "Set repository data");
    case Enum::RoleResolve :
        return i18nc("The role of the transaction, in past tense", "Resolved");
    case Enum::RoleGetFiles :
        return i18nc("The role of the transaction, in past tense", "Got file list");
    case Enum::RoleWhatProvides :
        return i18nc("The role of the transaction, in past tense", "Got what provides");
    case Enum::RoleInstallSignature :
        return i18nc("The role of the transaction, in past tense", "Installed signature");
    case Enum::RoleGetPackages :
        return i18nc("The role of the transaction, in past tense", "Got package lists");
    case Enum::RoleAcceptEula :
        return i18nc("The role of the transaction, in past tense", "Accepted EULA");
    case Enum::RoleDownloadPackages :
        return i18nc("The role of the transaction, in past tense", "Downloaded packages");
    case Enum::RoleGetDistroUpgrades :
        return i18nc("The role of the transaction, in past tense", "Got distribution upgrades");
    case Enum::RoleGetCategories :
        return i18nc("The role of the transaction, in past tense", "Got categories");
    case Enum::RoleGetOldTransactions :
        return i18nc("The role of the transaction, in past tense", "Got old transactions");
    case Enum::RoleSimulateInstallFiles :
        return i18nc("The role of the transaction, in past tense", "Simulated the install of files");
    case Enum::RoleSimulateInstallPackages :
        return i18nc("The role of the transaction, in past tense", "Simulated the install");
    case Enum::RoleSimulateRemovePackages :
        return i18nc("The role of the transaction, in past tense", "Simulated the remove");
    case Enum::RoleSimulateUpdatePackages :
        return i18nc("The role of the transaction, in past tense", "Simulated the update");
    }
    kWarning() << "action unrecognised: " << action;
    return QString();
}

QString KpkStrings::error(PackageKit::Enum::Error error)
{
    switch (error) {
    case Enum::ErrorNoNetwork :
        return i18n("No network connection available");
    case Enum::ErrorNoCache :
        return i18n("No package cache is available");
    case Enum::ErrorOom :
        return i18n("Out of memory");
    case Enum::ErrorCreateThreadFailed :
        return i18n("Failed to create a thread");
    case Enum::ErrorNotSupported :
        return i18n("Not supported by this backend");
    case Enum::ErrorInternalError :
        return i18n("An internal system error has occurred");
    case Enum::ErrorGpgFailure :
        return i18n("A security trust relationship is not present");
    case Enum::ErrorPackageNotInstalled :
        return i18n("The package is not installed");
    case Enum::ErrorPackageNotFound :
        return i18n("The package was not found");
    case Enum::ErrorPackageAlreadyInstalled :
        return i18n("The package is already installed");
    case Enum::ErrorPackageDownloadFailed :
        return i18n("The package download failed");
    case Enum::ErrorGroupNotFound :
        return i18n("The group was not found");
    case Enum::ErrorGroupListInvalid :
        return i18n("The group list was invalid");
    case Enum::ErrorDepResolutionFailed :
        return i18n("Dependency resolution failed");
    case Enum::ErrorFilterInvalid :
        return i18n("Search filter was invalid");
    case Enum::ErrorPackageIdInvalid :
        return i18n("The package identifier was not well formed");
    case Enum::ErrorTransactionError :
        return i18n("Transaction error");
    case Enum::ErrorRepoNotFound :
        return i18n("Repository name was not found");
    case Enum::ErrorCannotRemoveSystemPackage :
        return i18n("Could not remove a protected system package");
    case Enum::ErrorTransactionCancelled :
        return i18n("The task was canceled");
    case Enum::ErrorProcessKill :
        return i18n("The task was forcibly canceled");
    case Enum::ErrorFailedConfigParsing :
        return i18n("Reading the config file failed");
    case Enum::ErrorCannotCancel :
        return i18n("The task cannot be cancelled");
    case Enum::ErrorCannotInstallSourcePackage :
        return i18n("Source packages cannot be installed");
    case Enum::ErrorNoLicenseAgreement :
        return i18n("The license agreement failed");
    case Enum::ErrorFileConflicts :
        return i18n("Local file conflict between packages");
    case Enum::ErrorPackageConflicts :
        return i18n("Packages are not compatible");
    case Enum::ErrorRepoNotAvailable :
        return i18n("Problem connecting to a software origin");
    case Enum::ErrorFailedInitialization :
        return i18n("Failed to initialize");
    case Enum::ErrorFailedFinalise :
        return i18n("Failed to finalize");
    case Enum::ErrorCannotGetLock :
        return i18n("Cannot get lock");
    case Enum::ErrorNoPackagesToUpdate :
        return i18n("No packages to update");
    case Enum::ErrorCannotWriteRepoConfig :
        return i18n("Cannot write repository configuration");
    case Enum::ErrorLocalInstallFailed :
        return i18n("Local install failed");
    case Enum::ErrorBadGpgSignature :
        return i18n("Bad GPG signature");
    case Enum::ErrorMissingGpgSignature :
        return i18n("Missing GPG signature");
    case Enum::ErrorRepoConfigurationError :
        return i18n("Repository configuration invalid");
    case Enum::ErrorInvalidPackageFile :
        return i18n("Invalid package file");
    case Enum::ErrorPackageInstallBlocked :
        return i18n("Package install blocked");
    case Enum::ErrorPackageCorrupt :
        return i18n("Package is corrupt");
    case Enum::ErrorAllPackagesAlreadyInstalled :
        return i18n("All packages are already installed");
    case Enum::ErrorFileNotFound :
        return i18n("The specified file could not be found");
    case Enum::ErrorNoMoreMirrorsToTry :
        return i18n("No more mirrors are available");
    case Enum::ErrorNoDistroUpgradeData :
        return i18n("No distribution upgrade data is available");
    case Enum::ErrorIncompatibleArchitecture :
        return i18n("Package is incompatible with this system");
    case Enum::ErrorNoSpaceOnDevice :
        return i18n("No space is left on the disk");
    case Enum::ErrorMediaChangeRequired :
        return i18n("A media change is required");
    case Enum::ErrorNotAuthorized :
        return i18n("Authorization failed");
    case Enum::ErrorUpdateNotFound :
        return i18n("Update not found");
    case Enum::ErrorCannotInstallRepoUnsigned :
        return i18n("Cannot install from untrusted origin");
    case Enum::ErrorCannotUpdateRepoUnsigned :
        return i18n("Cannot update from untrusted origin");
    case Enum::ErrorCannotGetFilelist :
        return i18n("Cannot get the file list");
    case Enum::ErrorCannotGetRequires :
        return i18n("Cannot get package requires");
    case Enum::ErrorCannotDisableRepository :
        return i18n("Cannot disable origin");
    case Enum::ErrorRestrictedDownload :
        return i18n("The download failed");
    case Enum::ErrorPackageFailedToConfigure :
        return i18n("Package failed to configure");
    case Enum::ErrorPackageFailedToBuild :
        return i18n("Package failed to build");
    case Enum::ErrorPackageFailedToInstall :
        return i18n("Package failed to install");
    case Enum::ErrorPackageFailedToRemove :
        return i18n("Package failed to be removed");
    case Enum::ErrorUpdateFailedDueToRunningProcess :
        return i18n("Update failed due to running process");
    case Enum::ErrorPackageDatabaseChanged :
        return i18n("The package database was changed");
    case Enum::LastError :
    case Enum::UnknownError :
        return i18n("Unknown error");
    }
    kWarning() << "error unrecognised: " << error;
    return QString();
}

QString KpkStrings::errorMessage(PackageKit::Enum::Error error)
{
    switch (error) {
    case Enum::ErrorNoNetwork :
        return i18n("There is no network connection available.\n"
                    "Please check your connection settings and try again");
    case Enum::ErrorNoCache :
        return i18n("The package list needs to be rebuilt.\n"
                    "This should have been done by the backend automatically.");
    case Enum::ErrorOom :
        return i18n("The service that is responsible for handling user requests is out of memory.\n"
                    "Please close some programs or restart your computer.");
    case Enum::ErrorCreateThreadFailed :
        return i18n("A thread could not be created to service the user request.");
    case Enum::ErrorNotSupported :
        return i18n("The action is not supported by this backend.\n"
                    "Please report a bug as this should not have happened.");
    case Enum::ErrorInternalError :
        return i18n("A problem that we were not expecting has occurred.\n"
                    "Please report this bug with the error description.");
    case Enum::ErrorGpgFailure :
        return i18n("A security trust relationship could not be made with the software origin.\n"
                    "Please check your software signature settings.");
    case Enum::ErrorPackageNotInstalled :
        return i18n("The package that is trying to be removed or updated is not already installed.");
    case Enum::ErrorPackageNotFound :
        return i18n("The package that is being modified was not found on your system or in any software origin.");
    case Enum::ErrorPackageAlreadyInstalled :
        return i18n("The package that is trying to be installed is already installed.");
    case Enum::ErrorPackageDownloadFailed :
        return i18n("The package download failed.\n"
                    "Please check your network connectivity.");
    case Enum::ErrorGroupNotFound :
        return i18n("The group type was not found.\n"
                    "Please check your group list and try again.");
    case Enum::ErrorGroupListInvalid :
        return i18n("The group list could not be loaded.\n"
                    "Refreshing your cache may help, although this is normally a software "
                    "origin error.");
    case Enum::ErrorDepResolutionFailed :
        return i18n("A package dependency could not be found.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorFilterInvalid :
        return i18n("The search filter was not correctly formed.");
    case Enum::ErrorPackageIdInvalid :
        return i18n("The package identifier was not well formed when sent to the system daemon.\n"
                    "This normally indicates an internal bug and should be reported.");
    case Enum::ErrorTransactionError :
        return i18n("An error occurred while running the transaction.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorRepoNotFound :
        return i18n("The remote software origin name was not found.\n"
                    "You may need to enable an item in Software Origins.");
    case Enum::ErrorCannotRemoveSystemPackage :
        return i18n("Removing a protected system package is not allowed.");
    case Enum::ErrorTransactionCancelled :
        return i18n("The task was canceled successfully and no packages were changed.");
    case Enum::ErrorProcessKill :
        return i18n("The task was canceled successfully and no packages were changed.\n"
                    "The backend did not exit cleanly.");
    case Enum::ErrorFailedConfigParsing :
        return i18n("The native package configuration file could not be opened.\n"
                    "Please make sure your system's configuration is valid.");
    case Enum::ErrorCannotCancel :
        return i18n("The task is not safe to be cancelled at this time.");
    case Enum::ErrorCannotInstallSourcePackage :
        return i18n("Source packages are not normally installed this way.\n"
                    "Check the extension of the file you are trying to install.");
    case Enum::ErrorNoLicenseAgreement :
        return i18n("The license agreement was not agreed to.\n"
                    "To use this software you have to accept the license.");
    case Enum::ErrorFileConflicts :
        return i18n("Two packages provide the same file.\n"
                    "This is usually due to mixing packages for different software origins.");
    case Enum::ErrorPackageConflicts :
        return i18n("Multiple packages exist that are not compatible with each other.\n"
                    "This is usually due to mixing packages from different software origins.");
    case Enum::ErrorRepoNotAvailable :
        return i18n("There was a (possibly temporary) problem connecting to a software origins.\n"
                    "Please check the detailed error for further details.");
    case Enum::ErrorFailedInitialization :
        return i18n("Failed to initialize packaging backend.\n"
                    "This may occur if other packaging tools are being used simultaneously.");
    case Enum::ErrorFailedFinalise :
        return i18n("Failed to close down the backend instance.\n"
                    "This error can normally be ignored.");
    case Enum::ErrorCannotGetLock :
        return i18n("Cannot get the exclusive lock on the packaging backend.\n"
                    "Please close any other legacy packaging tools that may be open.");
    case Enum::ErrorNoPackagesToUpdate :
        return i18n("None of the selected packages could be updated.");
    case Enum::ErrorCannotWriteRepoConfig :
        return i18n("The repository configuration could not be modified.");
    case Enum::ErrorLocalInstallFailed :
        return i18n("Installing the local file failed.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorBadGpgSignature :
        return i18n("The package signature could not be verified.");
    case Enum::ErrorMissingGpgSignature :
        return i18n("The package signature was missing and this package is untrusted.\n"
                    "This package was not signed with a GPG key when created.");
    case Enum::ErrorRepoConfigurationError :
        return i18n("Repository configuration was invalid and could not be read.");
    case Enum::ErrorInvalidPackageFile :
        return i18n("The package you are attempting to install is not valid.\n"
                    "The package file could be corrupt, or not a proper package.");
    case Enum::ErrorPackageInstallBlocked :
        return i18n("Installation of this package was prevented by your packaging system's configuration.");
    case Enum::ErrorPackageCorrupt :
        return i18n("The package that was downloaded is corrupt and needs to be downloaded again.");
    case Enum::ErrorAllPackagesAlreadyInstalled :
        return i18n("All of the packages selected for install are already installed on the system.");
    case Enum::ErrorFileNotFound :
        return i18n("The specified file could not be found on the system.\n"
                    "Check that the file still exists and has not been deleted.");
    case Enum::ErrorNoMoreMirrorsToTry :
        return i18n("Required data could not be found on any of the configured software origins.\n"
                    "There were no more download mirrors that could be tried.");
    case Enum::ErrorNoDistroUpgradeData :
        return i18n("Required upgrade data could not be found in any of the configured software origins.\n"
                    "The list of distribution upgrades will be unavailable.");
    case Enum::ErrorIncompatibleArchitecture :
        return i18n("The package that is trying to be installed is incompatible with this system.");
    case Enum::ErrorNoSpaceOnDevice :
        return i18n("There is insufficient space on the device.\n"
                    "Free some space on the system disk to perform this operation.");
    case Enum::ErrorMediaChangeRequired :
        return i18n("Additional media is required to complete the transaction.");
    case Enum::ErrorNotAuthorized :
        return i18n("You have failed to provide correct authentication.\n"
                    "Please check any passwords or account settings.");
    case Enum::ErrorUpdateNotFound :
        return i18n("The specified update could not be found.\n"
                    "It could have already been installed or no longer available on the remote server.");
    case Enum::ErrorCannotInstallRepoUnsigned :
        return i18n("The package could not be installed from untrusted origin.");
    case Enum::ErrorCannotUpdateRepoUnsigned :
        return i18n("The package could not be updated from untrusted origin.");
    case Enum::ErrorCannotGetFilelist :
        return i18n("The file list is not available for this package.");
    case Enum::ErrorCannotGetRequires :
        return i18n("The information about what requires this package could not be obtained.");
    case Enum::ErrorCannotDisableRepository :
        return i18n("The specified software origin could not be disabled.");
    case Enum::ErrorRestrictedDownload :
        return i18n("The download could not be done automatically and should be done manually.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorPackageFailedToConfigure :
        return i18n("One of the selected packages failed to configure correctly.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorPackageFailedToBuild :
        return i18n("One of the selected packages failed to build correctly.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorPackageFailedToInstall :
        return i18n("One of the selected packages failed to install correctly.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorPackageFailedToRemove :
        return i18n("One of the selected packages failed to be removed correctly.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorUpdateFailedDueToRunningProcess :
        return i18n("A program is running that has to be closed before the update can proceed.\n"
                    "More information is available in the detailed report.");
    case Enum::ErrorPackageDatabaseChanged :
        return i18n("The package database was changed while the request was running.");
    case Enum::LastError :
    case Enum::UnknownError :
        return i18n("Unknown error, please report a bug.\n"
                    "More information is available in the detailed report.");
    }
    kWarning() << "error unrecognised: " << error;
    return QString();
}

QString KpkStrings::groups(Enum::Group group)
{
    switch (group) {
    case Enum::GroupAccessibility :
        return i18nc("The group type", "Accessibility");
    case Enum::GroupAccessories :
        return i18nc("The group type", "Accessories");
    case Enum::GroupEducation :
        return i18nc("The group type", "Education");
    case Enum::GroupGames :
        return i18nc("The group type", "Games");
    case Enum::GroupGraphics :
        return i18nc("The group type", "Graphics");
    case Enum::GroupInternet :
        return i18nc("The group type", "Internet");
    case Enum::GroupOffice :
        return i18nc("The group type", "Office");
    case Enum::GroupOther :
        return i18nc("The group type", "Others");
    case Enum::GroupProgramming :
        return i18nc("The group type", "Development");
    case Enum::GroupMultimedia :
        return i18nc("The group type", "Multimedia");
    case Enum::GroupSystem :
        return i18nc("The group type", "System");
    case Enum::GroupDesktopGnome :
        return i18nc("The group type", "GNOME desktop");
    case Enum::GroupDesktopKde :
        return i18nc("The group type", "KDE desktop");
    case Enum::GroupDesktopXfce :
        return i18nc("The group type", "XFCE desktop");
    case Enum::GroupDesktopOther :
        return i18nc("The group type", "Other desktops");
    case Enum::GroupPublishing :
        return i18nc("The group type", "Publishing");
    case Enum::GroupServers :
        return i18nc("The group type", "Servers");
    case Enum::GroupFonts :
        return i18nc("The group type", "Fonts");
    case Enum::GroupAdminTools :
        return i18nc("The group type", "Admin tools");
    case Enum::GroupLegacy :
        return i18nc("The group type", "Legacy");
    case Enum::GroupLocalization :
        return i18nc("The group type", "Localization");
    case Enum::GroupVirtualization :
        return i18nc("The group type", "Virtualization");
    case Enum::GroupSecurity :
        return i18nc("The group type", "Security");
    case Enum::GroupPowerManagement :
        return i18nc("The group type", "Power management");
    case Enum::GroupCommunication :
        return i18nc("The group type", "Communication");
    case Enum::GroupNetwork :
        return i18nc("The group type", "Network");
    case Enum::GroupMaps :
        return i18nc("The group type", "Maps");
    case Enum::GroupRepos :
        return i18nc("The group type", "Software sources");
    case Enum::GroupScience :
        return i18nc("The group type", "Science");
    case Enum::GroupDocumentation :
        return i18nc("The group type", "Documentation");
    case Enum::GroupElectronics :
        return i18nc("The group type", "Electronics");
    case Enum::GroupCollections ://TODO check this one
        return i18nc("The group type", "Package collections");
    case Enum::GroupVendor :
        return i18nc("The group type", "Vendor");
    case Enum::GroupNewest :
        return i18nc("The group type", "Newest packages");
    case Enum::LastGroup :
    case Enum::UnknownGroup :
        return i18nc("The group type", "Unknown group");
    }
    kWarning() << "group unrecognised: " << group;
    return QString();
}

QString KpkStrings::info(Enum::Info state)
{
    switch (state) {
    case Enum::InfoLow :
        return i18nc("The type of update", "Trivial update");
    case Enum::InfoNormal :
        return i18nc("The type of update", "Normal update");
    case Enum::InfoImportant :
        return i18nc("The type of update", "Important update");
    case Enum::InfoSecurity :
        return i18nc("The type of update", "Security update");
    case Enum::InfoBugfix :
        return i18nc("The type of update", "Bug fix update");
    case Enum::InfoEnhancement :
        return i18nc("The type of update", "Enhancement update");
    case Enum::InfoBlocked :
        return i18nc("The type of update", "Blocked update");
    case Enum::InfoInstalled :
    case Enum::InfoCollectionInstalled :
        return i18nc("The type of update", "Installed");
    case Enum::InfoAvailable :
    case Enum::InfoCollectionAvailable :
        return i18nc("The type of update", "Available");
    case Enum::UnknownInfo :
        return i18nc("The type of update", "Unknown update");
    default : // In this case we don't want to map all enums
        kWarning() << "info unrecognised: " << state;
        return QString();
    }
}

QString KpkStrings::infoUpdate(Enum::Info state, int number)
{
    switch (state) {
    case Enum::InfoLow :
        return i18np("1 trivial update", "%1 trivial updates", number);
    case Enum::InfoNormal :
        return i18ncp("Type of update, in the case it's just an update", "1 update", "%1 updates", number);
    case Enum::InfoImportant :
        return i18np("1 important update", "%1 important updates", number);
    case Enum::InfoSecurity :
        return i18np("1 security update", "%1 security updates", number);
    case Enum::InfoBugfix :
        return i18np("1 bug fix update", "%1 bug fix updates", number);
    case Enum::InfoEnhancement :
        return i18np("1 enhancement update", "%1 enhancement updates", number);
    case Enum::InfoBlocked :
        return i18np("1 blocked update", "%1 blocked updates", number);
    case Enum::InfoInstalled :
        return i18np("1 installed package", "%1 installed packages", number);
    case Enum::InfoAvailable :
        return i18np("1 available package", "%1 available packages", number);
    default : // In this case we don't want to map all enums
        kWarning() << "update info unrecognised: " << state;
        return i18np("1 unknown update", "%1 unknown updates", number);
    }
}

QString KpkStrings::infoUpdate(Enum::Info state, int updates, int selected)
{
    if (updates == selected) {
        switch (state) {
        case Enum::InfoLow :
            return i18np("1 trivial update selected", "%1 trivial updates selected", updates);
        case Enum::InfoNormal :
            return i18ncp("Type of update, in the case it's just an update",
                          "1 update selected",
                          "%1 updates selected", updates);
        case Enum::InfoImportant :
            return i18np("1 important update selected", "%1 important updates selected", updates);
        case Enum::InfoSecurity :
            return i18np("1 security update selected", "%1 security updates selected", updates);
        case Enum::InfoBugfix :
            return i18np("1 bug fix update selected", "%1 bug fix updates selected", updates);
        case Enum::InfoEnhancement :
            return i18np("1 enhancement update selected", "%1 enhancement updates selected", updates);
        case Enum::InfoInstalled :
            return i18np("1 installed package selected to be removed",
                         "%1 installed packages selected to be removed", updates);
        case Enum::InfoAvailable :
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
        case Enum::InfoLow :
            return i18np("%1 trivial update", "%1 trivial updates, %2 selected", updates, selected);
        case Enum::InfoNormal :
            return i18ncp("Type of update, in the case it's just an update",
                          "%1 update", "%1 updates, %2 selected", updates, selected);
        case Enum::InfoImportant :
            return i18np("%1 important update", "%1 important updates, %2 selected", updates, selected);
        case Enum::InfoSecurity :
            return i18np("%1 security update", "%1 security updates, %2 selected", updates, selected);
        case Enum::InfoBugfix :
            return i18np("%1 bug fix update", "%1 bug fix updates, %2 selected", updates, selected);
        case Enum::InfoEnhancement :
            return i18np("%1 enhancement update", "%1 enhancement updates, %2 selected", updates, selected);
        case Enum::InfoBlocked :
            // Blocked updates aren't selectable
            return i18np("%1 blocked update", "%1 blocked updates", updates);
        case Enum::InfoInstalled :
            return i18np("%1 installed package", "%1 installed packages, %2 selected to be removed", updates, selected);
        case Enum::InfoAvailable :
            return i18np("%1 available package", "%1 available packages, %2 selected to be installed", updates, selected);
        default : // In this case we don't want to map all enums
            kWarning() << "update info unrecognised: " << state;
            return i18np("%1 unknown update", "%1 unknown updates", updates);
        }
    }
}

QString KpkStrings::restartTypeFuture(Enum::Restart value)
{
    switch (value) {
    case Enum::RestartNone :
        return i18n("No restart is necessary");
    case Enum::RestartApplication :
        return i18n("You will be required to restart this application");
    case Enum::RestartSession :
        return i18n("You will be required to log out and back in");
    case Enum::RestartSystem :
        return i18n("A restart will be required");
    case Enum::RestartSecuritySession :
        return i18n("You will be required to log out and back in due to a security update.");
    case Enum::RestartSecuritySystem :
        return i18n("A restart will be required due to a security update.");
    case Enum::LastRestart :
    case Enum::UnknownRestart :
        kWarning() << "restartTypeFuture(Enum::UnknownRestartType)";
        return QString();
    }
    kWarning() << "restart unrecognised: " << value;
    return QString();
}

QString KpkStrings::restartType(Enum::Restart value)
{
    switch (value) {
    case Enum::RestartNone :
        return i18n("No restart is required");
    case Enum::RestartSystem :
        return i18n("A restart is required");
    case Enum::RestartSession :
        return i18n("You need to log out and log back in");
    case Enum::RestartApplication :
        return i18n("You need to restart the application");
    case Enum::RestartSecuritySession :
        return i18n("You need to log out and log back in to remain secure.");
    case Enum::RestartSecuritySystem :
        return i18n("A restart is required to remain secure.");
    case Enum::LastRestart :
    case Enum::UnknownRestart :
        kWarning() << "restartType(Enum::UnknownRestartType)";
        return QString();
    }
    kWarning() << "restart unrecognised: " << value;
    return QString();
}

QString KpkStrings::updateState(Enum::UpdateState value)
{
    switch (value) {
    case Enum::UpdateStateStable :
        return i18n("Stable");
    case Enum::UpdateStateUnstable :
        return i18n("Unstable");
    case Enum::UpdateStateTesting :
        return i18n("Testing");
    case Enum::LastUpdateState :
    case Enum::UnknownUpdateState :
        kWarning() << "updateState(Enum::UnknownUpdateState)";
        return QString();
    }
    kWarning() << "value unrecognised: " << value;
    return QString();
}

QString KpkStrings::mediaMessage(Enum::MediaType value, const QString &text)
{
    switch (value) {
    case Enum::MediaTypeCd :
        return i18n("Please insert the CD labeled '%1', and press continue.", text);
    case Enum::MediaTypeDvd :
        return i18n("Please insert the DVD labeled '%1', and press continue.", text);
    case Enum::MediaTypeDisc :
        return i18n("Please insert the disc labeled '%1', and press continue.", text);
    case Enum::LastMediaType :
    case Enum::UnknownMediaType :
        return i18n("Please insert the medium labeled '%1', and press continue.", text);
    }
    kWarning() << "value unrecognised: " << value;
    return i18n("Please insert the medium labeled '%1', and press continue.", text);
}

QString KpkStrings::message(PackageKit::Enum::Message value)
{
    switch (value) {
    case Enum::MessageBrokenMirror :
        return i18n("A mirror is possibly broken");
    case Enum::MessageConnectionRefused :
        return i18n("The connection was refused");
    case Enum::MessageParameterInvalid :
        return i18n("The parameter was invalid");
    case Enum::MessagePriorityInvalid :
        return i18n("The priority was invalid");
    case Enum::MessageBackendError :
        return i18n("Backend warning");
    case Enum::MessageDaemonError :
        return i18n("Daemon warning");
    case Enum::MessageCacheBeingRebuilt :
        return i18n("The package list cache is being rebuilt");
    case Enum::MessageUntrustedPackage :
        return i18n("An untrusted package was installed");
    case Enum::MessageNewerPackageExists :
        return i18n("A newer package exists");
    case Enum::MessageCouldNotFindPackage :
        return i18n("Could not find package");
    case Enum::MessageConfigFilesChanged :
        return i18n("Configuration files were changed");
    case Enum::MessagePackageAlreadyInstalled :
        return i18n("Package is already installed");
    case Enum::MessageAutoremoveIgnored :
        return i18n("Automatic cleanup is being ignored");
    case Enum::MessageRepoMetadataDownloadFailed :
        return i18n("Software source download failed");
    case Enum::MessageRepoForDevelopersOnly :
        return i18n("This software source is for developers only");
    case Enum::LastMessage :
    case Enum::UnknownMessage :
        kWarning() << "message(Enum::UnknownMessageType)";
        return QString();
    }
    kWarning() << "value unrecognised: " << value;
    return QString();
}

QString KpkStrings::daemonError(PackageKit::Client::DaemonError value)
{
    switch (value) {
    case Client::ErrorFailedAuth :
        return i18n("You do not have the necessary privileges to perform this action.");
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
    case Client::LastDaemonError :
    case Client::UnkownError :
        return i18n("An unknown error happened.");
    }
    kWarning() << "value unrecognised: " << value;
    return i18n("An unknown error happened.");
}
