#include "mainwindow.h"
#include "osgwidget.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui_(new Ui::MainWindow)
{
  ui_->setupUi(this);

  QTreeView* treeView = findChild <QTreeView*>("treeView");
  QStandardItemModel* standardModel = new QStandardItemModel ;
  QStandardItem *rootNode = standardModel->invisibleRootItem();

  //defining a couple of items
  QStandardItem *americaItem = new QStandardItem("America");
  QStandardItem *mexicoItem =  new QStandardItem("Canada");
  QStandardItem *usaItem =     new QStandardItem("USA");
  QStandardItem *bostonItem =  new QStandardItem("Boston");
  QStandardItem *europeItem =  new QStandardItem("Europe");
  QStandardItem *italyItem =   new QStandardItem("Italy");
  QStandardItem *romeItem =    new QStandardItem("Rome");
  QStandardItem *veronaItem =  new QStandardItem("Verona");

  //building up the hierarchy
  rootNode->    appendRow(americaItem);
  rootNode->    appendRow(europeItem);
  americaItem-> appendRow(mexicoItem);
  americaItem-> appendRow(usaItem);
  usaItem->     appendRow(bostonItem);
  europeItem->  appendRow(italyItem);
  italyItem->   appendRow(romeItem);
  italyItem->   appendRow(veronaItem);

  italyItem->setCheckable(true);

  //register the model
  treeView->setModel(standardModel);
  treeView->expandAll();
}

MainWindow::~MainWindow()
{
  delete ui_;
  delete mdiArea_;
}

void MainWindow::onCreateView()
{
  QDockWidget* dockOSG = new QDockWidget (tr("OSG Viewer"), this);
  OSGWidget* osgWidget = new OSGWidget( this );
  dockOSG->setWidget(osgWidget);
  addDockWidget(Qt::RightDockWidgetArea, dockOSG);
}
