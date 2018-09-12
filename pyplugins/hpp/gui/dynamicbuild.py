#
#  Copyright (c) CNRS
#  Authors: Heidy Dallard, Joseph Mirabel
#

from PythonQt.QtGui import QDockWidget, QWidget, QLabel, QPushButton, QVBoxLayout, QFormLayout
from PythonQt.Qt import Qt as QNamespace, QAction, QKeySequence
from hpp.corbaserver import Client as BasicClient
from hpp.corbaserver.manipulation import Client as ManipClient, ConstraintGraph, Rule
from gepetto.corbaserver import Client as ViewerClient
import re

def xyzwTowxyz(q):
    return [q[(i+1)%4] for i in range(4)]

def fromHPP(t):
    ret = t[0:3]
    ret.extend(xyzwTowxyz(t[3:7]))
    return ret

class _Clients(object):
    def __init__(self, mainWindow):
        self.hppPlugin = mainWindow.getFromSlot("getHppIIOPurl")
        self.basic = BasicClient(url= str(self.hppPlugin.getHppIIOPurl()),
                postContextId= str(self.hppPlugin.getHppContext()))
        self.manipulation = ManipClient(url= str(self.hppPlugin.getHppIIOPurl()),)
                postContextId= str(self.hppPlugin.getHppContext()))
        self.viewer = ViewerClient()

class _GraspMode(QWidget):
    def __init__(self, parent):
        super(_GraspMode, self).__init__(parent)
        self.parentInstance = parent
        self.mainWindow = parent.mainWindow
        self.selectedHandles = []
        self.selectedGrippers = []
        self.grippers = []
        self.handles = []
        self.locked = []
        self.groupName = ""
        self.currentGripper = 0
        self.currentHandle = 0
        self.actionsList = []

        self.initActions()
        self.createWidget()

    def initActions(self):
        action = QAction("Choose as gripper", self)
        action.connect("triggered()", self.selectGripper)
        action.setShortcut(QKeySequence(QNamespace.Key_R))
        self.addAction(action)
        self.actionsList.append(action)
        self.mainWindow.registerShortcut(self.parentInstance.plugin.windowTitle, "Choose as gripper", action)

        action = QAction("Choose handle", self)
        action.connect("triggered()", self.selectHandle)
        action.setShortcut(QKeySequence(QNamespace.Key_H))
        self.addAction(action)
        self.actionsList.append(action)
        self.mainWindow.registerShortcut(self.parentInstance.plugin.windowTitle, "Choose as handle", action)

        action = QAction("Grasp from current", self.parentInstance)
        action.connect("triggered()", self.graspCurrent)
        action.setShortcut(QKeySequence(QNamespace.Key_G))
        self.mainWindow.registerShortcut(self.parentInstance.plugin.windowTitle, action)
        self.actionsList.append(action)

        action = QAction("Grasp from random", self.parentInstance)
        action.connect("triggered()", self.graspRandom)
        action.setShortcut(QKeySequence(QNamespace.ShiftModifier + QNamespace.Key_G))
        self.mainWindow.registerShortcut(self.parentInstance.plugin.windowTitle, action)
        self.actionsList.append(action)

        action = QAction("Pregrasp from current", self.parentInstance)
        action.connect("triggered()", self.pregraspCurrent)
        action.setShortcut(QKeySequence(QNamespace.Key_P))
        self.mainWindow.registerShortcut(self.parentInstance.plugin.windowTitle, action)
        self.actionsList.append(action)

        action = QAction("Pregrasp from random", self.parentInstance)
        action.connect("triggered()", self.pregraspRandom)
        action.setShortcut(QKeySequence(QNamespace.ShiftModifier + QNamespace.Key_P))
        self.mainWindow.registerShortcut(self.parentInstance.plugin.windowTitle, action)
        self.actionsList.append(action)

        action = QAction("Change handle used", self.parentInstance)
        action.connect("triggered()", self.changeHandle)
        action.setShortcut(QKeySequence(QNamespace.Key_F1))
        self.mainWindow.registerShortcut(self.parentInstance.plugin.windowTitle, action)
        self.actionsList.append(action)

        action = QAction("Change gripper used", self.parentInstance)
        action.connect("triggered()", self.changeGripper)
        action.setShortcut(QKeySequence(QNamespace.Key_F2))
        self.mainWindow.registerShortcut(self.parentInstance.plugin.windowTitle, action)
        self.actionsList.append(action)

        action = QAction("Lock current object", self.parentInstance)
        action.connect("triggered()", self.lock)
        action.setShortcut(QKeySequence(QNamespace.Key_L))
        self.mainWindow.registerShortcut(self.parentInstance.windowTitle, action)
        self.actionsList.append(action)

    def createWidget(self):
        box = QVBoxLayout(self)
        self.text = "You're in grasp/pregrasp mode.\n" +\
        "In this mode, you can select two object.\n" +\
        "To do so, click on it.\n" +\
        "Once you choose one, press :\n"

        self.addActions(self.actionsList)
        for a in self.actionsList:
            self.text += "  - " + a.shortcut.toString() + " to " + a.text + ".\n"

        box.addWidget(QLabel(self.text, self))
        fBox = QFormLayout()
        self.handleLabel = QLabel("None selected", self)
        fBox.addRow("Current handle", self.handleLabel)
        self.gripperLabel = QLabel("None selected", self)
        fBox.addRow("Current gripper", self.gripperLabel)
        box.addLayout(fBox)

        self.mainWindow.connect("selectJointFromBodyName(QString)", self.changeSelected)

    def changeSelected(self, name):
        self.selected = name.split("/")[0]

    def selectHandle(self):
        if (self.selected != ""):
            self.addHandle(self.selected)

    def selectGripper(self):
        if (self.selected != ""):
            self.addGripper(self.selected)

    def addHandle(self, handleName):
        if (len(self.selectedHandles) == 1):
            self.parentInstance.plugin.client.viewer.gui.deleteNode(self.groupName, True)
        self.selectedHandles = [str(handleName)]
        self.handles = self.getAvailable(handleName + "/", "handle")
        self.currentHandle = 0
        if (len(self.handles)):
            config = self.parentInstance.plugin.client.manipulation.robot.getHandlePositionInJoint(self.handles[self.currentHandle])
            self.handleLabel.setText(self.handles[self.currentHandle])
            self.drawXYZAxis("handle_"+self.handles[self.currentHandle].replace("/", "_"), config)
        else:
            self.handleLabel.setText("None selected")

    def addGripper(self, gripperName):
        if (len(self.selectedGrippers) == 1):
            self.parentInstance.plugin.client.viewer.gui.deleteNode(self.groupName, True)
        self.selectedGrippers = [str(gripperName)]
        self.grippers = self.getAvailable(gripperName + "/", "gripper")
        self.currentGripper = 0
        self.locked = []
        if (len(self.grippers) > 0):
            self.gripperLabel.setText(self.grippers[self.currentGripper])
            config = self.parentInstance.plugin.client.manipulation.robot.getGripperPositionInJoint(self.grippers[self.currentGripper])
            self.drawXYZAxis("gripper_"+self.grippers[self.currentGripper].replace("/", "_"), config)
        else:
            self.gripperLabel.setText("None selected")

    def getAvailable(self, comp, t):
        l = self.parentInstance.plugin.client.manipulation.problem.getAvailable(t)
        ret = []
        for name in l:
            if (name.startswith(comp)):
                ret.append(name)
        return ret
        
    def drawXYZAxis(self, name, config):
        obj = self.mainWindow.getFromSlot("requestCreateJointGroup")
        self.groupName = str(obj.requestCreateJointGroup(config[0]))
        self.parentInstance.plugin.client.viewer.gui.addXYZaxis(name, [0, 1, 0, 1], 0.005, 0.015)
        self.parentInstance.plugin.client.viewer.gui.applyConfiguration(name, fromHPP(config[1])) # XYZW -> WXYZ
        self.parentInstance.plugin.client.viewer.gui.addToGroup(name, self.groupName)
        self.parentInstance.plugin.client.viewer.gui.refresh()

    def changeHandle(self):
        if (len(self.handles) > 0):
            previous = self.currentHandle
            self.currentHandle += 1
            if (self.currentHandle == len(self.handles)):
                self.currentHandle = 0
            self.handleLabel.setText(self.handles[self.currentHandle])
            if (previous != self.currentHandle):
                self.parentInstance.plugin.client.viewer.gui.deleteNode(self.groupName, True)
                config = self.parentInstance.plugin.client.manipulation.robot.getHandlePositionInJoint(self.handles[self.currentHandle])
                self.drawXYZAxis("handle_"+self.handles[self.currentHandle].replace("/", "_"), config)

    def changeGripper(self):
        if (len(self.grippers) > 0):
            previous = self.currentGripper
            self.currentGripper += 1
            if (self.currentGripper == len(self.grippers)):
                self.currentGripper = 0
            self.gripperLabel.setText(self.grippers[self.currentGripper])
            if (self.currentGripper != previous):
                self.parentInstance.plugin.client.viewer.gui.deleteNode(self.groupName, True)
                config = self.parentInstance.plugin.client.manipulation.robot.getGripperPositionInJoint(self.grippers[self.currentGripper])
                self.drawXYZAxis("gripper_"+self.grippers[self.currentGripper].replace("/", "_"), config)

    def graspCurrent(self):
        if (len(self.handles) > 0 and len(self.grippers) > 0):
            self.grasp(self.parentInstance.plugin.client.basic.robot.getCurrentConfig())

    def graspRandom(self):
        if (len(self.handles) > 0 and len(self.grippers) > 0):
            self.grasp(self.parentInstance.plugin.client.basic.robot.shootRandomConfig())

    def grasp(self, config):
        self.parentInstance.plugin.client.basic.problem.resetConstraints()
        for j in self.locked:
            self.parentInstance.plugin.client.basic.problem.lockJoint(j, self.parentInstance.plugin.client.basic.robot.getJointConfig(j))
        name = self.grippers[self.currentGripper] + " grasps " + self.handles[self.currentHandle]
        self.parentInstance.plugin.client.manipulation.problem.createGrasp(name, self.grippers[self.currentGripper], self.handles[self.currentHandle])
        self.parentInstance.plugin.client.basic.problem.setNumericalConstraints("constraints", [name,], [0,])
        res = self.parentInstance.plugin.client.basic.problem.applyConstraints(config)
        if res[0] == True:
            self.parentInstance.plugin.client.basic.robot.setCurrentConfig(res[1])
            self.mainWindow.requestApplyCurrentConfiguration()

    def pregraspCurrent(self):
        if (len(self.handles) > 0 and len(self.grippers) > 0):
            self.pregrasp(self.parentInstance.plugin.client.basic.robot.getCurrentConfig())

    def pregraspRandom(self):
        if (len(self.handles) > 0 and len(self.grippers) > 0):
            self.pregrasp(self.parentInstance.plugin.client.basic.robot.shootRandomConfig())

    def pregrasp(self, config):
        self.parentInstance.plugin.client.basic.problem.resetConstraints()
        for j in self.locked:
            self.parentInstance.plugin.client.basic.problem.lockJoint(name, self.parentInstance.plugin.client.basic.robot.getJointConfig())
        name = self.grippers[self.currentGripper] + " pregrasps " + self.handles[self.currentHandle]
        self.parentInstance.plugin.client.manipulation.problem.createGrasp(name, self.grippers[self.currentGripper], self.handles[self.currentHandle])
        self.parentInstance.plugin.client.basic.problem.setNumericalConstraints("constraints", [name], [True])
        res = self.parentInstance.plugin.client.basic.problem.applyConstraints(self.parentInstance.plugin.client.basic.robot.getCurrentConfig())
        if (res[0] == True):
            self.parentInstance.plugin.client.basic.robot.setCurrentConfig(res[1])
            self.mainWindow.requestApplyCurrentConfiguration()

    def lock(self):
        if (self.selected != ""):
            self.locked = []
            joints = self.parentInstance.plugin.client.basic.robot.getAllJointNames()
            name = self.selected + "/"
            for j in joints:
                if j.startswith(name):
                    self.locked.append(j)

class _PlacementMode(QWidget):
    def __init__(self, parent):
        super(_PlacementMode, self).__init__(parent)
        self.step = -1
        self.surfaceName = ""
        self.carryName = ""

        self.initActions()
        self.createWidget()

    def initActions(self):
        self.action = QAction("Start pick", self)
        self.action.setShortcut(QKeySequence(QNamespace.Key_P))
        self.action.connect("triggered()", self.startPlacement)
        self.parent().mainWindow.registerShortcut(self.parent().plugin.windowTitle, self.action)
        self.addAction(self.action)

    def startPlacement(self):
        if (self.parent().plugin.osg is not None):
            self.step = -1
            self.parent().plugin.osg.connect("clicked(QString, QVector3D)", self.selected)

    def selected(self, bodyName, position):
        selectedName = bodyName[bodyName.rfind("/contact_") + 9:len(bodyName)]
        selectedName = selectedName.replace("__", "/")
        selectedName = re.sub(r'_[0-9]*_', '', selectedName)
        if (self.step == -1):
            names = self.parent().plugin.client.manipulation.problem.getRobotContactNames()
            for n in names:
                if n == selectedName:
                    self.surfaceName = str(n)
                    self.step = 0
                    break
        else:
            names = self.parent().plugin.client.manipulation.problem.getEnvironmentContactNames()
            for n in names:
                if n == selectedName:
                    self.carryName = n
                    self.endPlacement(position)
                    break

    def endPlacement(self, position):
        name = self.surfaceName[0:self.surfaceName.find("/")] + "/base_joint"
        names = self.parent().plugin.client.basic.robot.getAllJointNames()
        for n in names:
            if n.startswith(name + "_xyz"):
                self.parent().plugin.client.basic.robot.setJointConfig(n, [position.x(), position.y(), position.z()])
                break
            elif n.startswith(name + "_xy"):
                self.parent().plugin.client.basic.robot.setJointConfig(n, [position.x(), position.y()])
                break
            elif n.startswith(name):
                self.parent().plugin.client.basic.robot.setJointPositionInParentFrame(n, [position.x(), position.y(), position.z(), 1, 0, 0, 0])
                break
        self.parent().plugin.osg.disconnect("clicked(QString, QVector3D)", self.selected)
        self.parent().plugin.client.manipulation.problem.createPlacementConstraint("placement",
                                                                                   [self.surfaceName], [self.carryName])
        self.parent().plugin.client.basic.problem.setNumericalConstraints("numerical", ["placement"], [True])
        res = self.parent().plugin.client.basic.problem.applyConstraints(self.parent().plugin.client.basic.robot.getCurrentConfig())
        if res[0]:
            self.parent().plugin.client.basic.robot.setCurrentConfig(res[1])
        self.parent().mainWindow.requestApplyCurrentConfiguration()

    def createWidget(self):
        box = QVBoxLayout(self)

        text = "You are in placement mode.\n" +\
               "In this mode you can place an object\non an environment surface.\n" +\
               "The object and the environment must\nhave contact surface defined.\n" +\
               "Press :\n" +\
               "  - " + self.action.shortcut.toString() + " to " + self.action.text + "\n\n" +\
               "First selected body will be\nconsidered as the object.\n" +\
               "Second selected body will be\nconsidered as the environment.\n" +\
               "The object will be place where\nyou clicked on the environment."
        box.addWidget(QLabel(text, self))

class _DynamicBuilder(QWidget):
    def __init__(self, mainWindow, parent):
        super(_DynamicBuilder, self).__init__(parent)
        self.plugin = parent
        self.selected = ""
        self.mainWindow = mainWindow
        self.mode = 0

        # Init the widget view
        self.initActions()
        self.initWidget()

    def initActions(self):
        self.action = QAction("Switch mode", self)
        self.action.connect("triggered()", self.switchMode)
        self.action.setShortcut(QKeySequence(QNamespace.ControlModifier + QNamespace.Key_M))
        self.addAction(self.action)
        self.mainWindow.registerShortcut(self.plugin.windowTitle, self.action)

    def switchMode(self):
        self.mode = not self.mode
        if (not self.mode):
            self.widgets[1].hide()
            self.widgets[0].show()
        else:
            self.widgets[0].hide()
            self.widgets[1].show()

    def initWidget(self):
        mainBox = QVBoxLayout(self)

        mainBox.addWidget(QLabel("Press " + self.action.shortcut.toString() + " to " +\
                                 self.action.text))
        self.widgets = [_GraspMode(self), _PlacementMode(self)]
        mainBox.addWidget(self.widgets[0])
        mainBox.addWidget(self.widgets[1])
        self.widgets[1].hide()

class Plugin(QDockWidget):
    def __init__(self, mainWindow, flags = None):
        if (flags is not None):
            super(Plugin, self).__init__("Dynamic Builder", flags)
        else:
            super(Plugin, self).__init__("Dynamic Builder")
        self.osg = None
        self.mainWindow = mainWindow
        self.resetConnection()
        mainWindow.registerShortcut("Dynamic builder", "Toggle view", self.toggleViewAction())
        self.dynamicBuilder = _DynamicBuilder(mainWindow, self)
        self.setWidget(self.dynamicBuilder)

    def osgWidget(self, osg):
        if (self.osg is None):
            self.osg = osg

    def resetConnection(self):
        self.client = _Clients(self.mainWindow)
