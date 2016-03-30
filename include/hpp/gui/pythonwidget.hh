#ifndef HPP_GUI_PYTHONWIDGET_HH
#define HPP_GUI_PYTHONWIDGET_HH

#include <hpp/gui/config-dep.hh>

#if ! HPP_GUI_HAS_PYTHONQT
# error "hpp-gui was not compile with PythonQt dependency."
#endif

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
      virtual ~PythonWidget();

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

#endif // HPP_GUI_PYTHONWIDGET_HH
