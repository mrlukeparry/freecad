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


#ifndef RAYTRACINGGUI_TaskDlgRender_H
#define RAYTRACINGGUI_TaskDlgRender_H

#include <boost/signals.hpp>

#include <QDeclarativeView>
#include <QAbstractListModel>

#include <Mod/Raytracing/App/RenderFeature.h>
#include <Gui/Selection.h>
#include <Gui/TaskView/TaskDialog.h>

#include <Mod/Raytracing/App/RenderPreset.h>
#include "ViewProviderRender.h"

class SbBox3f;

namespace RaytracingGui {

// Helper Classes
class RaytracingGuiExport PresetsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY( int count READ getCount() NOTIFY countChanged())

Q_SIGNALS:
  void countChanged();
public:
    enum AppearancesRoles {
         IdRole = Qt::UserRole + 1,
         DescriptionRole,
         LabelRole
     };


    PresetsModel(QObject *parent = 0);
    ~PresetsModel(){}

    int getCount() { this->count = this->rowCount(); return count; }

    Q_INVOKABLE QVariant getById(QString id);
    Q_INVOKABLE QVariant get(int row);

    void addRenderPreset(Raytracing::RenderPreset *preset);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

private:
    QList<Raytracing::RenderPreset *> m_libPresets;
    int count;
};

class RaytracingGuiExport TemplatesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY( int count READ getCount() NOTIFY countChanged())

Q_SIGNALS:
    void countChanged();
  
public:
    enum AppearancesRoles {
         IdRole = Qt::UserRole + 1,
         DescriptionRole,
         LabelRole
     };

    TemplatesModel(QObject *parent = 0);
    ~TemplatesModel(){}

    Q_INVOKABLE QVariant getById(QString id);
    Q_INVOKABLE QVariant get(int row);

    void addRenderTemplate(Raytracing::RenderTemplate *renderTemplate);
    int getCount() { this->count = this->rowCount(); return count; }
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

private:
    QList<Raytracing::RenderTemplate *> m_rendTemplates;
    int count;
};

class RaytracingGuiExport MaterialsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY( int count READ getCount() NOTIFY countChanged())

Q_SIGNALS:
    void countChanged();

public:
    enum AppearancesRoles {
         IdRole = Qt::UserRole + 1,
         DescriptionRole,
         LabelRole,
         LinkLabelRole,
         SelectedRole
     };

    MaterialsModel(QObject *parent = 0);
    ~MaterialsModel(){}

//     Q_INVOKABLE QVariant getById(QString id);
//     Q_INVOKABLE QVariant get(int row);

    Q_INVOKABLE void clearSelection();
    const std::vector<bool> & getSelection() const { return selectList; }
    Q_INVOKABLE bool setState(int row, const QVariant & value);
    
    void addRenderMaterial(Raytracing::RenderMaterial *mat);
    int getCount() { this->count = this->rowCount(); return count; }
    int rowCount(const QModelIndex & parent = QModelIndex()) const;


    void clear() { m_rendMats.clear(); selectList.clear(); reset(); }
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

private:
    QList<Raytracing::RenderMaterial *> m_rendMats;
    std::vector<bool> selectList;
    int count;
};

class RaytracingGuiExport RenderFeatureData : public QObject
{
    Q_OBJECT
public:

    RenderFeatureData(Raytracing::RenderFeature *feat) : feature(feat) {}
    ~RenderFeatureData(){}
    // Getters
    Q_INVOKABLE int getOutputX() const { return (int) feature->OutputX.getValue(); }
    Q_INVOKABLE int getOutputY() const { return (int) feature->OutputY.getValue(); }
    Q_INVOKABLE QString getRenderPreset() const { return QString::fromAscii(feature->Preset.getValue()); }
    Q_INVOKABLE QString getRenderTemplate() const { return QString::fromAscii(feature->SceneTemplate.getValue()); }
    Q_INVOKABLE int getUpdateInterval() const { return (int) (feature->UpdateInterval.getValue()); }

    // Setters
    Q_INVOKABLE void setOutputX( int x) { feature->OutputX.setValue(x); }
    Q_INVOKABLE void setOutputY( int y) { feature->OutputY.setValue(y); }
    Q_INVOKABLE void setRenderPreset(QString val) {feature->Preset.setValue(val.toAscii()); }
    Q_INVOKABLE void setRenderTemplate(QString val) {feature->SceneTemplate.setValue(val.toAscii()); }
    Q_INVOKABLE void setUpdateInterval( int msecs) { feature->UpdateInterval.setValue(msecs); }

private:
    Raytracing::RenderFeature *feature;
};

/// simulation dialog for the TaskView
class RaytracingGuiExport TaskDlgRender : public Gui::TaskView::TaskDialog, public Gui::SelectionObserver
{
    Q_OBJECT

public:
    TaskDlgRender(ViewProviderRender *renderView);
    ViewProviderRender* getRenderView() const { return renderView; }
    ~TaskDlgRender();

Q_SIGNALS:
  void renderStart();
  void renderStop();

public Q_SLOTS:
  void preview();
  void previewWindow();
  void render();
  void renderStarted();
  void renderStopped();
  void saveCamera();
  // Materials Slots
  void onMaterialSelectionChanged(int index);
  void onEditMaterial(int index);
public:

    void slotMaterialsChanged();

    /// Observer message from the Selection
    void onSelectionChanged(const Gui::SelectionChanges& msg);

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
    virtual QDialogButtonBox::StandardButtons getStandardButtons(void) const
    { return QDialogButtonBox::Close|QDialogButtonBox::Help; }

protected:
  typedef boost::BOOST_SIGNALS_NAMESPACE::connection Connection;
  Connection connectionMaterialsChanged;

  bool isRenderActive();
  ViewProviderRender *renderView;
  QDeclarativeView   *view ;
  RenderFeatureData  *featViewData;
  PresetsModel       *presetsModel;
  TemplatesModel     *templatesModel;
  MaterialsModel     *materialsModel;
};



} //namespace RaytracingGui

#endif
