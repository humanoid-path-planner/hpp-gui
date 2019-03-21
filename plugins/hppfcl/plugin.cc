// Copyright (c) 2019 CNRS
// Authors: Joseph Mirabel
//
//
// This file is part of hpp-gui
// hpp-gui is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-gui is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-gui  If not, see
// <http://www.gnu.org/licenses/>.

#include <plugin.hh>

#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>

#include <gepetto/gui/mainwindow.hh>
#include <gepetto/gui/windows-manager.hh>

#include <node.hh>

namespace hpp {
  namespace gui {
    using gepetto::gui::MainWindow;

    void HppFclPlugin::init()
    {
      MainWindow* main = MainWindow::instance ();
      main->registerSlot("addBV", this);

      // TODO add a way to add an action to body tree items.
      QToolBar* toolBar = MainWindow::instance()->addToolBar("hpp-fcl tools");
      toolBar->setObjectName ("hppfclplugin.toolbar");
      QAction* openD = new QAction (QIcon::fromTheme("document-open"), "Load a BVH model", toolBar);
      toolBar->addAction (openD);
      connect (openD, SIGNAL(triggered()), SLOT (openDialog()));
    }

    void HppFclPlugin::addBV (QString name, QString filename, int splitMethod) const
    {
      std::string _name (name.toStdString());

      BVHDisplayPtr_t node (new BVHDisplay (filename.toStdString(), _name));
      switch (splitMethod) {
        default:
        case 0:
          node->init (hpp::fcl::SPLIT_METHOD_MEAN);
          break;
        case 1:
          node->init (hpp::fcl::SPLIT_METHOD_MEDIAN);
          break;
        case 2:
          node->init (hpp::fcl::SPLIT_METHOD_BV_CENTER);
          break;
      }
      MainWindow* main = MainWindow::instance ();
      main->osg()->insertNode (_name, node);
    }

    void HppFclPlugin::openDialog() const
    {
      QString filename = QFileDialog::getOpenFileName (NULL, "Select a mesh file");
      QString name = QInputDialog::getText(NULL, "Node name", "Node name", QLineEdit::Normal, "bvhmodel");
      int splitMethod = QInputDialog::getInt(NULL, "Split method type",
          "Split method type", 0, 0, 3, 1);
      addBV (filename, name, splitMethod);
    }

#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
    Q_EXPORT_PLUGIN2 (hppfclplugin, HppFclPlugin)
#endif

  } // namespace gui
} // namespace hpp
