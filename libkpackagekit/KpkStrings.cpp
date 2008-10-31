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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <KLocale>

#include <KDebug>

#include "KpkStrings.h"

KpkStrings::KpkStrings( QObject *parent )
 : QObject(parent)
{
}

KpkStrings::~KpkStrings()
{
}

QString KpkStrings::status(PackageKit::Transaction::Status status)
{
    switch (status) {
        case Transaction::UnknownStatus :
	    return i18n("Unknown state");
        case Transaction::Setup :
	    return i18n("Waiting for service to start");
        case Transaction::Wait :
	    return i18n("Waiting for other tasks");
        case Transaction::Running :
	    return i18n("Running task");
        case Transaction::Query :
	    return i18n("Querying");
        case Transaction::Info :
	    return i18n("Getting information");
        case Transaction::Remove :
	    return i18n("Removing");
        case Transaction::RefreshCache :
	    return i18n("Refreshing software list");
        case Transaction::Download :
	    return i18n("Downloading");
        case Transaction::Install :
	    return i18n("Installing");
        case Transaction::Update :
	    return i18n("Updating");
        case Transaction::Cleanup :
	    return i18n("Cleaning Up");
        case Transaction::Obsolete :
	    return i18n("Obsoleting");
        case Transaction::DepResolve :
	    return i18n("Resolving dependencies");
        case Transaction::SigCheck :
	    return i18n("Checking signatures");
        case Transaction::Rollback :
	    return i18n("Rolling back");
        case Transaction::TestCommit :
	    return i18n("Testing changes");
        case Transaction::Commit :
	    return i18n("Committing changes");
        case Transaction::Request :
	    return i18n("Requesting data");
        case Transaction::Finished :
	    return i18n("Finished");
        case Transaction::Cancel :
	    return i18n("Cancelling");
        case Transaction::DownloadRepository :
	    return i18n("Downloading repository information");
	case Transaction::DownloadPackagelist :
	    return i18n("Downloading list of packages");
	case Transaction::DownloadFilelist :
	    return i18n("Downloading file lists");
	case Transaction::DownloadChangelog :
	    return i18n("Downloading lists of changes");
	case Transaction::DownloadGroup :
	    return i18n("Downloading groups");
	case Transaction::DownloadUpdateinfo :
	    return i18n("Downloading update information");
	case Transaction::Repackaging :
	    return i18n("Repackaging files");
        case Transaction::LoadingCache :
	    return i18n("Loading cache");
        case Transaction::ScanApplications :
	    return i18n("Scanning installed applications");
        case Transaction::GeneratePackageList :
	    return i18n("Generating package lists");
	default :
	    kDebug() << "status unrecognised: " << status;
	    return QString();
    }
}

QString KpkStrings::action(Client::Action action)
{
    switch (action) {
        case Client::ActionCancel :
	    return i18n("Canceling");
        case Client::ActionGetDepends :
	    return i18n("Getting dependencies");
        case Client::ActionGetDetails :
	    return i18n("Getting details");
        case Client::ActionGetFiles :
	    return i18n("Searching for file");
        case Client::ActionGetPackages :
	    return i18n("Getting package lists");
        case Client::ActionGetRepoList :
	    return i18n("Getting list of repositories");
        case Client::ActionGetRequires :
	    return i18n("Getting requires");
        case Client::ActionGetUpdateDetail :
	    return i18n("Getting update detail");
        case Client::ActionGetUpdates :
	    return i18n("Getting updates");
        case Client::ActionInstallFiles :
	    return i18n("Installing file");
        case Client::ActionInstallPackages :
	    return i18n("Installing");
        case Client::ActionInstallSignature :
	    return i18n("Installing signature");
        case Client::ActionRefreshCache :
	    return i18n("Refreshing package cache");
        case Client::ActionRemovePackages :
	    return i18n("Removing");
        case Client::ActionRepoEnable :
	    return i18n("Enabling repository");
        case Client::ActionRepoSetData :
	    return i18n("Setting repository data");
        case Client::ActionResolve :
	    return i18n("Resolving");
        case Client::ActionRollback :
	    return i18n("Rolling back");
        case Client::ActionSearchDetails :
	    return i18n("Searching details");
        case Client::ActionSearchFile :
	    return i18n("Searching for file");
        case Client::ActionSearchGroup :
	    return i18n("Searching groups");
        case Client::ActionSearchName :
	    return i18n("Searching for package name");
        case Client::ActionServicePack :
	    return i18n("Service pack");
        case Client::ActionUpdatePackages :
	    return i18n("Updating packages");
        case Client::ActionUpdateSystem :
	    return i18n("Updating system");
        case Client::ActionWhatProvides :
	    return i18n("Getting what provides");
        case Client::ActionAcceptEula :
	    return i18n("Accepting EULA");
        case Client::ActionDownloadPackages :
	    return i18n("Downloading packages");
        case Client::ActionGetDistroUpgrades :
	    return i18n("Getting distribution upgrade information");
	case Client::UnkownAction :
	    return QString();
        default :
	    kDebug() << "action unrecognised: " << action;
	    return QString();
    }
}

QString KpkStrings::error(PackageKit::Client::ErrorType error)
{
    switch (error) {
	case Client::Oom :
	    return i18n("Out of memory");
	case Client::NoNetwork :
	    return i18n("No network connection available");
	case Client::NotSupported :
	    return i18n("Not supported by this backend");
	case Client::InternalError :
	    return i18n("An internal system error has occurred");
	case Client::GpgFailure :
	    return i18n("A security trust relationship is not present");
	case Client::PackageIdInvalid :
	    return i18n("The package identifier was not well formed");
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
	case Client::CreateThreadFailed :
	    return i18n("Failed to create a thread");
	case Client::TransactionError :
	    return i18n("Transaction error");
	case Client::TransactionCancelled :
	    return i18n("The task was canceled");
	case Client::NoCache :
	    return i18n("No package cache is available");
	case Client::RepoNotFound :
	    return i18n("Repository name was not found");
	case Client::CannotRemoveSystemPackage :
	    return i18n("Could not remove a protected system package");
	case Client::ProcessKill :
	    return i18n("The task was forcibly canceled");
	case Client::FailedInitialization :
	    return i18n("Failed to initialize");
	case Client::FailedFinalise :
	    return i18n("Failed to finalize");
	case Client::FailedConfigParsing :
	    return i18n("Reading the config file failed");
	case Client::CannotCancel :
	    return i18n("The task cannot be cancelled");
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
	case Client::CannotInstallSourcePackage :
	    return i18n("Source packages cannot be installed");
	case Client::RepoConfigurationError :
	    return i18n("Repository configuration invalid");
	case Client::NoLicenseAgreement :
	    return i18n("The license agreement failed");
	case Client::FileConflicts :
	    return i18n("Local file conflict between packages");
	case Client::PackageConflicts :
	    return i18n("Packages are not compatible");
	case Client::RepoNotAvailable :
	    return i18n("Problem connecting to a software source");
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
	case Client::UnknownErrorType :
	    return i18n("Unknown error");
	default :
	    kDebug() << "error unrecognised: " << error;
	    return QString();
    }
}

QString KpkStrings::errorMessage(PackageKit::Client::ErrorType error)
{
    switch (error) {
	case Client::Oom :
	    return i18n("The service that is responsible for handling user requests is out of memory.\n"
			"Please restart your computer.");
	case Client::NoNetwork :
	    return i18n("There is no network connection available.\n"
			"Please check your connection settings and try again");
	case Client::NotSupported :
	    return i18n("The action is not supported by this backend.\n"
			"Please report a bug as this shouldn't have happened.");
	case Client::InternalError :
	    return i18n("A problem that we were not expecting has occurred.\n"
			"Please report this bug with the error description.");
	case Client::GpgFailure :
	    return i18n("A security trust relationship could not be made with software source.\n"
			"Please check your security settings.");
	case Client::PackageIdInvalid :
	    return i18n("The package identifier was not well formed when sent to the server.\n"
			"This normally indicates an internal error and should be reported.");
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
	    return i18n("A package could not be found that allows the task to complete.\n"
			"More information is available in the detailed report.");
	case Client::FilterInvalid :
	    return i18n("The search filter was not correctly formed.");
	case Client::CreateThreadFailed :
	    return i18n("A thread could not be created to service the user request.");
	case Client::TransactionError :
	    return i18n("An unspecified task error has occurred.\n"
			"More information is available in the detailed report.");
	case Client::TransactionCancelled :
	    return i18n("The task was canceled successfully and no packages were changed.");
	case Client::NoCache :
	    return i18n("The package list needs to be rebuilt.\n"
			"This should have been done by the backend automatically.");
	case Client::RepoNotFound :
	    return i18n("The remote software source name was not found.\n"
			"You may need to enable an item in Software Sources.");
	case Client::CannotRemoveSystemPackage :
	    return i18n("Removing a protected system package is not alloed.");
	case Client::ProcessKill :
	    return i18n("The task was canceled successfully and no packages were changed.\n"
			"The backend did not exit cleanly.");
	case Client::FailedInitialization :
	    return i18n("Failed to initialize packaging backend.\n"
			"This may occur if other packaging tools are being used simultaneously.");
	case Client::FailedFinalise :
	    return i18n("Failed to close down the backend instance.\n"
			"This error can normally be ignored.");
	case Client::FailedConfigParsing :
	    return i18n("The native package configuration file could not be opened.\n"
			"Please make sure configuration is valid.");
	case Client::CannotCancel :
	    return i18n("The task is not safe to be cancelled at this time.");
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
	case Client::CannotInstallSourcePackage :
	    return i18n("Source packages are not normally installed this way.\n"
			"Check the extension of the file you are trying to install.");
	case Client::RepoConfigurationError :
	    return i18n("Repository configuration was invalid and could not be read.");
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
	    return i18n("There was a (possibly temporary) problem connecting to a software source\n"
			"Please check the detailed error for further details.");
	case Client::InvalidPackageFile :
	    return i18n("The package you are attempting to install is not valid.\n"
			"The package file could be corrupt, or not a proper package.");
	case Client::PackageInstallBlocked :
	    return i18n("Installation of this package prevented by your packaging system's configuration.");
	case Client::PackageCorrupt :
	    return i18n("The package that was downloaded is corrupt and needs to be downloaded again.");
	case Client::AllPackagesAlreadyInstalled :
	    return i18n("All of the packages selected for install are already installed on the system.");
	case Client::FileNotFound :
	    return i18n("The specified file could not be found on the system.\n"
			 "Check the file still exists and has not been deleted.");
	case Client::NoMoreMirrorsToTry :
	    return i18n("Required data could not be found on any of the configured software sources.\n"
			 "There were no more download mirrors that could be tried.");
	case Client::UnknownErrorType :
	    return i18n("Unknown error, please report a bug.\n"
			"More information is available in the detailed report.");
	default :
	    kDebug() << "error unrecognised: " << error;
	    return QString();
    }
}

QString KpkStrings::groups(Client::Group group)
{
    switch (group) {
        case Client::Accessibility :
	    return i18n("Accessibility");
        case Client::Accessories :
	    return i18n("Accessories");
        case Client::AdminTools :
	    return i18n("Admin tools");
        case Client::Communication :
	    return i18n("Communication");
        case Client::DesktopGnome :
	    return i18n("GNOME desktop");
        case Client::DesktopKde :
	    return i18n("KDE desktop");
        case Client::DesktopOther :
	    return i18n("Other desktops");
        case Client::DesktopXfce :
	    return i18n("XFCE desktop");
        case Client::Education :
	    return i18n("Education");
        case Client::Fonts :
	    return i18n("Fonts");
        case Client::Games :
	    return i18n("Games");
        case Client::Graphics :
	    return i18n("Graphics");
        case Client::Internet :
	    return i18n("Internet");
        case Client::Legacy :
	    return i18n("Legacy");
        case Client::Localization :
	    return i18n("Localization");
        case Client::Maps :
	    return i18n("Maps");
        case Client::Multimedia :
	    return i18n("Multimedia");
        case Client::Network :
	    return i18n("Network");
        case Client::Office :
	    return i18n("Office");
        case Client::Other :
	    return i18n("Other");
        case Client::PowerManagement :
	    return i18n("Power management");
        case Client::Programming :
	    return i18n("Development");
        case Client::Publishing :
	    return i18n("Publishing");
        case Client::Repos :
	    return i18n("Software sources");
        case Client::Science :
	    return i18n("Science");
        case Client::Documentation :
	    return i18n("Documentation");
        case Client::Electronics :
	    return i18n("Electronics");
        case Client::MetaPackages ://TODO check this one
	    return i18n("Package collections");
        case Client::Security :
	    return i18n("Security");
        case Client::Servers :
	    return i18n("Servers");
        case Client::System :
	    return i18n("System");
        case Client::Virtualization :
	    return i18n("Virtualization");
        case Client::UnknownGroup :
	    return i18n("Unknown group");
	default :
	    kDebug() << "group unrecognised: " << group;
	    return QString();
    }
}

QString KpkStrings::info(Package::State state)
{
    switch (state) {
        case Package::Low :
	    return i18n("Trivial update");
        case Package::Normal :
	    return i18n("Update");
        case Package::Important :
	    return i18n("Important update");
        case Package::Security :
	    return i18n("Security update");
        case Package::Bugfix :
	    return i18n("Bug fix update");
        case Package::Enhancement :
	    return i18n("Enhancement update");
        case Package::Blocked :
	    return i18n("Blocked update");
        case Package::Installed :
	    return i18n("Installed");
        case Package::Available :
	    return i18n("Available");
        case Package::UnknownState :
	    return i18n("Unknown update");
        default :
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
	    return i18np("1 update", "%1 updates", number);
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
	case Package::Installed:
	    return i18np("1 installed package", "%1 installed packages", number);
	case Package::Available:
	    return i18np("1 available package", "%1 available packages", number);
        default :
	    kDebug() << "update info unrecognised: " << state;
	    return i18np("1 unknown update", "%1 unknown updates", number);
    }
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
        default :
	    kDebug() << "value unrecognised: " << value;
	    return QString();
    }
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
        default :
	    kDebug() << "value unrecognised: " << value;
	    return QString();
    }
}

#include "KpkStrings.moc"
