#include "hpp/gui/config-python.hh"

#if PYTHONQT_NEED_INCLUDE==1
#include <QFileDialog>
#include "hpp/gui/osgwidget.hh"
#include "hpp/gui/pythonwidget.hh"
#include "hpp/gui/mainwindow.hh"

namespace hpp {
    namespace gui {
        PythonWidget::PythonWidget(QWidget *parent) :
            QDockWidget("PythonQt console", parent)
        {
            PythonQt::init();
            PythonQt_QtAll::init();
            mainContext_ = PythonQt::self()->getMainModule();
            console_ = new PythonQtScriptingConsole(NULL, mainContext_);
            mainContext_.addObject("mainWindow", MainWindow::instance());

            QWidget* widget = new QWidget;
            QVBoxLayout* layout = new QVBoxLayout;
            button_ = new QPushButton;

            button_->setText("Choose file");
            layout->addWidget(console_);
            layout->addWidget(button_);
            widget->setLayout(layout);
            this->layout()->addWidget(widget);

            connect(button_, SIGNAL(clicked()), SLOT(browseFile()));
        }

        void PythonWidget::browseFile() {
            QFileDialog* fd = new QFileDialog;

            fd->setFileMode(QFileDialog::ExistingFile);
            fd->setNameFilter("All python file (*.py)");
            if (fd->exec() == QDialog::Accepted) {
                QStringList file = fd->selectedFiles();

                mainContext_.evalFile(file.at(0));
            }
            fd->close();
            fd->deleteLater();
        }

        void PythonWidget::addToContext(QString const& name, QObject* obj) {
            mainContext_.addObject(name, obj);
        }
    }
}

#endif
