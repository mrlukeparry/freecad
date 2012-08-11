/***************************************************************************
 *   Copyright (c) Luke Parry          (l.parry@warwick.ac.uk)    2012     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef _AppearancesModel_h_
#define _AppearancesModel_h_

#include <QAbstractListModel>

#include <Mod/Raytracing/App/LibraryMaterial.h>

using namespace Raytracing;

namespace RaytracingGui
{
  
class RaytracingGuiExport AppearancesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum AppearancesRoles {
         MaterialIdRole = Qt::UserRole + 1,
         LabelRole,
         DescriptionRole,
         PreviewFilenameRole
     };

    AppearancesModel(QObject *parent = 0);

    void addLibraryMaterial(LibraryMaterial *mat);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

private:
    QList<LibraryMaterial *> m_libMats;
};

}

#endif // AppearancesModel