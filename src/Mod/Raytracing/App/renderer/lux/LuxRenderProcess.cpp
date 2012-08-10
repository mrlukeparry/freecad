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
#include "../../PreCompiled.h"
#ifndef _PreComp_
#endif

#include <App/Application.h>
#include <Base/Console.h>
#include <Base/Exception.h>

#include "LuxRenderProcess.h"

using namespace Raytracing;

LuxRenderProcess::LuxRenderProcess(void) {}
LuxRenderProcess::~LuxRenderProcess(void) {};

void LuxRenderProcess::initialiseSettings()
{
    // Set the render process properties
    // For current simplicity assume luxrender is in home

    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Preferences/Mod/Raytracing");
    std::string path = hGrp->GetASCII("luxRenderExecPath");

    execPath = QString::fromStdString(path);
    //outputPath = QString::fromAscii("test.png");

    // Add Output
    args.push_back(QString::fromAscii("-o"));
    args.push_back(outputPath);

   // Append input file
    args.push_back(inputPath);
}

// bool LuxRenderProcess::getOutput(QImage &img)
// {
//     return img.load(outputPath);
// }

// QImage LuxRenderProcess::getPreview()
// {
// 
// }


