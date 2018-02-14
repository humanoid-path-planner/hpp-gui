#
#  Copyright (c) CNRS
#  Author: Joseph Mirabel
#

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

    def shootRandom(self):
        q = self.plugin.client.robot.shootRandomConfig()
        self.plugin.client.robot.setCurrentConfig(q)
        self.plugin.main.requestApplyCurrentConfiguration()

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
                if self.projectPath.isChecked ():
                    success = self.plugin.client.problem.projectPath(pid)
                    if not success:
                        self.plugin.main.logError ("Path could not be projected.")

        else:
            self.plugin.main.logError ("Configuration does not have the good size. Did you save them ?")

    def initWidget (self):
        box = QtGui.QVBoxLayout(self)
        random =  QtGui.QPushButton(self)
        box.addWidget(random)
        setFrom =  QtGui.QPushButton(self)
        box.addWidget(setFrom)
        setTo =  QtGui.QPushButton(self)
        box.addWidget(setTo)
        self.validatePath = QtGui.QCheckBox(self)
        box.addWidget(self.validatePath)
        self.projectPath = QtGui.QCheckBox(self)
        box.addWidget(self.projectPath)
        makePath = QtGui.QPushButton(self)
        box.addWidget(makePath)
        random.text = "Shoot random config"
        setFrom.text = 'Save config as origin'
        setTo.text = 'Save config as destination'
        self.validatePath.text = 'Validate path'
        self.projectPath.text = "Project path"
        makePath.text = 'Create path'
        random.connect('clicked()', self.shootRandom)
        setFrom.connect('clicked()', self.getFrom)
        setTo.connect('clicked()', self.getTo)
        makePath.connect('clicked()', self.makePath)
