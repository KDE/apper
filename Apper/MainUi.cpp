/***************************************************************************
 *   Copyright (C) 2009-2018 by Daniel Nicoletti                           *
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

#include "MainUi.h"

#include <QLayout>
#include <QLoggingCategory>

#include <KConfig>
#include <KConfigGroup>
#include <QIcon>
#include <QDialog>
#include <KLocalizedString>

#include "ApperKCM.h"

Q_DECLARE_LOGGING_CATEGORY(APPER)

MainUi::MainUi(QWidget *parent) :
    QMainWindow(parent),
    m_apperModule(0)
{
    setWindowIcon(QIcon::fromTheme(QLatin1String("system-software-install")));
    setWindowTitle(i18n("Apper"));

    KConfig config(QLatin1String("apper"));
    KConfigGroup configGroup(&config, "MainUi");
    restoreGeometry(configGroup.readEntry("geometry", QByteArray()));
    restoreState(configGroup.readEntry("state", QByteArray()));

    m_apperModule = new ApperKCM(this);
    setCentralWidget(m_apperModule);
    connect(m_apperModule, &ApperKCM::caption, this, &MainUi::setWindowTitle);
}

MainUi::~MainUi()
{
}

void MainUi::showAll()
{
    if (m_apperModule) {
        m_apperModule->setProperty("page", QLatin1String("home"));
    }
}

void MainUi::showUpdates()
{
    if (m_apperModule) {
        m_apperModule->setProperty("page", QLatin1String("updates"));
    }
}

void MainUi::showSettings()
{
    if (m_apperModule) {
        m_apperModule->setProperty("page", QLatin1String("settings"));
    }
}

void MainUi::closeEvent(QCloseEvent *event)
{
    KConfig config(QLatin1String("apper"));
    KConfigGroup configGroup(&config, "MainUi");
    configGroup.writeEntry("geometry", saveGeometry());
    configGroup.writeEntry("state", saveState());
    QMainWindow::closeEvent(event);

    emit finished();
}

#include "moc_MainUi.cpp"
