from PythonQt.QtGui import QDockWidget, QWidget, QLabel, QPushButton, QVBoxLayout, QFormLayout
from PythonQt.Qt import Qt as QNamespace, QAction, QKeySequence
from hpp.corbaserver import Client as BasicClient
from hpp.corbaserver.manipulation import Client as ManipClient, ConstraintGraph, Rule
from hpp.corbaserver.manipulation.pr2 import Robot
from gepetto.corbaserver import Client as ViewerClient
import re

class _Clients(object):
    def __init__(self):
        self.basic = BasicClient()
        self.manipulation = ManipClient()
        self.viewer = ViewerClient()

class _DualSelect(object):
    def __init__(self, parent, mainWindow):
        self.parent = parent
        self.mainWindow = mainWindow
        self.selectedHandles = []
        self.selectedGrippers = []
        self.grippers = []
        self.handles = []
        self.locked = []
        self.groupName = ""
        self.currentGripper = 0
        self.currentHandle = 0
        self.actions = []

        self.initActions()
        self.createWidget()

    def initActions(self):
        action = QAction("Grasp from current", self.parent)
        action.connect("triggered()", self.graspCurrent)
        action.setShortcut(QKeySequence(QNamespace.Key_G))
        self.mainWindow.registerShortcut("Dynamic builder", action)
        self.actions.append(action)

        action = QAction("Grasp from random", self.parent)
        action.connect("triggered()", self.graspRandom)
        action.setShortcut(QKeySequence(QNamespace.ShiftModifier + QNamespace.Key_G))
        self.mainWindow.registerShortcut("Dynamic builder", action)
        self.actions.append(action)

        action = QAction("Pregrasp from current", self.parent)
        action.connect("triggered()", self.pregraspCurrent)
        action.setShortcut(QKeySequence(QNamespace.Key_P))
        self.mainWindow.registerShortcut("Dynamic builder", action)
        self.actions.append(action)

        action = QAction("Pregrasp from random", self.parent)
        action.connect("triggered()", self.pregraspRandom)
        action.setShortcut(QKeySequence(QNamespace.ShiftModifier + QNamespace.Key_P))
        self.mainWindow.registerShortcut("Dynamic builder", action)
        self.actions.append(action)

        action = QAction("Change handle used", self.parent)
        action.connect("triggered()", self.changeGripper)
        action.setShortcut(QKeySequence(QNamespace.Key_F1))
        self.mainWindow.registerShortcut("Dynamic builder", action)
        self.actions.append(action)

        action = QAction("Change gripper used", self.parent)
        action.connect("triggered()", self.changeGripper)
        action.setShortcut(QKeySequence(QNamespace.Key_F2))
        self.mainWindow.registerShortcut("Dynamic builder", action)
        self.actions.append(action)

        action = QAction("Lock current object", self.parent)
        action.connect("triggered()", self.lock)
        action.setShortcut(QKeySequence(QNamespace.Key_L))
        self.mainWindow.registerShortcut("Dynamic builder", action)
        self.actions.append(action)

    def createWidget(self):
        self.w = QWidget(self.parent)
        box = QVBoxLayout(self.w)
        self.text = "You're in grasp/pregrasp mode.\n" +\
        "In this mode, you can select two object.\n" +\
        "To do so, click on it.\n" +\
        "Once you choose one press :\n"

        for a in self.parent.actionsList:
            self.text += "  - " + a.shortcut.toString() + " to " + a.text + ".\n"

        self.text += "Once you have selected one of each, press :\n"

        self.w.addActions(self.actions)
        for a in self.actions:
            self.text += "  - " + a.shortcut.toString() + " to " + a.text + ".\n"

        box.addWidget(QLabel(self.text, self.w))
        fBox = QFormLayout()
        self.handleLabel = QLabel("None selected", self.w)
        fBox.addRow("Current handle", self.handleLabel)
        self.gripperLabel = QLabel("None selected", self.w)
        fBox.addRow("Current gripper", self.gripperLabel)
        box.addLayout(fBox)

    def getWidget(self):
        return self.w

    def addHandle(self, handleName):
        if (len(self.selectedHandles) == 1):
            self.parent.plugin.client.viewer.gui.deleteNode(self.groupName, True)
        self.selectedHandles = [str(handleName)]
        self.handles = self.getAvailable(handleName + "/", "handle")
        self.currentHandle = 0
        if (len(self.handles)):
            config = self.parent.plugin.client.manipulation.robot.getHandlePositionInJoint(self.handles[self.currentHandle])
            self.handleLabel.setText(self.handles[self.currentHandle])
            self.drawXYZAxis("handle_"+self.handles[self.currentHandle].replace("/", "_"), config)
        else:
            self.handleLabel.setText("None selected")

    def addGripper(self, gripperName):
        if (len(self.selectedGrippers) == 1):
            self.parent.plugin.client.viewer.gui.deleteNode(self.groupName, True)
        self.selectedGrippers = [str(gripperName)]
        self.grippers = self.getAvailable(gripperName + "/", "gripper")
        self.currentGripper = 0
        self.locked = []
        if (len(self.grippers) > 0):
            self.gripperLabel.setText(self.grippers[self.currentGripper])
            config = self.parent.plugin.client.manipulation.robot.getGripperPositionInJoint(self.grippers[self.currentGripper])
            self.drawXYZAxis("gripper_"+self.grippers[self.currentGripper].replace("/", "_"), config)
        else:
            self.gripperLabel.setText("None selected")

    def getAvailable(self, comp, t):
        l = self.parent.plugin.client.manipulation.problem.getAvailable(t)
        ret = []
        for name in l:
            if (name.startswith(comp)):
                ret.append(name)
        return ret
        
    def drawXYZAxis(self, name, config):
        obj = self.mainWindow.getFromSlot("requestCreateJointGroup")
        self.groupName = str(obj.requestCreateJointGroup(config[0]))
        self.parent.plugin.client.viewer.gui.addXYZaxis(name, [0, 1, 0, 1], 0.005, 1)
        self.parent.plugin.client.viewer.gui.applyConfiguration(name, config[1])
        self.parent.plugin.client.viewer.gui.addToGroup(name, self.groupName)
        self.parent.plugin.client.viewer.gui.refresh()

    def changeHandle(self):
        if (len(self.handles) > 0):
            previous = self.currentHandle
            self.currentHandle += 1
            if (self.currentHandle == len(self.handles)):
                self.currentHandle = 0
            self.handleLabel.setText(self.handles[self.currentHandle])
            if (previous != self.currentHandle):
                self.parent.plugin.client.viewer.gui.deleteNode(self.groupName, True)
                config = self.parent.plugin.client.manipulation.robot.getHandlePositionInJoint(self.handles[self.currentHandle])
                self.drawXYZAxis("handle_"+self.handles[self.currentHandle].replace("/", "_"), config)

    def changeGripper(self):
        if (len(self.grippers) > 0):
            previous = self.currentGripper
            self.currentGripper += 1
            if (self.currentGripper == len(self.grippers)):
                self.currentGripper = 0
            self.gripperLabel.setText(self.grippers[self.currentGripper])
            if (self.currentGripper != previous):
                self.parent.plugin.client.viewer.gui.deleteNode(self.groupName, True)
                config = self.parent.plugin.client.manipulation.robot.getGripperPositionInJoint(self.grippers[self.currentGripper])
                self.drawXYZAxis("gripper_"+self.grippers[self.currentGripper].replace("/", "_"), config)

    def graspCurrent(self):
        if (len(self.handles) > 0 and len(self.grippers) > 0):
            self.grasp(self.parent.plugin.client.basic.robot.getCurrentConfig())

    def graspRandom(self):
        if (len(self.handles) > 0 and len(self.grippers) > 0):
            self.grasp(self.parent.plugin.client.basic.robot.shootRandomConfig())

    def grasp(self, config):
        self.parent.plugin.client.basic.problem.resetConstraints()
        for j in self.locked:
            self.parent.plugin.client.basic.problem.lockJoint(j, self.parent.plugin.client.basic.robot.getJointConfig(j))
        name = self.grippers[self.currentGripper] + " grasps " + self.handles[self.currentHandle]
        self.parent.plugin.client.manipulation.problem.createGrasp(name, self.grippers[self.currentGripper], self.handles[self.currentHandle])
        self.parent.plugin.client.basic.problem.setNumericalConstraints("constraints", [name], [True])
        res = self.parent.plugin.client.basic.problem.applyConstraints(config)
        if res[0] == True:
            self.parent.plugin.client.basic.robot.setCurrentConfig(res[1])
            self.mainWindow.requestApplyCurrentConfiguration()

    def pregraspCurrent(self):
        if (len(self.handles) > 0 and len(self.grippers) > 0):
            self.pregrasp(self.parent.plugin.client.basic.robot.getCurrentConfig())

    def pregraspRandom(self):
        if (len(self.handles) > 0 and len(self.grippers) > 0):
            self.pregrasp(self.parent.plugin.client.basic.robot.shootRandomConfig())

    def pregrasp(self, config):
        self.parent.plugin.client.basic.problem.resetConstraints()
        for j in self.locked:
            self.parent.plugin.client.basic.problem.lockJoint(name, self.parent.plugin.client.basic.robot.getJointConfig())
        name = self.grippers[self.currentGripper] + " pregrasps " + self.handles[self.currentHandle]
        self.parent.plugin.client.manipulation.problem.createGrasp(name, self.grippers[self.currentGripper], self.handles[self.currentHandle])
        self.parent.plugin.client.basic.problem.setNumericalConstraints("constraints", [name], [True])
        res = self.parent.plugin.client.basic.problem.applyConstraints(self.parent.plugin.client.basic.robot.getCurrentConfig())
        if (res[0] == True):
            self.parent.plugin.client.basic.robot.setCurrentConfig(res[1])
            self.mainWindow.requestApplyCurrentConfiguration()

    def lock(self):
        if (self.parent.selected != ""):
            self.locked = []
            joints = self.parent.plugin.client.basic.robot.getAllJointNames()
            name = self.parent.selected + "/"
            for j in joints:
                if j.startswith(name):
                    self.locked.append(j)

class _DynamicBuilder(QWidget):
    def __init__(self, mainWindow, parent):
        super(_DynamicBuilder, self).__init__(parent)
        self.plugin = parent
        self.selected = ""
        self.mainWindow = mainWindow
        self.actionsList = []

        self.mainWindow.connect("selectJointFromBodyName(QString)", self.changeSelected)

        # Init the widget view
        self.initActions()
        self.modeInstance = _DualSelect(self, mainWindow)
        self.initWidget()

    def initActions(self):
        action = QAction("Choose as gripper", self)
        action.connect("triggered()", self.selectGripper)
        action.setShortcut(QKeySequence(QNamespace.Key_R))
        self.addAction(action)
        self.actionsList.append(action)
        self.mainWindow.registerShortcut("Dynamic builder", "Choose as gripper", action)

        action = QAction("Choose handle", self)
        action.connect("triggered()", self.selectHandle)
        action.setShortcut(QKeySequence(QNamespace.Key_H))
        self.addAction(action)
        self.actionsList.append(action)
        self.mainWindow.registerShortcut("Dynamic builder", "Choose as handle", action)

    def changeSelected(self, name):
        self.selected = name.split("/")[0]

    def initWidget(self):
        mainBox = QVBoxLayout(self)

        mainBox.addWidget(self.modeInstance.getWidget())

    def selectHandle(self):
        if (self.selected != ""):
            self.modeInstance.addHandle(self.selected)

    def selectGripper(self):
        if (self.selected != ""):
            self.modeInstance.addGripper(self.selected)

class Plugin(QDockWidget):
    def __init__(self, mainWindow, flags = None):
        if (flags is not None):
            super(Plugin, self).__init__("Dynamic Builder", flags)
        else:
            super(Plugin, self).__init__("Dynamic Builder")
        self.resetConnection()
        mainWindow.registerShortcut("Dynamic builder", "Toggle view", self.toggleViewAction())
        self.dynamicBuilder = _DynamicBuilder(mainWindow, self)
        self.setWidget(self.dynamicBuilder)

    def resetConnection(self):
        self.client = _Clients()
