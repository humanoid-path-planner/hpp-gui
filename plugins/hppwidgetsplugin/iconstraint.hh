#ifndef HPP_GUI_ICONSTRAINT_HH
#define HPP_GUI_ICONSTRAINT_HH

#include <QObject>

namespace hpp {
  namespace gui {
    class IConstraint : public QObject
    {
        Q_OBJECT
    signals:
      void constraintCreated(QString name);
      void finished();

    public:
      virtual ~IConstraint() {}
      virtual QString getName() const = 0;
      virtual QWidget* getWidget() const = 0;
      virtual void reload() = 0;
      virtual void operator()(QString const& name) = 0;
    };
  } // namespace hpp
} // namespace gui

#endif // HPP_GUI_ICONSTRAINT_HH
