/***************************************************************************
 *   Copyright (c) 2009                                                    *
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

#include "SelectionLineEdit.h"

#include <Gui/BitmapFactory.h>
#include <QToolButton>
#include <QEvent>
#include <QKeyEvent>
#include <QStyle>

using namespace PartDesignGui;


SelectionLineEdit::SelectionLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    clearButton = new QToolButton(this);
    QPixmap pixmap = Gui::BitmapFactory().pixmap("delete.png");
    clearButton->setIcon(QIcon(pixmap));
    QSize size(16,16);
    clearButton->setIconSize(size);
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    clearButton->hide();
 
    QObject::connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    QObject::connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(clearButton->sizeHint().width() + frameWidth + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));
}

SelectionLineEdit::~SelectionLineEdit() {
    delete clearButton;
}

bool SelectionLineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
          QKeyEvent *ke = static_cast<QKeyEvent *>(event);
          if (ke->key() == Qt::Key_Delete) {
            // Clear the Line Edit and Colour
            this->clear();
          }
    }
    return QWidget::event(event);
 }

void SelectionLineEdit::updateStatus(const EditMode &mode){
  QColor colour;
    switch(mode) {
      case STATUS_SELECT:
        colour = Qt::yellow;
        break;
      case STATUS_VALID:
        colour = Qt::green;
        break;
      case STATUS_INVALID:
        colour = Qt::red;
        break;
      case STATUS_EMPTY:
        colour = Qt::white;
        break;
      default:
        return;
    }
    QPalette p = this->palette();
    p.setColor(QPalette::Normal, QPalette::Base, colour);
    this->setPalette(p);
}
void SelectionLineEdit::resizeEvent(QResizeEvent *)
{
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - sz.width(),
                      (rect().bottom() + 1 - sz.height())/2);
}

void SelectionLineEdit::updateCloseButton(const QString& text)
{
    clearButton->setVisible(!text.isEmpty());
}
