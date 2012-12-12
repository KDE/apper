/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include "PkStrings.h"

#include <KLocale>
#include <KGlobal>

#include <KDebug>

using namespace PackageKit;

QString PkStrings::status(int status, uint speed, qulonglong downloadRemaining)
{
    Transaction::Status statusEnum = static_cast<Transaction::Status>(status);
    switch (statusEnum) {
    case Transaction::StatusUnknown:
        return i18nc("This is when the transaction status is not known",
                     "Unknown state");
    case Transaction::StatusSetup:
        return i18nc("transaction state, the daemon is in the process of starting",
                     "Waiting for service to start");
    case Transaction::StatusWait:
        return i18nc("transaction state, the transaction is waiting for another to complete",
                     "Waiting for other tasks");
    case Transaction::StatusRunning :
        return i18nc("transaction state, just started",
                     "Running task");
    case Transaction::StatusQuery :
        return i18nc("transaction state, is querying data",
                     "Querying");
    case Transaction::StatusInfo :
        return i18nc("transaction state, getting data from a server",
                     "Getting information");
    case Transaction::StatusRemove :
        return i18nc("transaction state, removing packages",
                     "Removing packages");
    case Transaction::StatusDownload:
        if (speed != 0 && downloadRemaining != 0) {
            return i18nc("transaction state, downloading package files",
                         "Downloading at %1/s, %2 remaining",
                         KGlobal::locale()->formatByteSize(speed),
                         KGlobal::locale()->formatByteSize(downloadRemaining));
        } else if (speed != 0 && downloadRemaining == 0) {
            return i18nc("transaction state, downloading package files",
                         "Downloading at %1/s",
                         KGlobal::locale()->formatByteSize(speed));
        } else if (speed == 0 && downloadRemaining != 0) {
            return i18nc("transaction state, downloading package files",
                         "Downloading, %1 remaining",
                         KGlobal::locale()->formatByteSize(downloadRemaining));
        } else {
            return i18nc("transaction state, downloading package files",
                         "Downloading");
        }
    case Transaction::StatusInstall :
        return i18nc("transaction state, installing packages",
                     "Installing packages");
    case Transaction::StatusRefreshCache :
        return i18nc("transaction state, refreshing internal lists",
                     "Refreshing software list");
    case Transaction::StatusUpdate :
        return i18nc("transaction state, installing updates",
                     "Updating packages");
    case Transaction::StatusCleanup :
        return i18nc("transaction state, removing old packages, and cleaning config files",
                     "Cleaning up packages");
    case Transaction::StatusObsolete :
        return i18nc("transaction state, obsoleting old packages",
                     "Obsoleting packages");
    case Transaction::StatusDepResolve :
        return i18nc("transaction state, checking the transaction before we do it",
                     "Resolving dependencies");
    case Transaction::StatusSigCheck :
        return i18nc("transaction state, checking if we have all the security keys for the operation",
                     "Checking signatures");
    case Transaction::StatusTestCommit :
        return i18nc("transaction state, when we're doing a test transaction",
                     "Testing changes");
    case Transaction::StatusCommit :
        return i18nc("transaction state, when we're writing to the system package database",
                     "Committing changes");
    case Transaction::StatusRequest :
        return i18nc("transaction state, requesting data from a server",
                     "Requesting data");
    case Transaction::StatusFinished :
        return i18nc("transaction state, all done!",
                     "Finished");
    case Transaction::StatusCancel :
        return i18nc("transaction state, in the process of cancelling",
                     "Cancelling");
    case Transaction::StatusDownloadRepository :
        return i18nc("transaction state, downloading metadata",
                     "Downloading repository information");
    case Transaction::StatusDownloadPackagelist :
        return i18nc("transaction state, downloading metadata",
                     "Downloading list of packages");
    case Transaction::StatusDownloadFilelist :
        return i18nc("transaction state, downloading metadata",
                     "Downloading file lists");
    case Transaction::StatusDownloadChangelog :
        return i18nc("transaction state, downloading metadata",
                     "Downloading lists of changes");
    case Transaction::StatusDownloadGroup :
        return i18nc("transaction state, downloading metadata",
                     "Downloading groups");
    case Transaction::StatusDownloadUpdateinfo :
        return i18nc("transaction state, downloading metadata",
                     "Downloading update information");
    case Transaction::StatusRepackaging :
        return i18nc("transaction state, repackaging delta files",
                     "Repackaging files");
    case Transaction::StatusLoadingCache :
        return i18nc("transaction state, loading databases",
                     "Loading cache");
    case Transaction::StatusScanApplications :
        return i18nc("transaction state, scanning for running processes",
                     "Scanning installed applications");
    case Transaction::StatusGeneratePackageList :
        return i18nc("transaction state, generating a list of packages installed on the system",
                     "Generating package lists");
    case Transaction::StatusWaitingForLock :
        return i18nc("transaction state, when we're waiting for the native tools to exit",
                     "Waiting for package manager lock");
    case Transaction::StatusWaitingForAuth :
        return i18nc("waiting for user to type in a password",
                     "Waiting for authentication");
    case Transaction::StatusScanProcessList :
        return i18nc("we are updating the list of processes",
                     "Updating the list of running applications");
    case Transaction::StatusCheckExecutableFiles :
        return i18nc("we are checking executable files in use",
                     "Checking for applications currently in use");
    case Transaction::StatusCheckLibraries :
        return i18nc("we are checking for libraries in use",
                     "Checking for libraries currently in use");
    case Transaction::StatusCopyFiles :
        return i18nc("we are copying package files to prepare to install",
                     "Copying files");
    }
    kWarning() << "status unrecognised: " << statusEnum;
    return QString();
}

QString PkStrings::statusPast(Transaction::Status status)
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

QString PkStrings::action(int role)
{
    Transaction::Role roleEnum = static_cast<Transaction::Role>(role);
    switch (roleEnum) {
    case Transaction::RoleUnknown :
        return i18nc("The role of the transaction, in present tense", "Unknown role type");
    case Transaction::RoleGetDepends :
        return i18nc("The role of the transaction, in present tense", "Getting dependencies");
    case Transaction::RoleGetUpdateDetail :
        return i18nc("The role of the transaction, in present tense", "Getting update detail");
    case Transaction::RoleGetDetails :
        return i18nc("The role of the transaction, in present tense", "Getting details");
    case Transaction::RoleGetRequires :
        return i18nc("The role of the transaction, in present tense", "Getting requires");
    case Transaction::RoleGetUpdates :
        return i18nc("The role of the transaction, in present tense", "Getting updates");
    case Transaction::RoleSearchDetails :
        return i18nc("The role of the transaction, in present tense", "Searching details");
    case Transaction::RoleSearchFile :
        return i18nc("The role of the transaction, in present tense", "Searching for file");
    case Transaction::RoleSearchGroup :
        return i18nc("The role of the transaction, in present tense", "Searching groups");
    case Transaction::RoleSearchName :
        return i18nc("The role of the transaction, in present tense", "Searching by package name");
    case Transaction::RoleRemovePackages :
        return i18nc("The role of the transaction, in present tense", "Removing");
    case Transaction::RoleInstallPackages :
        return i18nc("The role of the transaction, in present tense", "Installing");
    case Transaction::RoleInstallFiles :
        return i18nc("The role of the transaction, in present tense", "Installing file");
    case Transaction::RoleRefreshCache :
        return i18nc("The role of the transaction, in present tense", "Refreshing package cache");
    case Transaction::RoleUpdatePackages :
        return i18nc("The role of the transaction, in present tense", "Updating packages");
    case Transaction::RoleCancel :
        return i18nc("The role of the transaction, in present tense", "Canceling");
    case Transaction::RoleGetRepoList :
        return i18nc("The role of the transaction, in present tense", "Getting list of repositories");
    case Transaction::RoleRepoEnable :
        return i18nc("The role of the transaction, in present tense", "Enabling repository");
    case Transaction::RoleRepoSetData :
        return i18nc("The role of the transaction, in present tense", "Setting repository data");
    case Transaction::RoleResolve :
        return i18nc("The role of the transaction, in present tense", "Resolving");
    case Transaction::RoleGetFiles :
        return i18nc("The role of the transaction, in present tense", "Getting file list");
    case Transaction::RoleWhatProvides :
        return i18nc("The role of the transaction, in present tense", "Getting what provides");
    case Transaction::RoleInstallSignature :
        return i18nc("The role of the transaction, in present tense", "Installing signature");
    case Transaction::RoleGetPackages :
        return i18nc("The role of the transaction, in present tense", "Getting package lists");
    case Transaction::RoleAcceptEula :
        return i18nc("The role of the transaction, in present tense", "Accepting EULA");
    case Transaction::RoleDownloadPackages :
        return i18nc("The role of the transaction, in present tense", "Downloading packages");
    case Transaction::RoleGetDistroUpgrades :
        return i18nc("The role of the transaction, in present tense", "Getting distribution upgrade information");
    case Transaction::RoleGetCategories :
        return i18nc("The role of the transaction, in present tense", "Getting categories");
    case Transaction::RoleGetOldTransactions :
        return i18nc("The role of the transaction, in present tense", "Getting old transactions");
    case Transaction::RoleUpgradeSystem :
        return i18nc("The role of the transaction, in present tense", "Upgrading system");
    case Transaction::RoleRepairSystem :
        return i18nc("The role of the transaction, in present tense", "Repairing system");
    }
    kWarning() << "action unrecognised: " << role;
    return QString();
}

QString PkStrings::actionPast(Transaction::Role action)
{
    switch (action) {
    case Transaction::RoleUnknown:
        return i18nc("The role of the transaction, in past tense", "Unknown role type");
    case Transaction::RoleGetDepends :
        return i18nc("The role of the transaction, in past tense", "Got dependencies");
    case Transaction::RoleGetUpdateDetail :
        return i18nc("The role of the transaction, in past tense", "Got update detail");
    case Transaction::RoleGetDetails :
        return i18nc("The role of the transaction, in past tense", "Got details");
    case Transaction::RoleGetRequires :
        return i18nc("The role of the transaction, in past tense", "Got requires");
    case Transaction::RoleGetUpdates :
        return i18nc("The role of the transaction, in past tense", "Got updates");
    case Transaction::RoleSearchDetails :
        return i18nc("The role of the transaction, in past tense", "Searched for package details");
    case Transaction::RoleSearchFile :
        return i18nc("The role of the transaction, in past tense", "Searched for file");
    case Transaction::RoleSearchGroup :
        return i18nc("The role of the transaction, in past tense", "Searched groups");
    case Transaction::RoleSearchName :
        return i18nc("The role of the transaction, in past tense", "Searched for package name");
    case Transaction::RoleRemovePackages :
        return i18nc("The role of the transaction, in past tense", "Removed packages");
    case Transaction::RoleInstallPackages :
        return i18nc("The role of the transaction, in past tense", "Installed packages");
    case Transaction::RoleInstallFiles :
        return i18nc("The role of the transaction, in past tense", "Installed local files");
    case Transaction::RoleRefreshCache :
        return i18nc("The role of the transaction, in past tense", "Refreshed package cache");
    case Transaction::RoleUpdatePackages :
        return i18nc("The role of the transaction, in past tense", "Updated packages");
    case Transaction::RoleCancel :
        return i18nc("The role of the transaction, in past tense", "Canceled");
    case Transaction::RoleGetRepoList :
        return i18nc("The role of the transaction, in past tense", "Got list of repositories");
    case Transaction::RoleRepoEnable :
        return i18nc("The role of the transaction, in past tense", "Enabled repository");
    case Transaction::RoleRepoSetData :
        return i18nc("The role of the transaction, in past tense", "Set repository data");
    case Transaction::RoleResolve :
        return i18nc("The role of the transaction, in past tense", "Resolved");
    case Transaction::RoleGetFiles :
        return i18nc("The role of the transaction, in past tense", "Got file list");
    case Transaction::RoleWhatProvides :
        return i18nc("The role of the transaction, in past tense", "Got what provides");
    case Transaction::RoleInstallSignature :
        return i18nc("The role of the transaction, in past tense", "Installed signature");
    case Transaction::RoleGetPackages :
        return i18nc("The role of the transaction, in past tense", "Got package lists");
    case Transaction::RoleAcceptEula :
        return i18nc("The role of the transaction, in past tense", "Accepted EULA");
    case Transaction::RoleDownloadPackages :
        return i18nc("The role of the transaction, in past tense", "Downloaded packages");
    case Transaction::RoleGetDistroUpgrades :
        return i18nc("The role of the transaction, in past tense", "Got distribution upgrades");
    case Transaction::RoleGetCategories :
        return i18nc("The role of the transaction, in past tense", "Got categories");
    case Transaction::RoleGetOldTransactions :
        return i18nc("The role of the transaction, in past tense", "Got old transactions");
    case Transaction::RoleUpgradeSystem :
        return i18nc("The role of the transaction, in past tense", "Upgraded system");
    case Transaction::RoleRepairSystem:
        return i18nc("The role of the transaction, in past tense", "Repaired system");
    }
    kWarning() << "action unrecognised: " << action;
    return QString();
}

QString PkStrings::infoPresent(Transaction::Info info)
{
    switch (info) {
    case Transaction::InfoDownloading :
        return i18n("Downloading");
    case Transaction::InfoUpdating :
        return i18n("Updating");
    case Transaction::InfoInstalling :
        return i18n("Installing");
    case Transaction::InfoRemoving :
        return i18n("Removing");
    case Transaction::InfoCleanup :
        return i18n("Cleaning up");
    case Transaction::InfoObsoleting :
        return i18n("Obsoleting");
    case Transaction::InfoReinstalling :
        return i18n("Reinstalling");
    case Transaction::InfoPreparing :
        return i18n("Preparing");
    case Transaction::InfoDecompressing :
        return i18n("Decompressing");
    default :
        kWarning() << "info unrecognised: " << info;
        return QString();
    }
}

QString PkStrings::infoPast(Transaction::Info info)
{
    switch (info) {
    case Transaction::InfoDownloading :
        return i18n("Downloaded");
    case Transaction::InfoUpdating :
        return i18n("Updated");
    case Transaction::InfoInstalling :
        return i18n("Installed");
    case Transaction::InfoRemoving :
        return i18n("Removed");
    case Transaction::InfoCleanup :
        return i18n("Cleaned up");
    case Transaction::InfoObsoleting :
        return i18n("Obsoleted");
    case Transaction::InfoReinstalling :
        return i18n("Reinstalled");
    case Transaction::InfoPreparing :
        return i18n("Prepared");
    case Transaction::InfoDecompressing :
        return i18n("Decompressed");
    default :
        kWarning() << "info unrecognised: " << info;
        return QString();
    }
}

QString PkStrings::error(Transaction::Error error)
{
    switch (error) {
    case Transaction::ErrorNoNetwork :
        return i18n("No network connection available");
    case Transaction::ErrorNoCache :
        return i18n("No package cache is available");
    case Transaction::ErrorOom :
        return i18n("Out of memory");
    case Transaction::ErrorCreateThreadFailed :
        return i18n("Failed to create a thread");
    case Transaction::ErrorNotSupported :
        return i18n("Not supported by this backend");
    case Transaction::ErrorInternalError :
        return i18n("An internal system error has occurred");
    case Transaction::ErrorGpgFailure :
        return i18n("A security trust relationship is not present");
    case Transaction::ErrorPackageNotInstalled :
        return i18n("The package is not installed");
    case Transaction::ErrorPackageNotFound :
        return i18n("The package was not found");
    case Transaction::ErrorPackageAlreadyInstalled :
        return i18n("The package is already installed");
    case Transaction::ErrorPackageDownloadFailed :
        return i18n("The package download failed");
    case Transaction::ErrorGroupNotFound :
        return i18n("The group was not found");
    case Transaction::ErrorGroupListInvalid :
        return i18n("The group list was invalid");
    case Transaction::ErrorDepResolutionFailed :
        return i18n("Dependency resolution failed");
    case Transaction::ErrorFilterInvalid :
        return i18n("Search filter was invalid");
    case Transaction::ErrorPackageIdInvalid :
        return i18n("The package identifier was not well formed");
    case Transaction::ErrorTransactionError :
        return i18n("Transaction error");
    case Transaction::ErrorRepoNotFound :
        return i18n("Repository name was not found");
    case Transaction::ErrorCannotRemoveSystemPackage :
        return i18n("Could not remove a protected system package");
    case Transaction::ErrorTransactionCancelled :
        return i18n("The task was canceled");
    case Transaction::ErrorProcessKill :
        return i18n("The task was forcibly canceled");
    case Transaction::ErrorFailedConfigParsing :
        return i18n("Reading the config file failed");
    case Transaction::ErrorCannotCancel :
        return i18n("The task cannot be cancelled");
    case Transaction::ErrorCannotInstallSourcePackage :
        return i18n("Source packages cannot be installed");
    case Transaction::ErrorNoLicenseAgreement :
        return i18n("The license agreement failed");
    case Transaction::ErrorFileConflicts :
        return i18n("Local file conflict between packages");
    case Transaction::ErrorPackageConflicts :
        return i18n("Packages are not compatible");
    case Transaction::ErrorRepoNotAvailable :
        return i18n("Problem connecting to a software origin");
    case Transaction::ErrorFailedInitialization :
        return i18n("Failed to initialize");
    case Transaction::ErrorFailedFinalise :
        return i18n("Failed to finalize");
    case Transaction::ErrorCannotGetLock :
        return i18n("Cannot get lock");
    case Transaction::ErrorNoPackagesToUpdate :
        return i18n("No packages to update");
    case Transaction::ErrorCannotWriteRepoConfig :
        return i18n("Cannot write repository configuration");
    case Transaction::ErrorLocalInstallFailed :
        return i18n("Local install failed");
    case Transaction::ErrorBadGpgSignature :
        return i18n("Bad GPG signature");
    case Transaction::ErrorMissingGpgSignature :
        return i18n("Missing GPG signature");
    case Transaction::ErrorRepoConfigurationError :
        return i18n("Repository configuration invalid");
    case Transaction::ErrorInvalidPackageFile :
        return i18n("Invalid package file");
    case Transaction::ErrorPackageInstallBlocked :
        return i18n("Package install blocked");
    case Transaction::ErrorPackageCorrupt :
        return i18n("Package is corrupt");
    case Transaction::ErrorAllPackagesAlreadyInstalled :
        return i18n("All packages are already installed");
    case Transaction::ErrorFileNotFound :
        return i18n("The specified file could not be found");
    case Transaction::ErrorNoMoreMirrorsToTry :
        return i18n("No more mirrors are available");
    case Transaction::ErrorNoDistroUpgradeData :
        return i18n("No distribution upgrade data is available");
    case Transaction::ErrorIncompatibleArchitecture :
        return i18n("Package is incompatible with this system");
    case Transaction::ErrorNoSpaceOnDevice :
        return i18n("No space is left on the disk");
    case Transaction::ErrorMediaChangeRequired :
        return i18n("A media change is required");
    case Transaction::ErrorNotAuthorized :
        return i18n("Authorization failed");
    case Transaction::ErrorUpdateNotFound :
        return i18n("Update not found");
    case Transaction::ErrorCannotInstallRepoUnsigned :
        return i18n("Cannot install from untrusted origin");
    case Transaction::ErrorCannotUpdateRepoUnsigned :
        return i18n("Cannot update from untrusted origin");
    case Transaction::ErrorCannotGetFilelist :
        return i18n("Cannot get the file list");
    case Transaction::ErrorCannotGetRequires :
        return i18n("Cannot get package requires");
    case Transaction::ErrorCannotDisableRepository :
        return i18n("Cannot disable origin");
    case Transaction::ErrorRestrictedDownload :
        return i18n("The download failed");
    case Transaction::ErrorPackageFailedToConfigure :
        return i18n("Package failed to configure");
    case Transaction::ErrorPackageFailedToBuild :
        return i18n("Package failed to build");
    case Transaction::ErrorPackageFailedToInstall :
        return i18n("Package failed to install");
    case Transaction::ErrorPackageFailedToRemove :
        return i18n("Package failed to be removed");
    case Transaction::ErrorUpdateFailedDueToRunningProcess :
        return i18n("Update failed due to running process");
    case Transaction::ErrorPackageDatabaseChanged :
        return i18n("The package database was changed");
    case Transaction::ErrorProvideTypeNotSupported :
        return i18n("Virtual provide type is not supported");
    case Transaction::ErrorInstallRootInvalid :
        return i18n("Install root is invalid");
    case Transaction::ErrorCannotFetchSources :
        return i18n("Cannot fetch install sources");
    case Transaction::ErrorCancelledPriority :
        return i18n("Rescheduled due to priority");
    case Transaction::ErrorUnfinishedTransaction:
        return i18n("Unfinished transaction");
    case Transaction::ErrorLockRequired:
        return i18n("Lock required");
    case Transaction::ErrorUnknown:
        return i18n("Unknown error");
    }
    kWarning() << "error unrecognised: " << error;
    return QString();
}

QString PkStrings::errorMessage(Transaction::Error error)
{
    switch (error) {
    case Transaction::ErrorNoNetwork :
        return i18n("There is no network connection available.\n"
                    "Please check your connection settings and try again");
    case Transaction::ErrorNoCache :
        return i18n("The package list needs to be rebuilt.\n"
                    "This should have been done by the backend automatically.");
    case Transaction::ErrorOom :
        return i18n("The service that is responsible for handling user requests is out of memory.\n"
                    "Please close some programs or restart your computer.");
    case Transaction::ErrorCreateThreadFailed :
        return i18n("A thread could not be created to service the user request.");
    case Transaction::ErrorNotSupported :
        return i18n("The action is not supported by this backend.\n"
                    "Please report a bug as this should not have happened.");
    case Transaction::ErrorInternalError :
        return i18n("A problem that we were not expecting has occurred.\n"
                    "Please report this bug with the error description.");
    case Transaction::ErrorGpgFailure :
        return i18n("A security trust relationship could not be made with the software origin.\n"
                    "Please check your software signature settings.");
    case Transaction::ErrorPackageNotInstalled :
        return i18n("The package that is trying to be removed or updated is not already installed.");
    case Transaction::ErrorPackageNotFound :
        return i18n("The package that is being modified was not found on your system or in any software origin.");
    case Transaction::ErrorPackageAlreadyInstalled :
        return i18n("The package that is trying to be installed is already installed.");
    case Transaction::ErrorPackageDownloadFailed :
        return i18n("The package download failed.\n"
                    "Please check your network connectivity.");
    case Transaction::ErrorGroupNotFound :
        return i18n("The group type was not found.\n"
                    "Please check your group list and try again.");
    case Transaction::ErrorGroupListInvalid :
        return i18n("The group list could not be loaded.\n"
                    "Refreshing your cache may help, although this is normally a software "
                    "origin error.");
    case Transaction::ErrorDepResolutionFailed :
        return i18n("A package dependency could not be found.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorFilterInvalid :
        return i18n("The search filter was not correctly formed.");
    case Transaction::ErrorPackageIdInvalid :
        return i18n("The package identifier was not well formed when sent to the system daemon.\n"
                    "This normally indicates an internal bug and should be reported.");
    case Transaction::ErrorTransactionError :
        return i18n("An error occurred while running the transaction.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorRepoNotFound :
        return i18n("The remote software origin name was not found.\n"
                    "You may need to enable an item in Software Origins.");
    case Transaction::ErrorCannotRemoveSystemPackage :
        return i18n("Removing a protected system package is not allowed.");
    case Transaction::ErrorTransactionCancelled :
        return i18n("The task was canceled successfully and no packages were changed.");
    case Transaction::ErrorProcessKill :
        return i18n("The task was canceled successfully and no packages were changed.\n"
                    "The backend did not exit cleanly.");
    case Transaction::ErrorFailedConfigParsing :
        return i18n("The native package configuration file could not be opened.\n"
                    "Please make sure your system's configuration is valid.");
    case Transaction::ErrorCannotCancel :
        return i18n("The task is not safe to be cancelled at this time.");
    case Transaction::ErrorCannotInstallSourcePackage :
        return i18n("Source packages are not normally installed this way.\n"
                    "Check the extension of the file you are trying to install.");
    case Transaction::ErrorNoLicenseAgreement :
        return i18n("The license agreement was not agreed to.\n"
                    "To use this software you have to accept the license.");
    case Transaction::ErrorFileConflicts :
        return i18n("Two packages provide the same file.\n"
                    "This is usually due to mixing packages for different software origins.");
    case Transaction::ErrorPackageConflicts :
        return i18n("Multiple packages exist that are not compatible with each other.\n"
                    "This is usually due to mixing packages from different software origins.");
    case Transaction::ErrorRepoNotAvailable :
        return i18n("There was a (possibly temporary) problem connecting to a software origins.\n"
                    "Please check the detailed error for further details.");
    case Transaction::ErrorFailedInitialization :
        return i18n("Failed to initialize packaging backend.\n"
                    "This may occur if other packaging tools are being used simultaneously.");
    case Transaction::ErrorFailedFinalise :
        return i18n("Failed to close down the backend instance.\n"
                    "This error can normally be ignored.");
    case Transaction::ErrorCannotGetLock :
        return i18n("Cannot get the exclusive lock on the packaging backend.\n"
                    "Please close any other legacy packaging tools that may be open.");
    case Transaction::ErrorNoPackagesToUpdate :
        return i18n("None of the selected packages could be updated.");
    case Transaction::ErrorCannotWriteRepoConfig :
        return i18n("The repository configuration could not be modified.");
    case Transaction::ErrorLocalInstallFailed :
        return i18n("Installing the local file failed.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorBadGpgSignature :
        return i18n("The package signature could not be verified.");
    case Transaction::ErrorMissingGpgSignature :
        return i18n("The package signature was missing and this package is untrusted.\n"
                    "This package was not signed with a GPG key when created.");
    case Transaction::ErrorRepoConfigurationError :
        return i18n("Repository configuration was invalid and could not be read.");
    case Transaction::ErrorInvalidPackageFile :
        return i18n("The package you are attempting to install is not valid.\n"
                    "The package file could be corrupt, or not a proper package.");
    case Transaction::ErrorPackageInstallBlocked :
        return i18n("Installation of this package was prevented by your packaging system's configuration.");
    case Transaction::ErrorPackageCorrupt :
        return i18n("The package that was downloaded is corrupt and needs to be downloaded again.");
    case Transaction::ErrorAllPackagesAlreadyInstalled :
        return i18n("All of the packages selected for install are already installed on the system.");
    case Transaction::ErrorFileNotFound :
        return i18n("The specified file could not be found on the system.\n"
                    "Check that the file still exists and has not been deleted.");
    case Transaction::ErrorNoMoreMirrorsToTry :
        return i18n("Required data could not be found on any of the configured software origins.\n"
                    "There were no more download mirrors that could be tried.");
    case Transaction::ErrorNoDistroUpgradeData :
        return i18n("Required upgrade data could not be found in any of the configured software origins.\n"
                    "The list of distribution upgrades will be unavailable.");
    case Transaction::ErrorIncompatibleArchitecture :
        return i18n("The package that is trying to be installed is incompatible with this system.");
    case Transaction::ErrorNoSpaceOnDevice :
        return i18n("There is insufficient space on the device.\n"
                    "Free some space on the system disk to perform this operation.");
    case Transaction::ErrorMediaChangeRequired :
        return i18n("Additional media is required to complete the transaction.");
    case Transaction::ErrorNotAuthorized :
        return i18n("You have failed to provide correct authentication.\n"
                    "Please check any passwords or account settings.");
    case Transaction::ErrorUpdateNotFound :
        return i18n("The specified update could not be found.\n"
                    "It could have already been installed or no longer available on the remote server.");
    case Transaction::ErrorCannotInstallRepoUnsigned :
        return i18n("The package could not be installed from untrusted origin.");
    case Transaction::ErrorCannotUpdateRepoUnsigned :
        return i18n("The package could not be updated from untrusted origin.");
    case Transaction::ErrorCannotGetFilelist :
        return i18n("The file list is not available for this package.");
    case Transaction::ErrorCannotGetRequires :
        return i18n("The information about what requires this package could not be obtained.");
    case Transaction::ErrorCannotDisableRepository :
        return i18n("The specified software origin could not be disabled.");
    case Transaction::ErrorRestrictedDownload :
        return i18n("The download could not be done automatically and should be done manually.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorPackageFailedToConfigure :
        return i18n("One of the selected packages failed to configure correctly.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorPackageFailedToBuild :
        return i18n("One of the selected packages failed to build correctly.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorPackageFailedToInstall :
        return i18n("One of the selected packages failed to install correctly.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorPackageFailedToRemove :
        return i18n("One of the selected packages failed to be removed correctly.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorUpdateFailedDueToRunningProcess :
        return i18n("A program is running that has to be closed before the update can proceed.\n"
                    "More information is available in the detailed report.");
    case Transaction::ErrorPackageDatabaseChanged :
        return i18n("The package database was changed while the request was running.");
    case Transaction::ErrorProvideTypeNotSupported :
        return i18n("The virtual provide type is not supported by this system.");
    case Transaction::ErrorInstallRootInvalid :
        return i18n("The install root is invalid. Please contact your administrator.");
    case Transaction::ErrorCannotFetchSources :
        return i18n("The list of software could not be downloaded.");
    case Transaction::ErrorCancelledPriority :
        return i18n("The transaction has been cancelled and will be retried when the system is idle.");
    case Transaction::ErrorUnfinishedTransaction :
        return i18n("A previous package management transaction was interrupted.");
    case Transaction::ErrorLockRequired :
        return i18n("A package manager lock is required.");
    case Transaction::ErrorUnknown:
        return i18n("Unknown error, please report a bug.\n"
                    "More information is available in the detailed report.");
    }
    kWarning() << "error unrecognised: " << error;
    return QString();
}

QString PkStrings::groups(Transaction::Group group)
{
    switch (group) {
    case Transaction::GroupAccessibility :
        return i18nc("The group type", "Accessibility");
    case Transaction::GroupAccessories :
        return i18nc("The group type", "Accessories");
    case Transaction::GroupEducation :
        return i18nc("The group type", "Education");
    case Transaction::GroupGames :
        return i18nc("The group type", "Games");
    case Transaction::GroupGraphics :
        return i18nc("The group type", "Graphics");
    case Transaction::GroupInternet :
        return i18nc("The group type", "Internet");
    case Transaction::GroupOffice :
        return i18nc("The group type", "Office");
    case Transaction::GroupOther :
        return i18nc("The group type", "Others");
    case Transaction::GroupProgramming :
        return i18nc("The group type", "Development");
    case Transaction::GroupMultimedia :
        return i18nc("The group type", "Multimedia");
    case Transaction::GroupSystem :
        return i18nc("The group type", "System");
    case Transaction::GroupDesktopGnome :
        return i18nc("The group type", "GNOME desktop");
    case Transaction::GroupDesktopKde :
        return i18nc("The group type", "KDE desktop");
    case Transaction::GroupDesktopXfce :
        return i18nc("The group type", "XFCE desktop");
    case Transaction::GroupDesktopOther :
        return i18nc("The group type", "Other desktops");
    case Transaction::GroupPublishing :
        return i18nc("The group type", "Publishing");
    case Transaction::GroupServers :
        return i18nc("The group type", "Servers");
    case Transaction::GroupFonts :
        return i18nc("The group type", "Fonts");
    case Transaction::GroupAdminTools :
        return i18nc("The group type", "Admin tools");
    case Transaction::GroupLegacy :
        return i18nc("The group type", "Legacy");
    case Transaction::GroupLocalization :
        return i18nc("The group type", "Localization");
    case Transaction::GroupVirtualization :
        return i18nc("The group type", "Virtualization");
    case Transaction::GroupSecurity :
        return i18nc("The group type", "Security");
    case Transaction::GroupPowerManagement :
        return i18nc("The group type", "Power management");
    case Transaction::GroupCommunication :
        return i18nc("The group type", "Communication");
    case Transaction::GroupNetwork :
        return i18nc("The group type", "Network");
    case Transaction::GroupMaps :
        return i18nc("The group type", "Maps");
    case Transaction::GroupRepos :
        return i18nc("The group type", "Software sources");
    case Transaction::GroupScience :
        return i18nc("The group type", "Science");
    case Transaction::GroupDocumentation :
        return i18nc("The group type", "Documentation");
    case Transaction::GroupElectronics :
        return i18nc("The group type", "Electronics");
    case Transaction::GroupCollections ://TODO check this one
        return i18nc("The group type", "Package collections");
    case Transaction::GroupVendor :
        return i18nc("The group type", "Vendor");
    case Transaction::GroupNewest :
        return i18nc("The group type", "Newest packages");
    case Transaction::GroupUnknown:
        return i18nc("The group type", "Unknown group");
    }
    kWarning() << "group unrecognised: " << group;
    return QString();
}

QString PkStrings::info(int state)
{
    Transaction::Info stateEnum = static_cast<Transaction::Info>(state);
    switch (stateEnum) {
    case Transaction::InfoLow :
        return i18nc("The type of update", "Trivial update");
    case Transaction::InfoNormal :
        return i18nc("The type of update", "Normal update");
    case Transaction::InfoImportant :
        return i18nc("The type of update", "Important update");
    case Transaction::InfoSecurity :
        return i18nc("The type of update", "Security update");
    case Transaction::InfoBugfix :
        return i18nc("The type of update", "Bug fix update");
    case Transaction::InfoEnhancement :
        return i18nc("The type of update", "Enhancement update");
    case Transaction::InfoBlocked :
        return i18nc("The type of update", "Blocked update");
    case Transaction::InfoInstalled :
    case Transaction::InfoCollectionInstalled :
        return i18nc("The type of update", "Installed");
    case Transaction::InfoAvailable :
    case Transaction::InfoCollectionAvailable :
        return i18nc("The type of update", "Available");
    case Transaction::InfoUnknown:
        return i18nc("The type of update", "Unknown update");
    default : // In this case we don't want to map all enums
        kWarning() << "info unrecognised: " << state;
        return QString();
    }
}

QString PkStrings::packageQuantity(bool updates, int packages, int selected)
{
    if (updates) {
        if (packages == 0) {
            return i18n("No Updates Available");
        } else if (packages == selected) {
            return i18ncp("Some updates were selected on the view",
                          "1 Update Selected",
                          "%1 Updates Selected", packages);
        } else if (selected == 0) {
            return i18ncp("Some updates are being shown on the screen",
                          "1 Update", "%1 Updates",
                          packages);
        } else {
            return i18nc("Type of update, in the case it's just an update", "%1, %2", 
                         i18ncp("Part of: %1 Updates, %1 Selected", "%1 Update", "%1 Updates", packages), 
                         i18ncp("Part of: %1 Updates, %1 Selected", "%1 Selected", "%1 Selected", selected));
        }
    } else {
        if (packages == 0) {
            return i18n("No Packages");
        }
        return i18np("1 Package", "%1 Packages", packages);
    }
}

QString PkStrings::restartTypeFuture(Transaction::Restart value)
{
    switch (value) {
    case Transaction::RestartNone:
        return i18n("No restart is necessary");
    case Transaction::RestartApplication:
        return i18n("You will be required to restart this application");
    case Transaction::RestartSession:
        return i18n("You will be required to log out and back in");
    case Transaction::RestartSystem:
        return i18n("A restart will be required");
    case Transaction::RestartSecuritySession:
        return i18n("You will be required to log out and back in due to a security update.");
    case Transaction::RestartSecuritySystem:
        return i18n("A restart will be required due to a security update.");
    case Transaction::RestartUnknown:
        kWarning() << "restartTypeFuture(Transaction::RestartUnknown)";
        return QString();
    }
    kWarning() << "restart unrecognised: " << value;
    return QString();
}

QString PkStrings::restartType(Transaction::Restart value)
{
    switch (value) {
    case Transaction::RestartNone:
        return i18n("No restart is required");
    case Transaction::RestartSystem:
        return i18n("A restart is required");
    case Transaction::RestartSession:
        return i18n("You need to log out and log back in");
    case Transaction::RestartApplication:
        return i18n("You need to restart the application");
    case Transaction::RestartSecuritySession:
        return i18n("You need to log out and log back in to remain secure.");
    case Transaction::RestartSecuritySystem:
        return i18n("A restart is required to remain secure.");
    case Transaction::RestartUnknown:
        kWarning() << "restartType(Transaction::RestartUnknown)";
        return QString();
    }
    kWarning() << "restart unrecognised: " << value;
    return QString();
}

QString PkStrings::updateState(Transaction::UpdateState value)
{
    switch (value) {
    case Transaction::UpdateStateStable:
        return i18n("Stable");
    case Transaction::UpdateStateUnstable:
        return i18n("Unstable");
    case Transaction::UpdateStateTesting:
        return i18n("Testing");
    case Transaction::UpdateStateUnknown:
        kWarning() << "updateState(Transaction::UnknownUpdateState)";
        return QString();
    }
    kWarning() << "value unrecognised: " << value;
    return QString();
}

QString PkStrings::mediaMessage(Transaction::MediaType value, const QString &text)
{
    switch (value) {
    case Transaction::MediaTypeCd :
        return i18n("Please insert the CD labeled '%1', and press continue.", text);
    case Transaction::MediaTypeDvd :
        return i18n("Please insert the DVD labeled '%1', and press continue.", text);
    case Transaction::MediaTypeDisc :
        return i18n("Please insert the disc labeled '%1', and press continue.", text);
    case Transaction::MediaTypeUnknown:
        return i18n("Please insert the medium labeled '%1', and press continue.", text);
    }
    kWarning() << "value unrecognised: " << value;
    return i18n("Please insert the medium labeled '%1', and press continue.", text);
}

QString PkStrings::message(Transaction::Message value)
{
    switch (value) {
    case Transaction::MessageBrokenMirror :
        return i18n("A mirror is possibly broken");
    case Transaction::MessageConnectionRefused :
        return i18n("The connection was refused");
    case Transaction::MessageParameterInvalid :
        return i18n("The parameter was invalid");
    case Transaction::MessagePriorityInvalid :
        return i18n("The priority was invalid");
    case Transaction::MessageBackendError :
        return i18n("Backend warning");
    case Transaction::MessageDaemonError :
        return i18n("Daemon warning");
    case Transaction::MessageCacheBeingRebuilt :
        return i18n("The package list cache is being rebuilt");
    case Transaction::MessageUntrustedPackage :
        return i18n("An untrusted package was installed");
    case Transaction::MessageNewerPackageExists :
        return i18n("A newer package exists");
    case Transaction::MessageCouldNotFindPackage :
        return i18n("Could not find package");
    case Transaction::MessageConfigFilesChanged :
        return i18n("Configuration files were changed");
    case Transaction::MessagePackageAlreadyInstalled :
        return i18n("Package is already installed");
    case Transaction::MessageAutoremoveIgnored :
        return i18n("Automatic cleanup is being ignored");
    case Transaction::MessageRepoMetadataDownloadFailed :
        return i18n("Software source download failed");
    case Transaction::MessageRepoForDevelopersOnly :
        return i18n("This software source is for developers only");
    case Transaction::MessageOtherUpdatesHeldBack :
        return i18n("Other updates have been held back");
    case Transaction::MessageUnknown:
        kWarning() << "message(Enum::UnknownMessageType)";
        return QString();
    }
    kWarning() << "value unrecognised: " << value;
    return QString();
}

QString PkStrings::daemonError(Transaction::InternalError value)
{
    switch (value) {
    case Transaction::InternalErrorFailedAuth :
        return i18n("You do not have the necessary privileges to perform this action.");
    case Transaction::InternalErrorNoTid :
        return i18n("Could not get a transaction id from packagekitd.");
    case Transaction::InternalErrorAlreadyTid :
        return i18n("Cannot connect to this transaction id.");
    case Transaction::InternalErrorRoleUnkown :
        return i18n("This action is unknown.");
    case Transaction::InternalErrorCannotStartDaemon :
        return i18n("The packagekitd service could not be started.");
    case Transaction::InternalErrorInvalidInput :
        return i18n("The query is not valid.");
    case Transaction::InternalErrorInvalidFile :
        return i18n("The file is not valid.");
    case Transaction::InternalErrorFunctionNotSupported :
        return i18n("This function is not yet supported.");
    case Transaction::InternalErrorDaemonUnreachable :
        return i18n("Could not talk to packagekitd.");
    case Transaction::InternalErrorNone:
    case Transaction::InternalErrorFailed :
    case Transaction::InternalErrorUnkown :
        return i18n("An unknown error happened.");
    }
    kWarning() << "value unrecognised: " << value;
    return i18n("An unknown error happened.");
}

QString PkStrings::prettyFormatDuration(unsigned long mSec)
{
    return KGlobal::locale()->prettyFormatDuration(mSec);
}

QString PkStrings::lastCacheRefreshTitle(uint lastTime)
{
    unsigned long fifteen = 60 * 60 * 24 * 15;
    unsigned long tirty = 60 * 60 * 24 * 30;

    if (lastTime != UINT_MAX && lastTime < fifteen) {
        return i18n("Your system is up to date");
    } else if (lastTime != UINT_MAX && lastTime > fifteen && lastTime < tirty) {
        return i18n("You have no updates");
    }
    return i18n("Last check for updates was more than a month ago");
}

QString PkStrings::lastCacheRefreshSubTitle(uint lastTime)
{
    unsigned long tirty = 60 * 60 * 24 * 30;

    if (lastTime != UINT_MAX && lastTime < tirty) {
        return i18n("Verified %1 ago", PkStrings::prettyFormatDuration(lastTime * 1000));
    }
    return i18n("It's strongly recommended that you check for new updates now");
}
