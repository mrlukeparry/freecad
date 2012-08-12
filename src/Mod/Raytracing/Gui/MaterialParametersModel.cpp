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

#include "PreCompiled.h"
#ifndef _PreComp_
#endif

#include <QColor>

#include "MaterialParametersModel.h"

using namespace Raytracing;
using namespace RaytracingGui;

MaterialParametersModel::MaterialParametersModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[ParamIdRole] = "paramId";
    roles[LabelRole] = "label";
    roles[DescriptionRole] = "description";
    roles[Default] = "default";
    roles[ParamType] = "type";
    roles[RangeMin] = "minValue";
    roles[RangeMax] = "maxValue";
    setRoleNames(roles);
}

void MaterialParametersModel::addParameter(MaterialParameter *param)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_params << param;
    endInsertRows();
}

int MaterialParametersModel::rowCount(const QModelIndex & parent) const {
    return m_params.count();
}

QVariant MaterialParametersModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() > m_params.count())
        return QVariant();

    const MaterialParameter *param = m_params[index.row()];
    if (role == ParamIdRole)
        return param->getId();
    else if (role == ParamType) {
        switch (param->getType()) {
          case MaterialParameter::BOOL:
            return QString::fromAscii("bool");
          break;
          case MaterialParameter::COLOR:
            return QString::fromAscii("color");
          break;
          case MaterialParameter::FLOAT:
            return QString::fromAscii("float");
          break;
        }
    } else if (role == LabelRole)
        return param->getLabel();
    else if (role == DescriptionRole)
        return param->getDescription();
    else if (role == Default) {
        switch(param->getType()) {
          case MaterialParameter::COLOR: {
              const MaterialParameterColor *tmp = static_cast<const MaterialParameterColor*>(param);
              const float *color = tmp->getValue();
              return QColor(color[0], color[1], color[2]);
          } break;
          case MaterialParameter::BOOL: {
              const MaterialParameterBool *tmp = static_cast<const MaterialParameterBool*>(param);
              return tmp->getValue();
          } break;
          case MaterialParameter::FLOAT: {
              const MaterialParameterFloat *tmp = static_cast<const MaterialParameterFloat*>(param);
              return tmp->getValue();
          } break;
          default:
            return false;
        }
    } else if (role == RangeMin || role == RangeMax) {
        if(param->getType() == MaterialParameter::FLOAT) {
            const MaterialParameterFloat *tmp = static_cast<const MaterialParameterFloat*>(param);
            if(tmp->hasRange())
                return (bool) false;

            float min, max;
            tmp->getRange(min, max);
            return (role == RangeMin) ? min : max;
        }
    } else
        return QVariant();
}

#include "moc_MaterialParametersModel.cpp"