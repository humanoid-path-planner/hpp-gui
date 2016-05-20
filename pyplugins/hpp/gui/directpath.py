from PythonQt import QtGui, Qt
from hpp.corbaserver import Client
import sys
sys.argv = ["none"]

class DirectPathBox(QtGui.QGroupBox):
    def __init__ (self, parent, plugin):
        super(DirectPathBox, self).__init__ ("Direct Path", parent)
        self.fromCfg = []
        self.toCfg = []
        self.plugin = plugin
        self.initWidget()

    def getFrom (self):
        self.fromCfg = self.plugin.client.robot.getCurrentConfig()

    def getTo (self):
        self.toCfg = self.plugin.client.robot.getCurrentConfig()

    def makePath (self):
        n = self.plugin.client.robot.getConfigSize()
        if len(self.fromCfg) == n and len(self.toCfg) == n:
            success, pid, msg = self.plugin.client.problem.directPath (self.fromCfg, self.toCfg, self.validatePath.isChecked())
            if not success:
                self.plugin.main.logError (msg)
            else: # Success
                # It would be nice to have access to the Path Player widget in order to
                # select the good path index...
                pass
        else:
            self.plugin.main.logError ("Configuration does not have the good size. Did you save them ?")

    def initWidget (self):
        box = QtGui.QVBoxLayout(self)
        setFrom =  QtGui.QPushButton(self)
        box.addWidget(setFrom)
        setTo =  QtGui.QPushButton(self)
        box.addWidget(setTo)
        self.validatePath = QtGui.QCheckBox(self)
        box.addWidget(self.validatePath)
        makePath = QtGui.QPushButton(self)
        box.addWidget(makePath)
        setFrom.text = 'Save config as origin'
        setTo.text = 'Save config as destination'
        self.validatePath.text = 'Validate path'
        makePath.text = 'Create path'
        setFrom.connect('clicked()', self.getFrom)
        setTo.connect('clicked()', self.getTo)
        makePath.connect('clicked()', self.makePath)
