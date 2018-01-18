/***************************************************************************
 *   Copyright (C) 2011 by Daniel Nicoletti                                *
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

#include "InfoWidget.h"
#include "ui_InfoWidget.h"


#include <QTextBrowser>
#include <QLoggingCategory>

InfoWidget::InfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWidget)
{
    ui->setupUi(this);
    ui->iconL->setPixmap(QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(128, 128));
}

InfoWidget::~InfoWidget()
{
    delete ui;
}

void InfoWidget::setIcon(const QIcon &icon)
{
    ui->iconL->setPixmap(icon.pixmap(128, 128));
}

void InfoWidget::setDescription(const QString &description)
{
    ui->descriptionL->setText(description);
}

void InfoWidget::setDetails(const QString &details)
{
    if (!details.isEmpty()) {
        auto browser = new QTextBrowser(this);
        browser->setFrameShape(QFrame::NoFrame);
        browser->setFrameShadow(QFrame::Plain);
        browser->setStyleSheet(QLatin1String("QTextEdit {\nbackground-color: transparent;\n};"));
        browser->setText(details);
        ui->descriptionLayout->addWidget(browser);
        ui->descriptionLayout->insertSpacing(0, 20);
    }
}

void InfoWidget::addWidget(QWidget *widget)
{
    if (widget) {
        ui->descriptionLayout->insertSpacing(0, 20);
        ui->descriptionLayout->addWidget(widget);
    }
}

void InfoWidget::reset()
{
    ui->iconL->setPixmap(QIcon::fromTheme(QLatin1String("dialog-information")).pixmap(128, 128));
    setWindowTitle(QLatin1String(""));
    setDescription(QLatin1String(""));
    setDetails(QLatin1String(""));
}

#include "moc_InfoWidget.cpp"
