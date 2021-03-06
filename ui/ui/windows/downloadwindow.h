/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include <QDialog>
#include <QWidget>

#include <memory>

#include "lt/link/platformlink.h"

class QLineEdit;
class QComboBox;
class AuthOptionsView;
class ProjectCombo;

namespace lt {
class Project;
using ProjectPtr = std::shared_ptr<Project>;
}

/**
 * Downloads ROM on connected control unit.
 */
class DownloadWindow : public QDialog
{
    Q_OBJECT
public:
    explicit DownloadWindow(lt::ProjectPtr project = lt::ProjectPtr(), QWidget * parent = nullptr);
    ~DownloadWindow() override;

public slots:
    void download();
    void platformChanged(int index);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent * event) override;

private:
    QComboBox * comboPlatform_;
    ProjectCombo * comboProject_;
    QLineEdit * lineName_;
    AuthOptionsView * authOptions_;

    lt::PlatformLink getPlatformLink();
};

#endif // DOWNLOADWINDOW_H
