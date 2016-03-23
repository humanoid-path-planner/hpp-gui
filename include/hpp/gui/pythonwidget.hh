#include "hpp/gui/config-python.hh"
#ifndef HPP_GUI_PYTHONWIDGET_H
#define HPP_GUI_PYTHONWIDGET_H

#if PYTHONQT_NEED_INCLUDE==1
#include <QDockWidget>
#include <QLayout>
#include <QPushButton>
#include <PythonQt/PythonQt.h>
#include <PythonQt/PythonQt_QtAll.h>
#include <PythonQt/gui/PythonQtScriptingConsole.h>

namespace hpp {
  namespace gui {
    class PythonWidget : public QDockWidget
    {
      Q_OBJECT
    public:
      explicit PythonWidget(QWidget *parent = 0);
      void addToContext(QString const& name, QObject *obj);

    private:
      PythonQtObjectPtr mainContext_;
      PythonQtScriptingConsole* console_;
      QPushButton* button_;

    signals:

    public slots:
      void browseFile();
    };
  } // namespace gui
} // namespace hpp

#endif // PYTHONQT_NEED_INSTALL
#endif // PYTHONWIDGET_H
