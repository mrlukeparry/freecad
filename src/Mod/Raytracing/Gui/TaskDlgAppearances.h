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


#ifndef RAYTRACINGGUI_TaskDlgAppearances_H
#define RAYTRACINGGUI_TaskDlgAppearances_H

#include <QDeclarativeView>
#include <QVariant>
#include <Gui/TaskView/TaskDialog.h>
#include <Mod/Raytracing/App/RenderMaterial.h>

namespace RaytracingGui {

  class RaytracingGuiExport RenderMaterialData : public QObject
{
    Q_OBJECT
public:

    RenderMaterialData(Raytracing::RenderMaterial *mat) : material(mat) {}
    ~RenderMaterialData()
    {
        // Unsure if the model should be responsible for deleting pointer;
        delete material;
        material = 0;
    }

    Q_INVOKABLE QVariant getPropertyValue (QString paramId) const {
      // Look up materialparemter
      Raytracing::MaterialParameter * param = findMaterialParameter(paramId);

      if(!param)
        return QVariant();

      Raytracing::MaterialProperty * const prop = material->Properties.value(paramId); // Pointer cannot change but value can

      if(!prop)
        return QVariant();
      switch(param->getType()) {
        case Raytracing::MaterialParameter::BOOL:{
            Raytracing::MaterialBoolProperty *myProp = static_cast<Raytracing::MaterialBoolProperty *>(prop);
            return QVariant(myProp->getValue());
        } break;
        case Raytracing::MaterialParameter::FLOAT:{
              Raytracing::MaterialFloatProperty *myProp = static_cast<Raytracing::MaterialFloatProperty *>(prop);
              return QVariant(myProp->getValue());

        } break;
        case Raytracing::MaterialParameter::COLOR:{
              Raytracing::MaterialColorProperty *myProp = static_cast<Raytracing::MaterialColorProperty *>(prop);
              float *colorArray = myProp->getValue();
              return QVariant(QColor(colorArray[0], colorArray[0], colorArray[0]));
        } break;
      }
    }

    // Setters
    Q_INVOKABLE void setProperty(QString paramId, QVariant variant) {
      // Look up materialparemter

      Raytracing::MaterialParameter *param = findMaterialParameter(paramId);

      if(!param)
        return;
      Raytracing::MaterialProperty * const prop = material->Properties.value(paramId); // Pointer cannot change but value can

      switch(param->getType()) {
        case Raytracing::MaterialParameter::BOOL:{
          if(!variant.type() == QVariant::Bool)
              return; //invalid variant
          if(!prop) {
            // Create a new property in Render Material
            Raytracing::MaterialBoolProperty *myProp = new  Raytracing::MaterialBoolProperty(false);
            myProp->setValue(variant.toBool());
            material->Properties.insert(paramId, myProp); // Push into the material properties
          } else {
              Raytracing::MaterialBoolProperty *myProp = static_cast<Raytracing::MaterialBoolProperty *>(prop);
              myProp->setValue(variant.toBool());
          }
        } break;
        case Raytracing::MaterialParameter::FLOAT:{
//           if(!variant.type() == QVariant::Double) TODO
//               return; //invalid variant
          if(!prop) {
            // Create a new property in Render Material
            Raytracing::MaterialFloatProperty *myProp = new  Raytracing::MaterialFloatProperty(0.f);
            myProp->setValue(variant.toFloat());
            material->Properties.insert(paramId, myProp); // Push into the material properties
          } else {
              Raytracing::MaterialFloatProperty *myProp = static_cast<Raytracing::MaterialFloatProperty *>(prop);
              myProp->setValue(variant.toFloat());
          }
          
        } break;
        case Raytracing::MaterialParameter::COLOR:{
          if(!variant.canConvert<QColor>())
              return; //invalid variant

              QColor color = variant.value<QColor>(); // QColor is not in QTCore so we must use a template cast
          if(!prop) {
            // Create a new property in Render Material
            Raytracing::MaterialColorProperty *myProp = new  Raytracing::MaterialColorProperty(color.red(), color.green(), color.blue());
            material->Properties.insert(paramId, myProp); // Push into the material properties
          } else {
              Raytracing::MaterialColorProperty *myProp = static_cast<Raytracing::MaterialColorProperty *>(prop);
              myProp->setValue(color.red(), color.green(), color.blue());
          }
        } break;
      }


    }
    Raytracing::RenderMaterial * getRenderMaterial(){ return material; }
protected:
      Raytracing::MaterialParameter * findMaterialParameter (const QString &paramId) const {
      QMap<QString, Raytracing::MaterialParameter *>params = material->getMaterial()->parameters;

      Raytracing::MaterialParameter *param = 0;
      QMap<QString, Raytracing::MaterialParameter*>::const_iterator i;
        for (i = params.constBegin(); i != params.constEnd(); ++i) {
            if(i.key() == paramId)
                param = i.value();
        }

        return param;
    }
    Raytracing::RenderMaterial *material;
};

/// simulation dialog for the TaskView
class RaytracingGuiExport TaskDlgAppearances : public Gui::TaskView::TaskDialog
{
    Q_OBJECT

public:
    TaskDlgAppearances();
    ~TaskDlgAppearances();

public Q_SLOTS:
  void dragInit(QString id);
  void materialParamSave();
  void materialParamCancel();

public:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    /// is called the TaskView when the dialog is opened
    virtual void open();
    /// is called by the framework if an button is clicked which has no accept or reject role
    virtual void clicked(int);
    /// is called by the framework if the dialog is accepted (Ok)
    virtual bool accept();
    /// is called by the framework if the dialog is rejected (Cancel)
    virtual bool reject();
    /// is called by the framework if the user presses the help button 
    virtual void helpRequested();
    virtual bool isAllowedAlterDocument(void) const
    { return false; }

    /// returns for Close and Help button 
//     virtual QDialogButtonBox::StandardButtons getStandardButtons(void) const
//     { return 0; }

protected:
    void materialDragEvent(QDragMoveEvent *);
    void materialDropEvent(QDropEvent *);
    QDeclarativeView *view;
    QDeclarativeView *paramView;
    RenderMaterialData *materialData;
};



} //namespace RaytracingGui

#endif
