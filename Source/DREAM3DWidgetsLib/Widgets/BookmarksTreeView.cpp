/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


#include "DREAM3DWidgetsLib/Widgets/BookmarksTreeView.h"
#include "DREAM3DWidgetsLib/Widgets/BookmarksItemDelegate.h"

#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtCore/QMimeData>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QDrag>

#include <iostream>


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BookmarksTreeView::BookmarksTreeView(QWidget* parent) :
QTreeView(parent),
m_IndexBeingDragged(QModelIndex()),
m_TopLevelItemPlaceholder(QModelIndex())
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this,
    SIGNAL(customContextMenuRequested(const QPoint&)),
    SLOT(onCustomContextMenuRequested(const QPoint&)));

  connect(this, SIGNAL(collapsed(const QModelIndex&)), SLOT(collapseIndex(const QModelIndex&)));
  connect(this, SIGNAL(expanded(const QModelIndex&)), SLOT(expandIndex(const QModelIndex&)));

  BookmarksItemDelegate* dlg = new BookmarksItemDelegate(this);
  setItemDelegate(dlg);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BookmarksTreeView::~BookmarksTreeView()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::addActionList(QList<QAction*> actionList)
{
  for (int i = 0; i < actionList.size(); i++)
  {
    m_Menu.addAction(actionList[i]);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::setNodeActionList(QList<QAction*> list)
{
  m_NodeActions = list;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::setLeafActionList(QList<QAction*> list)
{
  m_LeafActions = list;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::setLeafErrorActionList(QList<QAction*> list)
{
  m_LeafErrorActions = list;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::setDefaultActionList(QList<QAction*> list)
{
  m_DefaultActions = list;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::onCustomContextMenuRequested(const QPoint& pos)
{
  QModelIndex index = indexAt(pos);

  if (index.isValid())
  {
    // Note: We must map the point to global from the viewport to
    // account for the header.
    showContextMenu(index, viewport()->mapToGlobal(pos));
  }
  else
  {
    showContextMenu(QModelIndex(), mapToGlobal(pos));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    m_StartPos = event->pos();

    QModelIndex index = indexAt(event->pos());

    if (index.isValid() == false)
    {
      // Deselect the current item
      clearSelection();
      clearFocus();
      setCurrentIndex(QModelIndex());
    }
  }
  QTreeView::mousePressEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::showContextMenu(QModelIndex index, const QPoint& globalPos)
{
  BookmarksModel* model = BookmarksModel::Instance();

  // Clear menu's previous actions
  m_Menu.clear();

  if (index.isValid() == false)
  {
    if (!m_LeafActions.isEmpty())
    {
      addActionList(m_DefaultActions);
    }
  }
  else
  {
    QModelIndex actualIndex = model->index(index.row(), Path, index.parent());
    QString path = actualIndex.data().toString();
    if (path.isEmpty() == false)
    {
      if (model->data(actualIndex, Qt::BackgroundRole) != QColor(Qt::white) && !m_LeafErrorActions.isEmpty())
      {
        addActionList(m_LeafErrorActions);
      }
      else if (!m_LeafActions.isEmpty())
      {
        addActionList(m_LeafActions);
      }
    }
    else if (path.isEmpty() && !m_NodeActions.isEmpty())
    {
      addActionList(m_NodeActions);
    }
  }

  m_Menu.exec(globalPos);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::mouseMoveEvent(QMouseEvent* event)
{
  BookmarksModel* model = BookmarksModel::Instance();

  if (event->buttons() & Qt::LeftButton)
  {
    QModelIndex index = model->index(currentIndex().row(), Name, currentIndex().parent());
    QColor bgColor = model->data(index, Qt::BackgroundColorRole).value<QColor>();
    int distance = (event->pos() - m_StartPos).manhattanLength();
    if (distance >= QApplication::startDragDistance() && bgColor == QColor(Qt::white))
    {
      performDrag();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::performDrag()
{
  BookmarksModel* model = BookmarksModel::Instance();

  m_IndexBeingDragged = currentIndex();
  if (m_IndexBeingDragged.isValid())
  {
    QMimeData* mimeData = new QMimeData;
    QString path = model->index(m_IndexBeingDragged.row(), Path, m_IndexBeingDragged.parent()).data().toString();
    QString source = "Bookmarks";
    mimeData->setData("Source", source.toLatin1());
    mimeData->setText(path);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::dragEnterEvent(QDragEnterEvent* event)
{
  BookmarksTreeView* source =
    qobject_cast<BookmarksTreeView*>(event->source());
  if (source && source != this)
  {
    event->setDropAction(Qt::MoveAction);
    event->accept();
  }
  else
  {
    event->acceptProposedAction();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
  BookmarksModel* model = BookmarksModel::Instance();

  if (m_TopLevelItemPlaceholder.isValid())
  {
    model->removeRow(model->rowCount() - 1, rootIndex());
    m_TopLevelItemPlaceholder = QModelIndex();
  }

  clearSelection();

  setCurrentIndex(m_IndexBeingDragged);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::dragMoveEvent(QDragMoveEvent* event)
{
  BookmarksModel* model = BookmarksModel::Instance();
  int topLevelPHPos = model->rowCount();

  QModelIndex index = indexAt(event->pos());
  if (index.isValid() == false || index.row() == m_TopLevelItemPlaceholder.row())
  {
    if (m_TopLevelItemPlaceholder.isValid() == false)
    {
      clearSelection();
      blockSignals(true);
      model->insertRow(topLevelPHPos, rootIndex());
      m_TopLevelItemPlaceholder = model->index(topLevelPHPos, 0, rootIndex());
      model->setData(m_TopLevelItemPlaceholder, topLevelString, Qt::DisplayRole);
      setCurrentIndex(m_TopLevelItemPlaceholder);
      blockSignals(false);
    }
  }
  else if (index.isValid() && index.row() != m_TopLevelItemPlaceholder.row())
  {
    if (m_TopLevelItemPlaceholder.isValid())
    {
      model->removeRow(topLevelPHPos-1, rootIndex());
      m_TopLevelItemPlaceholder = QModelIndex();
    }
    clearSelection();

    if (model->flags(index).testFlag(Qt::ItemIsDropEnabled) == true)
    {
      //setExpanded(index, true);
      setCurrentIndex(index);
    }
  }

  BookmarksTreeView* source =
    qobject_cast<BookmarksTreeView*>(event->source());
  if (source && source != this)
  {
    event->setDropAction(Qt::MoveAction);
    event->accept();
  }
  else
  {
    event->acceptProposedAction();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::dropEvent(QDropEvent* event)
{
  BookmarksModel* model = BookmarksModel::Instance();

  const QMimeData* mimedata = event->mimeData();
  if (mimedata->hasFormat("Source"))
  {
    QByteArray byteArray = mimedata->data("Source");
    QString source = QString::fromStdString(byteArray.toStdString());
    if (source == "Bookmarks")
    {
      QModelIndexList selectList = selectedIndexes();
      if (selectList.size() > 0)
      {
        QModelIndex newParent = selectList[0];

        if (model->flags(newParent).testFlag(Qt::ItemIsDropEnabled) == true && newParent != m_IndexBeingDragged)
        {
          QModelIndex oldParent = m_IndexBeingDragged.parent();

          if (m_TopLevelItemPlaceholder.isValid())
          {
            // If the parent is the placeholder, change the parent to the root.
            if (m_TopLevelItemPlaceholder == newParent)
            {
              newParent = QModelIndex();
            }

            model->removeRow(model->rowCount() - 1, rootIndex());
            m_TopLevelItemPlaceholder = QModelIndex();
          }

          if (m_IndexBeingDragged.isValid())
          {
            model->moveIndexInternally(m_IndexBeingDragged, oldParent, newParent);
            expand(newParent);
            model->sort(Name, Qt::AscendingOrder);
          }
        }
      }
    }
  }
  else
  {
    QUrl url(mimedata->text());
    QString path = url.toLocalFile();
    path = QDir::toNativeSeparators(path);
    QModelIndex parentIndex = currentIndex();
    if (m_TopLevelItemPlaceholder.isValid())
    {
      if (parentIndex == m_TopLevelItemPlaceholder)
      {
        parentIndex = rootIndex();
      }

      model->removeRow(model->rowCount() - 1, rootIndex());
      m_TopLevelItemPlaceholder = QModelIndex();
    }

    model->addFileToTree(path, parentIndex);
    expand(parentIndex);
    model->sort(Name, Qt::AscendingOrder);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::setModel(QAbstractItemModel* model)
{
  // Set the model
  QTreeView::setModel(model);

  BookmarksModel* bModel = qobject_cast<BookmarksModel*>(model);

  // Expand indexes that have their expand member set to true
  expandChildren(QModelIndex(), bModel);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::expandChildren(const QModelIndex &parent, BookmarksModel* model)
{
  for (int row = 0; row < model->rowCount(parent); row++)
  {
    QModelIndex index = model->index(row, 0, parent);
    if (model->needsToBeExpanded(index))
    {
      expand(index);
    }

    // Recursive call, to expand all items in the tree
    expandChildren(index, model);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::collapseIndex(const QModelIndex & index)
{
  if (index.isValid())
  {
    BookmarksModel* model = BookmarksModel::Instance();
    QModelIndex sibling = model->sibling(index);

    QTreeView::collapse(index);
    model->setNeedsToBeExpanded(index, false);
    model->setNeedsToBeExpanded(sibling, false);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::expandIndex(const QModelIndex & index)
{
  BookmarksModel* model = BookmarksModel::Instance();
  QModelIndex sibling = model->sibling(index);

  QTreeView::expand(index);
  model->setNeedsToBeExpanded(index, true);
  model->setNeedsToBeExpanded(sibling, true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
  emit currentIndexChanged(current, previous);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QJsonObject BookmarksTreeView::toJsonObject()
{
  BookmarksModel* model = BookmarksModel::Instance();
  BookmarksItem* rootItem = model->getRootItem();

  QJsonObject treeObj;
  for (int i = 0; i < rootItem->childCount(); i++)
  {
    QModelIndex childIndex = model->index(i, Name, QModelIndex());

    QJsonObject childObj = wrapModel(childIndex);
    treeObj["Child " + QString::number(i + 1)] = childObj;
  }

  return treeObj;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QJsonObject BookmarksTreeView::wrapModel(QModelIndex currentIndex)
{
  BookmarksModel* model = BookmarksModel::Instance();

  QJsonObject obj;

  for (int i = 0; i < model->rowCount(currentIndex); i++)
  {
    QModelIndex childIndex = model->index(i, 0, currentIndex);

    QJsonObject childObj = wrapModel(childIndex);
    obj["Child " + QString::number(i + 1)] = childObj;
  }

  QString name = model->index(currentIndex.row(), Name, model->parent(currentIndex)).data().toString();
  QString path = model->index(currentIndex.row(), Path, model->parent(currentIndex)).data().toString();

  obj.insert("Name", name);

  if (model->flags(currentIndex).testFlag(Qt::ItemIsDropEnabled) == true)
  {
    obj.insert("Expanded", isExpanded(currentIndex));
  }
  else
  {
    obj.insert("Path", path);
  }

  return obj;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BookmarksModel* BookmarksTreeView::FromJsonObject(QJsonObject treeObject)
{
  BookmarksModel* model = BookmarksModel::Instance();

  for (QJsonObject::iterator iter = treeObject.begin(); iter != treeObject.end(); ++iter)
  {
    QJsonValue val = *iter;
    if (val.isObject())
    {
      BookmarksTreeView::UnwrapModel(val.toObject(), model, QModelIndex());
    }
  }

  QStringList paths = model->getFilePaths();
  model->setFileSystemWatcher(new QFileSystemWatcher(paths, model));
  connect(model->getFileSystemWatcher(), SIGNAL(fileChanged(const QString &)), model, SLOT(updateRowState(const QString &)));

  return model;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void BookmarksTreeView::UnwrapModel(QJsonObject object, BookmarksModel* model, QModelIndex parentIndex)
{
  int row = model->rowCount(parentIndex);
  bool value = model->insertRow(row, parentIndex);
  QModelIndex nameIndex = model->index(row, Name, parentIndex);
  QModelIndex pathIndex = model->index(row, Path, parentIndex);

  QString name = object["Name"].toString();
  QString path = object["Path"].toString();
  bool expanded = object["Expanded"].toBool();

  QFileInfo fi(path);
  if (path.isEmpty() == false && fi.exists() == false)
  {
    model->setRowState(nameIndex, Error);
  }

  path = QDir::toNativeSeparators(path);

  model->setData(nameIndex, name, Qt::DisplayRole);
  model->setData(pathIndex, path, Qt::DisplayRole);
  model->setNeedsToBeExpanded(nameIndex, expanded);
  model->setNeedsToBeExpanded(pathIndex, expanded);

  for (QJsonObject::iterator iter = object.begin(); iter != object.end(); ++iter)
  {
    QJsonValue val = *iter;
    if (val.isObject())
    {
      BookmarksTreeView::UnwrapModel(val.toObject(), model, nameIndex);
    }
  }
}





