#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QList>

#include <hpp/core/fwd.hh>

#include <hpp/gui/fwd.h>

#include <hpp/gui/osgwidget.h>
#include <hpp/gui/ledindicator.h>

#include <hpp/gui/omniorb/omniorbthread.h>
#include <hpp/gui/dialog/dialogloadrobot.h>
#include <hpp/gui/dialog/dialogloadenvironment.h>
#include <hpp/gui/attitude-device.h>


namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  struct JointElement {
    std::string name;
    std::string bodyName;
    JointTreeItem* item;
    bool updateViewer;

    JointElement ()
      : name (), bodyName (), item (NULL), updateViewer (false) {}
    JointElement (std::string n, std::string bn, JointTreeItem* i, bool updateV = true)
      : name (n), bodyName (bn), item (i), updateViewer (updateV) {}
  };
  typedef QMap <std::string, JointElement> JointMap;

  explicit MainWindow(QWidget *parent = 0, bool startHppServer = true);
  ~MainWindow();

  static MainWindow* instance ();

  hpp::corbaServer::Client* hppClient ();

  BackgroundQueue &worker();

  SolverWidget* solver() const;

  PathPlayer* pathPlayer() const;

  WindowsManagerPtr_t osg () const;

  OSGWidget* centralWidget() const;

  const JointMap& jointMap () const {
    return jointsMap_;
  }

  void log (const QString& text);
  void logError (const QString& text);

  void emitSendToBackground (WorkItem* item);

signals:
  void sendToBackground (WorkItem* item);
  void createView (QString name);

public slots:
  void logJobStarted (int id, const QString& text);
  void logJobDone    (int id, const QString& text);
  void logJobFailed  (int id, const QString& text);

  OSGWidget* delayedCreateView (QString name = "");
  void reload ();
  void addBodyToTree (graphics::GroupNodePtr_t group);
  void addJointToTree (const std::string name, JointTreeItem *parent);
  void updateRobotJoints (const QString robotName);
  void applyCurrentConfiguration ();
  void requestConfigurationValidation ();
  void selectJointFromBodyName (const std::string& bodyName);
  void selectJoint (const std::string& jointName);
  void onOpenPluginManager ();

private slots:
  OSGWidget* onCreateView();
  void openLoadRobotDialog ();
  void openLoadEnvironmentDialog ();
  void updateBodyTree (const QModelIndex& index);
  void updateJointTree (const QModelIndex& index);
  void showTreeContextMenu (const QPoint& point);

  void handleWorkerDone (int id);

private:
  CorbaServer &hppServer();

  void setupInterface ();
  void resetJointTree ();
  void createCentralWidget ();
  void computeObjectPosition ();
  void readSettings ();
  void writeSettings ();

  static MainWindow* instance_;

  Ui::MainWindow* ui_;
  OSGWidget* centralWidget_;
  QList <OSGWidget*> osgWindows_;

  hpp::core::ProblemSolverPtr_t problemSolver_;
  WindowsManagerPtr_t osgViewerManagers_;
  CorbaServer hppServer_, osgServer_;
  hpp::corbaServer::Client* hppClient_;
  BackgroundQueue backgroundQueue_;
  QThread worker_;
  SolverWidget* solver_;

  LedIndicator* collisionIndicator_;

  QStandardItemModel *bodyTreeModel_, *jointTreeModel_;

  QMap <std::string, JointElement> jointsMap_;

  PluginManager pluginManager_;

  struct LoadDoneStruct {
    LoadDoneStruct () : id (-1), parent (MainWindow::instance()) {}
    virtual ~LoadDoneStruct () {};
    virtual void done ();
    bool isValid () { return id >= 0; }
    void invalidate () { id = -1; }
    bool is (int ID) { return id == ID; }
    int id;
    QString what;
    MainWindow* parent;
  };
  friend struct LoadDoneStruct;

  struct LoadRobot : public LoadDoneStruct {
    std::string name_, urdfSuf_, srdfSuf_,
      package_, modelName_, rootJointType_;
    DialogLoadRobot::RobotDefinition rd;
    virtual void done ();
  };
  struct LoadEnvironment : public LoadDoneStruct {
    std::string prefix_, urdfFilename_, package_;
    virtual void done ();
  };
  QList <LoadDoneStruct*> loader_;

  QMutex delayedCreateView_;
};

#endif // MAINWINDOW_H
