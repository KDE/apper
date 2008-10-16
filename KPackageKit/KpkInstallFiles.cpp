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

#include "KpkInstallFiles.h"

#include <KMessageBox>

#include <KDebug>

KpkInstallFiles::KpkInstallFiles( QObject *parent ) :
 QObject( parent ), m_running(0)
{
}

KpkInstallFiles::~KpkInstallFiles()
{
}

void KpkInstallFiles::installFiles(KCmdLineArgs *args)
{
    // yeah we are running so please be
    // polited and don't close the application :P
    m_running++;

//     KUrl::List urlList;

    QStringList files;
    QStringList notFiles;
    QString lastDirectory = args->url(0).directory();
    QString lastDirectoryNotFiles = args->url(0).directory();
    bool showFullPath = false;
    bool showFullPathNotFiles = false;
    for ( int i = 0; i < args->count(); i++) {
	if ( QFileInfo( args->url(i).path() ).isFile() ) {
	    qDebug() << "isFIle";
// 	    urlList << args.url(i);
	    files << args->url(i).path();
	    // if the path of all the files is the same
	    // why bothering the user showing a full path?
	    if ( args->url(i).directory() != lastDirectory )
		showFullPath = true;
	    lastDirectory = args->url(i).directory();
	}
	else {
	    qDebug() << "~isFIle";
	    notFiles << args->url(i).path();
	    if ( args->url(i).directory() != lastDirectoryNotFiles )
		showFullPathNotFiles = true;
	    lastDirectoryNotFiles =args->url(i).directory();
	}
    }

    // check if there were "false" files
    if ( notFiles.count() ) {
	if (!showFullPathNotFiles)
	    for(int i = 0; i < notFiles.count(); i++) {
		notFiles[i] = KUrl( notFiles.at(i) ).fileName();
	    }
	    KMessageBox::errorList(0,
		i18np("This item is not supported by your backend or it is not a file", "These itens are not supported by your backend or they are not files", notFiles.count() ),
		notFiles,
		i18n("Impossible to install")
	    );
    }
	
    if ( files.count() ) {
	QStringList displayFiles = files;
	if (!showFullPath)
	    for(int i = 0; i < displayFiles.count(); i++) {
		displayFiles[i] = KUrl( displayFiles.at(i) ).fileName();
	    }

	KGuiItem installBt = KStandardGuiItem::yes();
	installBt.setText( i18n("Install") );

	if ( KMessageBox::questionYesNoList(0,
			i18np("Do you want to install this file?", "Do you want to install these files?", displayFiles.count() ),
			displayFiles,
			i18n("Install?"),
			installBt
      
		) == KMessageBox::Yes ) {
	    if ( Transaction *t = Client::instance()->installFiles(files, true) ) {
		KpkTransaction *trans = new KpkTransaction(t, this);
		connect( trans, SIGNAL( kTransactionFinished(KpkTransaction::ExitStatus) ), this, SLOT( installFilesFinished(KpkTransaction::ExitStatus) ) );
		trans->show();
		m_transactionFiles[trans] = files;
		//to skip the running thing
		return;
	    }
	    else {
		KMessageBox::error( 0, i18n("Authentication failed"), i18n("KPackageKit") );
	    }
	}
    }
    // ok we are not running anymore..
    m_running--;
    emit appClose();
}

void KpkInstallFiles::installFilesFinished(KpkTransaction::ExitStatus status)
{
    switch (status) {
	case KpkTransaction::Success :
	case KpkTransaction::Cancelled :
	    m_transactionFiles.remove( (KpkTransaction *) sender() );
	    break;
	case KpkTransaction::Failed :
	    m_transactionFiles.remove( (KpkTransaction *) sender() );
	    KMessageBox::error( 0, i18n("Sorry an error occured"), i18n("Erro KPackageKit") );
	    break;
	case KpkTransaction::ReQueue :
	    kDebug() << "ReQueue";
	    KpkTransaction *trans = (KpkTransaction *) sender();
	    trans->setTransaction( Client::instance()->installFiles(m_transactionFiles[trans], false) );
	    // return to avoid the running--
	    return;
    }
    m_running--;
    emit appClose();
}

#include "KpkInstallFiles.moc"
