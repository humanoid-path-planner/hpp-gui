from PythonQt.QtGui import QDockWidget, QWidget, QLabel, QPushButton, QVBoxLayout, QFormLayout
from PythonQt.Qt import Qt as QNamespace
from hpp.corbaserver import Client as BasicClient
from hpp.corbaserver.manipulation import Client as ManipClient, ConstraintGraph, Rule
from hpp.corbaserver.manipulation.pr2 import Robot
from gepetto.corbaserver import Client as ViewerClient
import re

class Clients(object):
    def __init__(self):
        self.basic = BasicClient()
        self.manipulation = ManipClient()
        self.viewer = ViewerClient()

class DualSelect(object):
    def __init__(self, plugin, mainWindow):
        self.plugin = plugin
        self.selectedHandles = []
        self.selectedGrippers = []
        self.grippers = []
        self.handles = []
        self.groupName = ""
        self.shift = 0 # 0 shift not pressed - 1 shift pressed
        self.changed = False
        self.cg = None
        self.mainWindow = mainWindow
        self.currentGripper = 0
        self.currentHandle = 0

    def createWidget(self, parent):
        w = QWidget(parent)
        box = QVBoxLayout(w)

        box.addWidget(QLabel("You're in grasp/pregrasp mode.\n"
                             "In this mode, you can select two object.\n"
                             "To do so, click on it.\n"
                             "Once you choose one press :\n"
                             "- r to use his grippers\n"
                             "- h to use his handles\n"
                             "Once you have selected one of each, press :\n"
                             "- g to generate a grasp from current config\n"
                             "- G to generate a grasp from random config\n"
                             "- p to generate a pregrasp from current config\n"
                             "- P to generate a pregrasp from random config\n", w))
        fBox = QFormLayout()
        self.handleLabel = QLabel("None selected", w)
        fBox.addRow("Current handle", self.handleLabel)
        self.gripperLabel = QLabel("None selected", w)
        fBox.addRow("Current handle", self.gripperLabel)
        box.addLayout(fBox)
        return w

    def addHandle(self, handleName):
        if (len(self.selectedHandles) == 1):
            self.plugin.client.viewer.gui.deleteNode(self.groupName, True)
        self.changed = True
        self.selectedHandles = [str(handleName)]
        self.handles = self.getAvailable(self.selectedHandles[0] + "/", "handle")
        self.currentHandle = 0
        config = self.plugin.client.manipulation.robot.getHandlePositionInJoint(self.handles[self.currentHandle])
        self.handleLabel.setText(self.handles[self.currentHandle])
        self.drawXYZAxis("handle_"+self.handles[self.currentHandle].replace("/", "_"), config)

    def addGripper(self, gripperName):
        if (len(self.selectedGrippers) == 1):
            self.plugin.client.viewer.gui.deleteNode(self.groupName, True)
        self.changed = True
        self.selectedGrippers = [str(gripperName)]
        self.grippers = self.getAvailable(self.selectedGrippers[0] + "/", "gripper")
        self.currentGripper = 0
        self.gripperLabel.setText(self.grippers[self.currentGripper])
        config = self.plugin.client.manipulation.robot.getGripperPositionInJoint(self.grippers[self.currentGripper])
        self.drawXYZAxis("gripper_"+self.grippers[self.currentGripper].replace("/", "_"), config)

    def getAvailable(self, comp, t):
        l = self.plugin.client.manipulation.problem.getAvailable(t)
        ret = []
        for name in l:
            if (name.startswith(comp)):
                ret.append(name)
        return ret
        
    def drawXYZAxis(self, name, config):
        obj = self.mainWindow.getFromSlot("requestCreateJointGroup")
        self.groupName = str(obj.requestCreateJointGroup(config[0]))
        self.plugin.client.viewer.gui.addXYZaxis(name, [0, 1, 0, 1], 0.005, 1)
        self.plugin.client.viewer.gui.applyConfiguration(name, config[1])
        self.plugin.client.viewer.gui.addToGroup(name, self.groupName)
        self.plugin.client.viewer.gui.setVisibility(name, "ALWAYS_ON_TOP")
        self.plugin.client.viewer.gui.refresh()

    def changeHandle(self):
        if (len(self.handles) > 0):
            previous = self.currentHandle
            self.currentHandle += 1
            if (self.currentHandle == len(self.handles)):
                self.currentHandle = 0
            self.handleLabel.setText(self.handles[self.currentHandle])
            if (previous != self.currentHandle):
                self.plugin.client.viewer.gui.deleteNode(self.groupName, True)
                config = self.plugin.client.manipulation.robot.getHandlePositionInJoint(self.handles[self.currentHandle])
                self.drawXYZAxis("handle_"+self.handles[self.currentHandle].replace("/", "_"), config)

    def changeGripper(self):
        if (len(self.grippers) > 0):
            previous = self.currentGripper
            self.currentGripper += 1
            if (self.currentGripper == len(self.grippers)):
                self.currentGripper = 0
            self.gripperLabel.setText(self.grippers[self.currentGripper])
            if (self.currentGripper != previous):
                self.plugin.client.viewer.gui.deleteNode(self.groupName, True)
                config = self.plugin.client.manipulation.robot.getGripperPositionInJoint(self.grippers[self.currentGripper])
                self.drawXYZAxis("gripper_"+self.grippers[self.currentGripper].replace("/", "_"), config)

    def grasp(self):
        config = self.plugin.client.basic.robot.shootRandomConfig() if self.shift else self.plugin.client.basic.robot.getCurrentConfig()
        self.plugin.client.basic.problem.resetConstraints()
        print (self.currentGripper)
        print (self.currentHandle)
        name = self.grippers[self.currentGripper] + " grasps " + self.handles[self.currentHandle]
        self.plugin.client.manipulation.problem.createGrasp(name, self.grippers[self.currentGripper], self.handles[self.currentHandle])
        self.plugin.client.basic.problem.setNumericalConstraints("constraints", [name], [True])
        res = self.plugin.client.basic.problem.applyConstraints(config)
        self.plugin.client.basic.robot.setCurrentConfig(res[1])
        self.mainWindow.requestApplyCurrentConfiguration()

    def pregrasp(self):
        config = self.plugin.client.basic.robot.shootRandomConfig() if self.shift else self.plugin.client.basic.robot.getCurrentConfig()
        self.plugin.client.basic.problem.resetConstraints()
        name = self.grippers[self.currentGripper] + " pregrasps " + self.handles[self.currentHandle]
        self.plugin.client.manipulation.problem.createGrasp(name, self.grippers[self.currentGripper], self.handles[self.currentHandle])
        self.plugin.client.basic.problem.setNumericalConstraints("constraints", [name], [True])
        res = self.plugin.client.basic.problem.applyConstraints(self.plugin.client.basic.robot.getCurrentConfig())
        self.plugin.client.basic.robot.setCurrentConfig(res[1])
        self.mainWindow.requestApplyCurrentConfiguration()

    def handleEvent(self, key):
        if (key == QNamespace.Key_G):
            return self.grasp()
        elif (key == QNamespace.Key_P):
            return self.pregrasp()
        elif (key == QNamespace.Key_F1):
            self.changeHandle()
        elif (key == QNamespace.Key_F2):
            self.changeGripper()
        return False

    def setShift(self):
        self.shift = not self.shift

class RuleMaker(object):
    def __init__(self, plugin, mainWindow):
        self.plugin = client
        self.shift = 0
        self.selectedHandles = []
        self.selectedGrippers = []
        self.knownHandles = []
        self.knownGrippers = []
        self.rules = []

        obj = mainWindow.getFromSlot("requestCreateJointGroup")
        # Draw the handles in the viewer
        handles = self.plugin.client.manipulation.problem.getAvailable("handle")
        for handle in handles:
            config = self.plugin.client.manipulation.robot.getHandlePositionInJoint(handle)
            groupName = str(obj.requestCreateJointGroup(config[0]))
            name = "handle_" + handle.replace("/", "_")
            self.plugin.client.viewer.gui.addXYZaxis(name, [0, 1, 0, 1], 0.005, 1)
            self.plugin.client.viewer.gui.applyConfiguration(name, config[1])
            self.plugin.client.viewer.gui.addToGroup(name, groupName)
            self.plugin.client.viewer.gui.setVisibility(name, "ALWAYS_ON_TOP")
            self.knownHandles.append(name)

        # Draw the grippers in the viewer
        grippers = self.plugin.client.manipulation.problem.getAvailable("gripper")
        for gripper in grippers:
            config = self.plugin.client.manipulation.robot.getGripperPositionInJoint(gripper)
            groupName = str(obj.requestCreateJointGroup(config[0]))
            name = "gripper_" + gripper.replace("/", "_")
            self.plugin.client.viewer.gui.addXYZaxis(name, [0, 1, 0, 1], 0.005, 1)
            self.plugin.client.viewer.gui.applyConfiguration(name, config[1])
            self.plugin.client.viewer.gui.addToGroup(name, groupName)
            self.plugin.client.viewer.gui.setVisibility(name, "ALWAYS_ON_TOP")
            self.knownGrippers.append(name)
        self.plugin.client.viewer.gui.refresh()

    def addHandle(self, handleName):
        if (self.knownHandles.count(handleName) == 1):
            if (self.selectedHandles.count(handleName) == 0):
                self.selectedHandles.append(handleName)
            else:
                self.selectedHandles.remove(handleName)
            print self.selectedHandles

    def addGripper(self, gripperName):
        if (self.knownGrippers.count(handleName) == 1):
            if (self.selectedGrippers.count(gripperName) == 0):
                self.selectedGrippers.append(gripperName)
            else:
                self.selectedGrippers.remove(gripperName)
            print self.selectedHandles

    def setShift(self):
        self.shift = not self.shift

    def createRule(self):
        for gripper in self.selectedGrippers:
            for handle in self.selectedHandles:
                print ("%s - %s - %s" % (gripper, handle, self.shift))
                self.rule.append(Rule(gripper, handle, self.shift))

    def handleEvent(self, key):
        if (key == QNamespace.Key_L):
            self.createRule()
        return False

    def getRules(self):
        return self.rules.copy()

class DynamicBuilder(QWidget):
    def __init__(self, mainWindow, parent):
        super(DynamicBuilder, self).__init__(parent)
        self.plugin = parent
        self.mode = 0
        self.modeInstance = DualSelect(self.plugin, mainWindow)
        self.running = False
        self.selected = ""
        self.mainWindow = mainWindow

        # Init the widget view
        self.initWidget()

    def changeSelected(self, name):
        self.selected = name.split("/")[0]

    def onStart(self):
        self.mode = 0
        self.button.setText("Stop")
        self.button.disconnect("clicked()", self.onStart)
        self.button.connect("clicked()", self.onStop)
        self.grabKeyboard()
        self.running = True
        self.mainWindow.connect("selectJointFromBodyName(QString)", self.changeSelected)

    def onStop(self):
        self.releaseKeyboard()
        self.button.setText("Start")
        self.button.disconnect("clicked()", self.onStop)
        self.button.connect("clicked()", self.onStart)
        self.running = False
        self.mainWindow.disconnect("selectJointFromBodyName(QString)", self.changeSelected)

    def initWidget(self):
        mainBox = QVBoxLayout(self)

        self.button = QPushButton("Start", self)
        self.button.connect("clicked()", self.onStart)
        mainBox.addWidget(self.button)
        mainBox.addWidget(self.modeInstance.createWidget(self))

    def keyPressEvent(self, event):
        if (self.running):
            print event.key()
            if (event.key() == QNamespace.Key_M):
                self.mode = not self.mode
                if (self.mode == 0):
                    self.modeInstance = DualSelect(self.plugin, self.mainWindow)
                else:
                    self.modeInstance = RuleMaker(self.plugin, self.mainWindow)
            elif (event.key() == QNamespace.Key_H):
                print "Handle"
                self.modeInstance.addHandle(self.selected)
            elif (event.key() == QNamespace.Key_R):
                print "Gripper"
                self.modeInstance.addGripper(self.selected)
            elif (event.key() == QNamespace.Key_Shift):
                self.modeInstance.setShift()
            else:
                if (self.modeInstance.handleEvent(event.key())):
                    self.mainWindow.requestApplyCurrentConfiguration()

    def keyReleaseEvent(self, event):
        if (self.running):
            if (event.key() == QNamespace.Key_Shift):
                self.modeInstance.setShift()

class Plugin(QDockWidget):
    def __init__(self, mainWindow, flags = None):
        if (flags is not None):
            super(Plugin, self).__init__("Dynamic Builder", flags)
        else:
            super(Plugin, self).__init__("Dynamic Builder")
        self.resetConnection()
        self.dynamicBuilder = DynamicBuilder(mainWindow, self)
        self.setWidget(self.dynamicBuilder)

    def resetConnection(self):
        self.client = Clients()
