/***************************************************************************
 *   Copyright (C) 2008-2009 by Daniel Nicoletti                           *
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

#include "KpkInstallFiles.h"

#include <KLocale>
#include <KMessageBox>
#include <KProtocolManager>

#include <KDebug>

KpkInstallFiles::KpkInstallFiles(const KUrl::List &urls, QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_urls(urls)
{
}

KpkInstallFiles::~KpkInstallFiles()
{
}

void KpkInstallFiles::start()
{
    // yeah we are running so please be
    // polited and don't close the application :P
    increaseRunning();

    QStringList files;
    QStringList notFiles;
    QString lastDirectory = m_urls.at(0).directory();
    QString lastDirectoryNotFiles = m_urls.at(0).directory();
    bool showFullPath = false;
    bool showFullPathNotFiles = false;
    for (int i = 0; i < m_urls.count(); i++) {
        if (QFileInfo(m_urls.at(i).path()).isFile()) {
            qDebug() << "isFIle";
            files << m_urls.at(i).path();
            // if the path of all the files is the same
            // why bothering the user showing a full path?
            if (m_urls.at(i).directory() != lastDirectory) {
                showFullPath = true;
            }
            lastDirectory = m_urls.at(i).directory();
        } else {
            qDebug() << "~isFIle";
            notFiles << m_urls.at(i).path();
            if (m_urls.at(i).directory() != lastDirectoryNotFiles) {
                showFullPathNotFiles = true;
            }
            lastDirectoryNotFiles =m_urls.at(i).directory();
        }
    }

    // check if there were "false" files
    if (notFiles.count()) {
        if (!showFullPathNotFiles)
            for(int i = 0; i < notFiles.count(); i++) {
                notFiles[i] = KUrl(notFiles.at(i)).fileName();
            }
            KMessageBox::errorList(0,
                i18np("This item is not supported by your backend, or it is not a file.",
                      "These items are not supported by your backend, or they are not files.",
                      notFiles.count()),
                notFiles,
                i18n("Impossible to install")
            );
    }

    if (files.count()) {
        QStringList displayFiles = files;
        if (!showFullPath) {
            for(int i = 0; i < displayFiles.count(); i++) {
                displayFiles[i] = KUrl(displayFiles.at(i)).fileName();
            }
        }

        KGuiItem installBt = KStandardGuiItem::yes();
        installBt.setText(i18n("Install"));

        int ret;
        ret = KMessageBox::questionYesNoList(0,
                        i18np("Do you want to install this file?", "Do you want to install these files?",
                        displayFiles.count()),
                        displayFiles,
                        i18n("Install?"),
                        installBt);
        if (ret == KMessageBox::Yes) {
            // Just in case it want to download some dependency
            Client::instance()->setProxy(KProtocolManager::proxyFor("http"), KProtocolManager::proxyFor("ftp"));
            if (Transaction *t = Client::instance()->installFiles(files, true)) {
                KpkTransaction *trans = new KpkTransaction(t);
                connect(trans, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                        this, SLOT(installFilesFinished(KpkTransaction::ExitStatus)));
                trans->show();
                m_transactionFiles[trans] = files;
                // return to avoid the decreaseRunning()
                return;
            } else {
                KMessageBox::sorry(0,
                                   i18n("You do not have the necessary privileges to perform this action."),
                                   i18np("Failed to install file",
                                         "Failed to install files", displayFiles.count()));
            }
        } else {
            KMessageBox::sorry(0, i18np("The file was not installed",
                                        "The files were not installed", displayFiles.count()),
                                  i18np("The file was not installed",
                                        "The files were not installed", displayFiles.count()));
        }
    }
    // ok we are not running anymore..
    decreaseRunning();
}

void KpkInstallFiles::installFilesFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
    KpkTransaction *transaction = (KpkTransaction *) sender();
    switch (status) {
        case KpkTransaction::Success :
            KMessageBox::information(0, i18np("File was installed successfully",
                                              "Files were installed successfully",
                                              m_transactionFiles[transaction].count()),
                                        i18np("File was installed successfully",
                                              "Files were installed successfully",
                                              m_transactionFiles[transaction].count()));
        case KpkTransaction::Cancelled :
            m_transactionFiles.remove(transaction);
            break;
        case KpkTransaction::Failed :
            m_transactionFiles.remove(transaction);
            KMessageBox::error(0, i18n("An error occurred."), i18n("KPackageKit Error"));
            break;
        case KpkTransaction::ReQueue :
            kDebug() << "ReQueue";
            Client::instance()->setProxy(KProtocolManager::proxyFor("http"), KProtocolManager::proxyFor("ftp"));
            transaction->setTransaction(Client::instance()->installFiles(m_transactionFiles[transaction], false));
            // return to avoid the decreaseRunning()
            return;
    }
    decreaseRunning();
}

#include "KpkInstallFiles.moc"
