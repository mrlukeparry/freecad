/***************************************************************************
 *   Copyright (c) 2007 Jürgen Riegel <juergen.riegel@web.de>              *
 *                                                                         *
 *   This file is Drawing of the FreeCAD CAx development system.           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A DrawingICULAR PURPOSE.  See the      *
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

#include "RenderView.h"

#include <Base/Console.h>
#include <QFileDialog>
#include <Base/PyObjectBase.h>

#include <Gui/Command.h>
#include <Gui/FileDialog.h>
#include <Gui/WaitCursor.h>

#include <QGraphicsObject>
#include <QGLWidget>
#include <QDeclarativeImageProvider>
#include <QDeclarativeEngine>
#include <QRegExp>
using namespace RaytracingGui;

// ----------------------------------------------------------------------------

class ImageProvider : public QDeclarativeImageProvider
{
public:
    ImageProvider(): QDeclarativeImageProvider(QDeclarativeImageProvider::Image){}
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize)
    {
      QRegExp reg(QString::fromAscii("^reload/(.*)$"));
      reg.indexIn(id);
      QString str = QString::fromAscii("picture");

      if(id == str ||  reg.cap(1)  == str)
          return image;

      QImage img;
      return img;
      }

      void setImage(QImage img) { image = img;}
private:
  QImage image;
};

RenderView::RenderView(Gui::Document* doc, QWidget* parent)
  : Gui::MDIView(doc, parent)
{

    qmlRegisterType<WheelArea>("FreeCAD", 1, 0, "WheelArea");
      
    view = new QDeclarativeView (this);

    view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view->setSource(QUrl(QString::fromAscii("qrc:/qml/renderPreviewUi.qml"))); // Load the Main QML File

    // Needed to improve performance by using Opengl Backend which should be guaranteed within Freecad
    QGLFormat format = QGLFormat::defaultFormat();
    // You can comment the next line if the graphical results are not acceptable
    format.setSampleBuffers(false);
    QGLWidget *glWidget = new QGLWidget(format);
    // Comment the following line if you get display problems
    // (usually when the top-level element is an Item and not a Rectangle)
    glWidget->setAutoFillBackground(false);
    view->setViewport(glWidget);

    view->setAttribute(Qt::WA_OpaquePaintEvent);
    view->setAttribute(Qt::WA_NoSystemBackground);
    view->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    view->viewport()->setAttribute(Qt::WA_NoSystemBackground);

    // Image provider contains the image file loaded that can be accessed through QML
    imgProv = new ImageProvider();
    view->engine()->addImageProvider(QString::fromAscii("previewImage"), imgProv);

    // Connect an Update Signal when an image is available
    QObject *rootObject = view->rootObject();
    QObject::connect(this, SIGNAL(updatePreview()), rootObject , SLOT(updatePreview())); 

    // Set the QML View to MDI window
    setCentralWidget(view);
}

bool RenderView::onMsg(const char* pMsg, const char** ppReturn)
{
    return false;
}

void RenderView::attachRender(Renderer *attachedRender)
{
    render = attachedRender;

    // When the render process polls with a successful update connect this classes signal
    QObject *renderProcQObj = qobject_cast<QObject *>(render->getRenderProcess());
    QObject::connect(renderProcQObj , SIGNAL(updateOutput()), this , SLOT(updateOutput()));

    view->show();

    // Connect signals from the QML Preview to Slots in RenderView
    QObject *item = view->rootObject();
    QObject::connect(item , SIGNAL(stopRender()), this , SLOT(stopRender()) );
    QObject::connect(item , SIGNAL(saveOutput()), this , SLOT(saveRender()) );
    QObject::connect(renderProcQObj , SIGNAL(finished()), item , SLOT(renderStopped())); // Connect Render Process Signal when stopped by user
    QObject::connect(renderProcQObj , SIGNAL(started()), item , SLOT(renderActive())); // Connect Render Process (QProcess) start signal when active
    // TODO create a connection when error

}

void RenderView::saveRender()
{
    QFile file(QString::fromAscii(render->getOutputPath()));
    file.read(QFile::ReadOnly);

    QFileInfo fileInfo(file);
    QString ext = fileInfo.suffix();

    QString selectString = QString::fromAscii("*.").append(ext);
    QString fileName = Gui::FileDialog::getSaveFileName(this,
        QObject::tr("Save Render"), QString(), selectString, &selectString);


    if (!fileName.isEmpty()) {
       bool result = QFile::copy(QString::fromAscii(render->getOutputPath()), fileName);
    }
}
void RenderView::stopRender()
{
    // Only one render process is assigned per Render Feature, so it should be safe to call this.
    render->getRenderProcess()->stop();
}

bool RenderView::onHasMsg(const char* pMsg) const
{
    return false;
}

void RenderView::updateOutput()
{
    // Attempt to load the image
    QImage img;
    if( !img.load(QString::fromAscii(render->getOutputPath())) )
        return; // The file may be being written to, attempt to read later or perhaps set poll.

    imgProv->setImage(img);

    Q_EMIT updatePreview();
}

PyObject* RenderView::getPyObject()
{
    Py_Return;
}

#include "moc_RenderView.cpp"
