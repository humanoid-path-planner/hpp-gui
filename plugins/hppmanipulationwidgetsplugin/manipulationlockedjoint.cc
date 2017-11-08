#include "hppmanipulationwidgetsplugin.hh"
#include "manipulationlockedjoint.hh"

namespace hpp {
  namespace gui {
    ManipulationLockedJoint::ManipulationLockedJoint(HppManipulationWidgetsPlugin* plugin)
      : ListJointConstraint(plugin)
    {
    }

    ManipulationLockedJoint::~ManipulationLockedJoint()
    {
    }

    QString ManipulationLockedJoint::getName() const
    {
      return "Locked Joint";
    }

    void ManipulationLockedJoint::operator ()(QString const& /*name*/) {
      QList<QListWidgetItem*> selected = jointList_->selectedItems();
      HppManipulationWidgetsPlugin* plugin = dynamic_cast<HppManipulationWidgetsPlugin*>(plugin_);

      foreach (QListWidgetItem *item, selected) {
        std::string jointName = item->text().toStdString().c_str();
        hpp::floatSeq_var config = plugin->client()->robot()->getJointConfig(jointName.c_str());
        plugin->client()->problem()->createLockedJoint(std::string("lock_" + jointName).c_str(),
                                                            jointName.c_str(),
                                                            config.in());
        emit constraintCreated(std::string("lock_" + jointName).c_str());
      }
    }
  }
}
