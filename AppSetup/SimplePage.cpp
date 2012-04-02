/* This file is part of Apper
 *
 * Copyright (C) 2012 Matthias Klumpp <matthias@tenstral.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "SimplePage.h"
#include "ui_SimplePage.h"

#include <QGroupBox>

#include <KTextBrowser>
#include <KDebug>

SimplePage::SimplePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimplePage)
{
    ui->setupUi(this);
}

SimplePage::~SimplePage()
{
    delete ui;
}

void SimplePage::setTitle(const QString& title)
{
    setWindowTitle(title);
}


void SimplePage::setDescription(const QString &description)
{
    ui->descriptionL->setText(description);
}

void SimplePage::setDetails(const QString &details, bool isHtml)
{
    if (!details.isEmpty()) {
        KTextBrowser *browser = new KTextBrowser(this);
        browser->setFrameShape(QFrame::NoFrame);
        browser->setFrameShadow(QFrame::Plain);
        browser->setStyleSheet("QTextEdit {\nbackground-color: transparent;\n};");
	if (isHtml)
	    browser->setHtml(details);
	else
	    browser->setText(details);
        ui->descriptionLayout->addWidget(browser);
        ui->descriptionLayout->insertSpacing(0, 20);
    }
}

void SimplePage::addWidget(QWidget *widget)
{
    if (widget) {
        ui->descriptionLayout->insertSpacing(0, 20);
        ui->descriptionLayout->addWidget(widget);
    }
}

void SimplePage::reset()
{
    setTitle("");
    setDescription("");
    setDetails("");
    QList<KTextBrowser*> detailsB = findChildren<KTextBrowser*>();
    foreach (KTextBrowser* browser, detailsB) {
	browser->setVisible(false);
	delete browser;
    }
}
