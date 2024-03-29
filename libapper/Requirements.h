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

#ifndef REQUIREMENTS_H
#define REQUIREMENTS_H

#include <QDialog>

#include <QButtonGroup>
#include <QToolButton>

namespace Ui {
    class Requirements;
}

class PackageModel;
class Requirements : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool embedded READ embedded WRITE setEmbedded USER true)
public:
    explicit Requirements(PackageModel *model, QWidget *parent = nullptr);
    ~Requirements() override;

    bool embedded() const;
    void setEmbedded(bool embedded);
    void setDownloadSizeRemaining(qulonglong size);
    bool trusted() const;

public Q_SLOTS:
    bool shouldShow() const;

protected Q_SLOTS:
     virtual void slotButtonClicked(int button);

private Q_SLOTS:
    void actionClicked(int type);

private:
    void confirmCBChanged(bool checked);
    void showUntrustedButton();

    bool m_embed = false;
    bool m_shouldShow = true;
    bool m_hideAutoConfirm = false;
    QToolButton *m_untrustedButton = nullptr;
    QButtonGroup *m_buttonGroup;
    Ui::Requirements *ui;
};

#endif
