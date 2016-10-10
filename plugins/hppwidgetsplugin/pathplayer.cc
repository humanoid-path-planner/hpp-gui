#include "hppwidgetsplugin/pathplayer.hh"

#include <hpp/corbaserver/common.hh>
#include <hpp/corbaserver/client.hh>

#include "gepetto/gui/mainwindow.hh"
#include <gepetto/gui/windows-manager.hh>
#include <gepetto/gui/osgwidget.hh>

#include "hppwidgetsplugin/ui_pathplayerwidget.h"

namespace hpp {
  namespace gui {
    PathPlayer::PathPlayer (HppWidgetsPlugin *plugin, QWidget *parent) :
      QWidget (parent)
      , ui_ (new ::Ui::PathPlayerWidget)
      , frameRate_ (25)
      , timerId_ (0)
      , process_ (new QProcess (this))
      , showPOutput_ (new QDialog (this, Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint))
      , pOutput_ (new QTextBrowser())
      , plugin_ (plugin)
    {
      ui_->setupUi (this);
      pathIndex()->setMaximum(0);
      connect (pathSlider(), SIGNAL (sliderMoved (int)), this, SLOT (pathSliderChanged(int)));
      connect (pathIndex(), SIGNAL (valueChanged(int)), this, SLOT (pathIndexChanged(int)));
      connect (playPause(), SIGNAL (toggled (bool)), this, SLOT (playPauseToggled(bool)));
      connect (stop(), SIGNAL (clicked()), this, SLOT (stopClicked()));
      connect (record(), SIGNAL (toggled(bool)), this, SLOT (recordToggled(bool)));
      connect (ui_->refreshButton_path, SIGNAL (clicked()), this, SLOT (update()));

      process_->setProcessChannelMode(QProcess::MergedChannels);
      connect (process_, SIGNAL (readyReadStandardOutput ()), SLOT (readyReadProcessOutput()));

      showPOutput_->setModal(false);
      showPOutput_->setLayout(new QHBoxLayout ());
      showPOutput_->layout()->addWidget(pOutput_);
    }

    PathPlayer::~PathPlayer()
    {
      if (process_ != NULL) {
        if (process_->state() == QProcess::Running)
          process_->kill();
        delete process_;
      }
      delete ui_;
    }

    void PathPlayer::displayWaypointsOfPath(const std::string jointName)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      if (!pathIndex()->isEnabled()) {
        main->logError("There is no path. Did you solve a problem ?");
        return;
      }
      int pid = pathIndex()->value();
      std::stringstream ss; ss << "path" << pid << "_" << jointName;
      std::string pn = ss.str();
      float colorN[] = {0.f, 0.f, 1.f, 1.f};
      float colorE[] = {1.f, 0.f, 0.f, 1.f};
      gepetto::gui::WindowsManagerPtr_t wsm = main->osg();
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      hpp::floatSeqSeq_var waypoints = hpp->problem()->getWaypoints((CORBA::UShort)pid);
      wsm->createScene (pn.c_str());
      hpp::floatSeq_var curCfg = hpp->robot()->getCurrentConfig();
      for (unsigned int i = 0; i < waypoints->length(); ++i) {
        float pos[7];
        float pos1[3], pos2[3];
        hpp->robot()->setCurrentConfig(waypoints[i]);
        hpp::Transform__var t = hpp->robot()->getLinkPosition(jointName.c_str());
        for (int j = 0; j < 7; ++j) { pos[j] = (float)t.in()[j]; }
        for (int j = 0; j < 3; ++j) { pos1[j] = pos2[j]; }
        for (int j = 0; j < 3; ++j) { pos2[j] = (float)t.in()[j]; }
        QString xyzName = QString::fromStdString(pn).append("/node%1").arg (i);
        wsm->addXYZaxis(xyzName.toLocal8Bit().data(), colorN, 0.01f, 1.f);
        wsm->applyConfiguration(xyzName.toLocal8Bit().data(), pos);
        if  (i > 0) {
          QString lineName = QString::fromStdString(pn).append("/edge%1").arg (i);
          wsm->addLine(lineName.toLocal8Bit().data(), pos1, pos2, colorE);
        }
      }
      hpp->robot()->setCurrentConfig(curCfg.in());
      wsm->addToGroup(pn.c_str(), "hpp-gui");
      wsm->refresh();
    }

    void PathPlayer::displayPath(const std::string jointName)
    {
      QFutureWatcher <void>* fw = new QFutureWatcher<void>(this);
      QProgressDialog* pd = new QProgressDialog ("Computing curved path. Please wait...", "Cancel", 0, 100, this);
      connect(this, SIGNAL(displayPath_status(int)), pd, SLOT (setValue(int)));
      pd->setCancelButton(0);
      pd->setRange(0, 100);
      pd->show();
      fw->setFuture(QtConcurrent::run (this, &PathPlayer::displayPath_impl, jointName));
      connect (fw, SIGNAL (finished()), pd, SLOT (deleteLater()));
      connect (fw, SIGNAL (finished()), fw, SLOT (deleteLater()));
    }

    void PathPlayer::displayPath_impl(const std::string jointName)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      if (!pathIndex()->isEnabled()) {
        main->logError("There is no path. Did you solve a problem ?");
        emit displayPath_status(100);
        return;
      }
      CORBA::UShort pid = (CORBA::UShort) pathIndex()->value();
      std::stringstream ss; ss << "curvedpath_" << pid << "_" << jointName;
      std::string pn = ss.str();
      float colorE[] = {1.f, 0.f, 0.f, 1.f};
      gepetto::gui::WindowsManagerPtr_t wsm = main->osg();
      HppWidgetsPlugin::HppClient* hpp = plugin_->client();
      hpp::floatSeq_var curCfg = hpp->robot()->getCurrentConfig();
      CORBA::Double length = hpp->problem()->pathLength(pid);
      double dt = lengthBetweenRefresh();
      std::size_t nbPos = (std::size_t)(length / dt) + 1;
      gepetto::corbaserver::PositionSeq_var posSeq = new gepetto::corbaserver::PositionSeq (nbPos);
      posSeq->length(nbPos);
      float statusStep = 100.f / (float) nbPos;
      emit displayPath_status(0);
      for (std::size_t i = 0; i < nbPos; ++i) {
        double t = std::min (i * dt, length);
        hpp::floatSeq_var q = hpp->problem()->configAtParam(pid, t);
        hpp->robot()->setCurrentConfig(q);
        hpp::Transform__var transform = hpp->robot()->getLinkPosition(jointName.c_str());
        for (int j = 0; j < 3; ++j) { posSeq[i][j] = (float)transform.in()[j]; }
        emit displayPath_status(qFloor ((float)i * statusStep));
      }
      wsm->addCurve(pn.c_str(), posSeq.in(), colorE);
      hpp->robot()->setCurrentConfig(curCfg.in());
      wsm->addToGroup(pn.c_str(), "hpp-gui");
      wsm->refresh();
      emit displayPath_status (100);
    }

    void PathPlayer::update ()
    {
      CORBA::Short nbPath = plugin_->client()->problem ()->numberPaths ();
      if (nbPath > 0)
      {
        pathIndex()->setEnabled(true);
        pathSlider()->setEnabled(true);
        playPause()->setEnabled(true);
        stop()->setEnabled(true);
        if (pathIndex()->maximum() == 0) {
          // If path index value is 0, no signal valueChanged will
          // be emitted. Force a value changed.
          if (pathIndex()->value() == 0)
            pathIndexChanged(0);
        }
        pathIndex()->setMaximum(nbPath - 1);
	pathIndex()->setValue(nbPath - 1);
      }
      else {
        pathIndex()->setEnabled(false);
        pathSlider()->setEnabled(false);
        playPause()->setEnabled(false);
        stop()->setEnabled(false);
      }
    }

    void PathPlayer::pathIndexChanged(int i)
    {
      assert (i >= 0);
      pathLength_ = plugin_->client()->problem()->pathLength ((short unsigned int)i);
      currentParam_= 0;
    }

    void PathPlayer::pathSliderChanged(int value)
    {
      // The user moved the slider manually.
      currentParam_ = sliderToLength(value);
      updateConfiguration();
    }

    void PathPlayer::playPauseToggled(bool toggled)
    {
      if (toggled) {
        if (timerId_ == 0) timerId_ = startTimer(timeBetweenRefresh());
      } else {
        killTimer(timerId_);
        timerId_ = 0;
      }
    }

    void PathPlayer::stopClicked()
    {
      // Pause
      playPause()->setChecked(false);
      // Reset current position.
      currentParam_ = 0;
      pathSlider()->setSliderPosition(0);
      updateConfiguration();
    }

    void PathPlayer::recordToggled(bool toggled)
    {
      gepetto::gui::MainWindow* main = gepetto::gui::MainWindow::instance();
      if (toggled) {
        QDir tmp ("/tmp");
        tmp.mkpath ("hpp-gui/record"); tmp.cd("hpp-gui/record");
        foreach (QString f, tmp.entryList(QStringList() << "img_0_*.jpeg", QDir::Files))
          tmp.remove(f);
        QString path = tmp.absoluteFilePath("img");
        QString ext = "jpeg";
        main->osg ()->startCapture(
            main->centralWidget()->windowID(),
            path.toLocal8Bit().data(),
            ext.toLocal8Bit().data());
        main->log("Saving images to " + path + "_*." + ext);
      } else {
        main->osg()->stopCapture(main->centralWidget()->windowID());
        QString outputFile = QFileDialog::getSaveFileName(this, tr("Save video to"), "untitled.mp4");
        if (!outputFile.isNull()) {
          if (QFile::exists(outputFile))
            QFile::remove(outputFile);
          QString avconv = "avconv";

          QStringList args;
          QString input = "/tmp/hpp-gui/record/img_0_%d.jpeg";
          args << "-r" << "50"
            << "-i" << input
            << "-vf" << "scale=trunc(iw/2)*2:trunc(ih/2)*2"
            << "-r" << "25"
            << "-vcodec" << "libx264"
            << outputFile;
          qDebug () << args;

          showPOutput_->setWindowTitle(avconv + " " + args.join(" "));
          pOutput_->clear();
          showPOutput_->resize(main->size() / 2);
          showPOutput_->show();
          process_->start(avconv, args);
        }
      }
    }

    void PathPlayer::pathPulse()
    {
      if (playPause()->isChecked()) {
        // Stil playing
        currentParam_ += lengthBetweenRefresh();
        if (currentParam_ > pathLength_) {
          currentParam_ = pathLength_;
          playPause()->setChecked(false);
        }
        pathSlider()->setSliderPosition(lengthToSlider(currentParam_));
        updateConfiguration();
      }
    }

    void PathPlayer::timerEvent(QTimerEvent *event)
    {
      if (timerId_ == event->timerId()) {
        pathPulse();
      }
    }

    void PathPlayer::readyReadProcessOutput()
    {
      pOutput_->append(process_->readAll());
    }

    void PathPlayer::updateConfiguration ()
    {
      hpp::floatSeq_var config =
        plugin_->client()->problem()->configAtParam ((short unsigned int)pathIndex()->value(),currentParam_);
      plugin_->client()->robot()->setCurrentConfig (config.in());
      gepetto::gui::MainWindow::instance()->requestApplyCurrentConfiguration();
    }

    inline double PathPlayer::sliderToLength(int v) const
    {
      return ((double)v / (double)pathSlider()->maximum()) * pathLength_;
    }

    int PathPlayer::lengthToSlider(double l) const
    {
      return (int) (pathSlider()->maximum() * l / pathLength_);
    }

    inline double PathPlayer::timeToLength(double time) const
    {
      return time * pathLength_ / timeSpinBox()->value();
    }

    inline int PathPlayer::timeBetweenRefresh() const
    {
      return 1000/frameRate_;
    }

    double PathPlayer::lengthBetweenRefresh() const
    {
      return pathLength_ / (timeSpinBox()->value() * frameRate_);
    }

    QDoubleSpinBox *PathPlayer::timeSpinBox() const
    {
      return ui_->timeSpinBox;
    }

    QSpinBox *PathPlayer::pathIndex() const
    {
      return ui_->pathIndexSpin;
    }

    QSlider *PathPlayer::pathSlider() const
    {
      return ui_->pathSlider;
    }

    QPushButton *PathPlayer::playPause() const
    {
      return ui_->playPauseButton;
    }

    QPushButton *PathPlayer::stop() const
    {
      return ui_->stopButton;
    }

    QPushButton *PathPlayer::record() const
    {
      return ui_->recordButton;
    }

    int	PathPlayer::getCurrentPath() const
    {
      return pathIndex()->value();
    }
  } // namespace gui
} // namespace hpp
