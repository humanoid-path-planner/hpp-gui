from PythonQt import QtGui, Qt
from hpp.corbaserver import Client
import sys
sys.argv = ["none"]

class DirectPath:
    def __init__ (self):
        self.fromCfg = []
        self.toCfg = []
        self.client = Client()

    def getFrom (self):
        self.fromCfg = self.client.robot.getCurrentConfig()

    def getTo (self):
        self.toCfg = self.client.robot.getCurrentConfig()

    def makePath (self):
        self.client.problem.directPath (self.fromCfg, self.toCfg, self.validatePath.isChecked())

    def createWidget (self):
        self.dialog = QtGui.QDialog(None, Qt.Qt.WindowStaysOnTopHint)
        box = QtGui.QVBoxLayout(self.dialog)
        setFrom =  QtGui.QPushButton(self.dialog)
        box.addWidget(setFrom)
        setTo =  QtGui.QPushButton(self.dialog)
        box.addWidget(setTo)
        self.validatePath = QtGui.QCheckBox(self.dialog)
        box.addWidget(self.validatePath)
        makePath = QtGui.QPushButton(self.dialog)
        box.addWidget(makePath)
        setFrom.text = 'Save config as origin'
        setTo.text = 'Save config as destination'
        self.validatePath.text = 'Validate path'
        makePath.text = 'Create path'
        setFrom.connect('clicked()', self.getFrom)
        setTo.connect('clicked()', self.getTo)
        makePath.connect('clicked()', self.makePath)
        self.dialog.show()

if __name__ == "__main__":
    directPath = DirectPath()
    toolBar = mainWindow.addToolBar("PyPlugins")
    makePath = QtGui.QAction ("Direct path", toolBar)
    toolBar.addAction(makePath)
    makePath.connect('activated()', directPath.createWidget)
