#
#  Copyright (c) CNRS
#  Author: Joseph Mirabel
#

import sys

from PythonQt import QtGui

sys.argv = ["none"]


class DirectPathBox(QtGui.QGroupBox):
    def __init__(self, parent, plugin):
        super(DirectPathBox, self).__init__("Direct Path", parent)
        self.fromCfg = []
        self.toCfg = []
        self.plugin = plugin
        self.initWidget()

    def shootRandom(self):
        q = self.plugin.client.robot.shootRandomConfig()
        self.plugin.hppPlugin.setCurrentQtConfig(q)

    def applyConstraints(self):
        q0 = self.plugin.hppPlugin.getCurrentQtConfig()
        res, q1, err = self.plugin.client.problem.applyConstraints(q0)
        self.plugin.hppPlugin.setCurrentQtConfig(q1)
        if not res:
            self.plugin.main.logError("Projection failed: " + str(err))
        else:
            self.plugin.main.logError("Projection succeeded.")

    def getFrom(self):
        self.fromCfg = self.plugin.hppPlugin.getCurrentQtConfig()

    def getTo(self):
        self.toCfg = self.plugin.hppPlugin.getCurrentQtConfig()

    def makePath(self):
        n = self.plugin.client.robot.getConfigSize()
        if len(self.fromCfg) == n and len(self.toCfg) == n:
            success, pid, msg = self.plugin.client.problem.directPath(
                self.fromCfg, self.toCfg, self.validatePath.isChecked()
            )
            if not success:
                self.plugin.main.logError(msg)
            else:  # Success
                # It would be nice to have access to the Path Player widget in order to
                # select the good path index...
                if self.projectPath.isChecked():
                    success = self.plugin.client.problem.projectPath(pid)
                    if not success:
                        self.plugin.main.logError("Path could not be projected.")

        else:
            self.plugin.main.logError(
                "Configuration does not have the good size. Did you save them ?"
            )

    def initWidget(self):
        box = QtGui.QVBoxLayout(self)
        random = QtGui.QPushButton(self)
        box.addWidget(random)
        applyConstraints = QtGui.QPushButton(self)
        box.addWidget(applyConstraints)
        setFrom = QtGui.QPushButton(self)
        box.addWidget(setFrom)
        setTo = QtGui.QPushButton(self)
        box.addWidget(setTo)
        self.validatePath = QtGui.QCheckBox(self)
        box.addWidget(self.validatePath)
        self.projectPath = QtGui.QCheckBox(self)
        box.addWidget(self.projectPath)
        makePath = QtGui.QPushButton(self)
        box.addWidget(makePath)
        random.text = "Shoot random config"
        applyConstraints.text = "Apply constraints"
        setFrom.text = "Save config as origin"
        setTo.text = "Save config as destination"
        self.validatePath.text = "Validate path"
        self.projectPath.text = "Project path"
        makePath.text = "Create path"
        random.connect("clicked()", self.shootRandom)
        applyConstraints.connect("clicked()", self.applyConstraints)
        setFrom.connect("clicked()", self.getFrom)
        setTo.connect("clicked()", self.getTo)
        makePath.connect("clicked()", self.makePath)
