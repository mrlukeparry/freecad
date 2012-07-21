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


#ifndef _RenderProcess_h_
#define _RenderProcess_h_

#include "PreCompiled.h"
#include <QProcess>
#include <QImage>
#include <QString>
# include <TopoDS.hxx>
# include <gp_Vec.hxx>
# include <TopoDS_Face.hxx>
#include <vector>

namespace Raytracing
{

class RenderProcess : public QProcess
{
  Q_OBJECT

public:
  enum State {
    INVALID,
    VALID,
    STARTED,
    RUNNING,
    FINISHED,
    TERMINATED,
    ERROR,
  };

  RenderProcess(void);
  ~RenderProcess(void);

  void addArguments(const QString &arg);
  void begin(void);
  void stop();

  bool isActive();
  State getState() { return status; };

  bool isExecPathValid(void);
//   virtual QImage getOutput() = 0;
//   virtual QImage getPreview() = 0;

  virtual bool isOutputAvailable();

  void setArguments(const QStringList &args);
  void setExecPath(const QString &str);
  void setInputPath(const QString &str);
  void setOutputPath(const QString &str);

protected:
 
  QStringList args;
  QString execPath;
  QString inputPath;
  QString tmpPath;
  QString outputPath;
  State status;
};

}

#endif //_RenderProcess_h_
