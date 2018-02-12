#ifndef HPP_GUI_PATHPLAYER_HH
#define HPP_GUI_PATHPLAYER_HH

#include <QWidget>

#include <gepetto/gui/fwd.hh>

class QDoubleSpinBox;
class QProcess;
class QPushButton;
class QSpinBox;
class QSlider;
class QTextBrowser;

namespace Ui {
  class PathPlayerWidget;
}

namespace hpp {
  namespace gui {
    class HppWidgetsPlugin;

    class PathPlayer : public QWidget
    {
      Q_OBJECT

      public:
        PathPlayer (HppWidgetsPlugin* plugin, QWidget* parent = 0);
        ~PathPlayer();

        public slots:
        /// Returns the path currently selected.
        int getCurrentPath() const;

        /// Display the waypoints of a joint in the viewer.
        /// The waypoints are those of the currently selected path index.
        /// \param jointName name of the joint
          void displayWaypointsOfPath (const std::string jointName);

        /// Prepare the display of the path of a joint in the viewer.
        /// The path is took from the currently selected path index.
        /// \param jointName name of the joint
        void displayPath (const std::string jointName);

        /// Get the number of paths in hpp and refresh the gui accordingly.
        void update ();

        /// Tells the path player to set the robot current velocity.
        void setRobotVelocity(bool set);

        /// Distance between two sampling point
        double lengthBetweenRefresh () const;

        /// Set the slider position
        void setCurrentTime (const double& param);

      protected:
        /// Display the path of the jointName in the viewer.
        /// The path is took from the currently selected path index.
        /// \param jointName name of the joint
        virtual void displayPath_impl (const std::string jointName);
signals:
        void displayPath_status (int progress);
        void appliedConfigAtParam (int pid, double param);

        private slots:
          void pathIndexChanged (int i);
        void pathSliderChanged (int value);
        //  void timeChanged (double d);
        void playPauseToggled (bool toggled);
        void stopClicked ();
        void recordToggled (bool toggled);
        void pathPulse ();
        void timerEvent(QTimerEvent *event);
        void readyReadProcessOutput ();

      private:
        void initSearchActions();
        void updateConfiguration ();
        double sliderToLength (int v) const;
        int lengthToSlider (double l) const;
        double timeToLength (double time) const;
        int timeBetweenRefresh() const;

        ::Ui::PathPlayerWidget* ui_;

        QDoubleSpinBox* timeSpinBox () const;
        QSpinBox* pathIndex() const;
        QSlider* pathSlider () const;
        QPushButton* playPause () const;
        QPushButton* stop () const;
        QPushButton* record () const;

        double pathLength_;
        double currentParam_;
        int timerId_;
        bool velocity_;

        QProcess* process_;
        QDialog* showPOutput_;
        QTextBrowser* pOutput_;

        HppWidgetsPlugin* plugin_;
    };
  } // namespace gui
} // namespace hpp

#endif // HPP_GUI_PATHPLAYER_HH
