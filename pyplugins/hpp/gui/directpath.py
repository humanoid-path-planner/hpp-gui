from PythonQt import QtGui, Qt
from hpp.corbaserver import Client
import sys
sys.argv = ["none"]

class DirectPathBox(QtGui.QGroupBox):
    def __init__ (self, parent):
        super(DirectPathBox, self).__init__ ("Direct Path", parent)
        self.fromCfg = []
        self.toCfg = []
        self.client = Client()
        self.initWidget()

    def getFrom (self):
        self.fromCfg = self.client.robot.getCurrentConfig()

    def getTo (self):
        self.toCfg = self.client.robot.getCurrentConfig()

    def makePath (self):
        self.client.problem.directPath (self.fromCfg, self.toCfg, self.validatePath.isChecked())

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
