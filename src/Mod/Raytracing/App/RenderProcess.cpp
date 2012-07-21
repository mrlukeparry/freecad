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

#include <Base/Console.h>
#include <QFile>
#include <Base/Exception.h>

#include "RenderProcess.h"

using namespace Raytracing;

RenderProcess::RenderProcess() : status(INVALID) {}
RenderProcess::~RenderProcess() {}

void RenderProcess::setExecPath(const QString &str)
{
    this->execPath = str;
}

void RenderProcess::setOutputPath(const QString &str)
{
    this->outputPath = str;
}

bool RenderProcess::isExecPathValid()
{
    QFile file(this->execPath);
    bool exists = file.exists();
    QFile::Permissions perm = file.permissions();
//     bool isExec = perm == QFile::ExeUser || QFile::ExeUser
    return (file.exists()/* && file.permissions() == QFile::ExeUser*/);
}

bool RenderProcess::isActive()
{
    return (this->status == STARTED || this->status == RUNNING); 
}
bool RenderProcess::isOutputAvailable()
{
    if(this->status != RUNNING || this->status != FINISHED)
        return false;

    // Load the Input File
    QFile inputFile(this->inputPath);
    if(inputFile.open(QIODevice::ReadOnly))
      return true;
    else
      return false;
}

void RenderProcess::addArguments(const QString &arg)
{
  this->args.push_back(arg);
}

void RenderProcess::setArguments(const QStringList &Args) {
  this->args = Args;
}

void RenderProcess::setInputPath(const QString &input) {
  this->inputPath = input;
}

void RenderProcess::begin()
{
    //Check path names are valid
    if(isExecPathValid())
    {
        this->status = VALID;
        this->start(execPath, args);
        std::string inputFilestr = this->inputPath.toStdString();
        bool test = true;
        std::string str = this->errorString().toStdString();
        test = false;
    }
}

void RenderProcess::stop()
{
  this->terminate();  
}