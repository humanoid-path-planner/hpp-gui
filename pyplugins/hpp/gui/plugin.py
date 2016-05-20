from PythonQt import QtGui, Qt
from hpp.corbaserver import Client
from directpath import DirectPathBox
import sys
sys.argv = ["none"]

class _PathTab(QtGui.QWidget):
    def __init__ (self, parent):
        super(_PathTab, self).__init__ (parent)
        box = QtGui.QVBoxLayout(self)

        # Create group
        box.addWidget(DirectPathBox(self))

class Plugin(QtGui.QDockWidget):
    """ Extra HPP functionalities for the Gepetto Viewer GUI """
    def __init__ (self, mainWindow, flags = None):
        title = "HPP extra functionalities"
        if flags is None:
            super(Plugin, self).__init__ (title, mainWindow)
        else:
            super(Plugin, self).__init__ (title, mainWindow, flags)
        # Initialize the widget
        self.tabWidget = QtGui.QTabWidget(self)
        self.setWidget (self.tabWidget)
        self.nodeCreator = _PathTab(self)
        self.tabWidget.addTab (self.nodeCreator, "Path")
