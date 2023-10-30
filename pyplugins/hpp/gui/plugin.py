#
#  Copyright (c) CNRS
#  Author: Joseph Mirabel and Heidy Dallard
#

from __future__ import print_function

from gepetto.corbaserver import Client as GuiClient
from hpp.corbaserver import Client
from hpp.corbaserver.robot import Robot
from PythonQt import Qt, QtGui

from .collision_pairs import CollisionPairs
from .directpath import DirectPathBox
from .findGrasp import GraspFinder
from .inspector import InspectBodies
from .parameters import Parameters


class _PathTab(QtGui.QWidget):
    def __init__(self, parent):
        super(_PathTab, self).__init__(parent)
        self.plugin = parent
        box = QtGui.QVBoxLayout(self)

        # Create group
        box.addWidget(DirectPathBox(self, self.plugin))


class _PathManagement(QtGui.QWidget):
    def __init__(self, parent):
        super(_PathManagement, self).__init__(parent)
        self.plugin = parent
        parent.widgetToRefresh.append(self)

        box = QtGui.QVBoxLayout(self)

        button = QtGui.QPushButton("Get paths", self)
        button.connect("clicked()", self.refresh)
        box.addWidget(button)
        box.addWidget(
            QtGui.QLabel(
                "Choose the paths you want to concatenate.\n"
                "All the paths will be concatenate in the first choose."
            )
        )
        box.addWidget(QtGui.QLabel("List of path:"))
        self.paths = QtGui.QListWidget()
        box.addWidget(self.paths)
        self.paths.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)

        hBox = QtGui.QHBoxLayout()

        self.buttonConcatenate = QtGui.QPushButton("Concatenate")
        hBox.addWidget(self.buttonConcatenate)
        self.buttonConcatenate.connect("clicked()", self.concatenate)

        self.buttonErase = QtGui.QPushButton("Erase")
        hBox.addWidget(self.buttonErase)
        self.buttonErase.connect("clicked()", self.erase)

        box.addLayout(hBox)

    def refresh(self):
        self.paths.clear()

        nbPaths = self.plugin.client.problem.numberPaths()
        for i in range(0, nbPaths):
            self.paths.addItem(str(i))

    def concatenate(self):
        selected = self.paths.selectedItems()
        if len(selected) > 1:
            first = int(selected[0].text())
            for i in range(1, len(selected)):
                print("Concatenate %s and %s" % (first, int(selected[i].text())))
                self.plugin.client.problem.concatenatePath(
                    first, int(selected[i].text())
                )

    def erase(self):
        selected = self.paths.selectedItems()
        if len(selected) > 0:
            for i in range(len(selected) - 1, -1, -1):
                print("erase path %s " % (int(selected[i].text())))
                self.plugin.client.problem.erasePath(int(selected[i].text()))
            self.refresh()


class _RoadmapTab(QtGui.QWidget):
    def __init__(self, parent):
        super(_RoadmapTab, self).__init__(parent)
        self.plugin = parent
        box = QtGui.QGridLayout(self)

        box.addWidget(QtGui.QLabel("Number of nodes:"), 0, 0)
        self.nbNode = QtGui.QLabel()
        box.addWidget(self.nbNode, 0, 1)

        box.addWidget(QtGui.QLabel("Number of edges:"), 1, 0)
        self.nbEdge = QtGui.QLabel()
        box.addWidget(self.nbEdge, 1, 1)

        box.addWidget(QtGui.QLabel("Number of connected components :"), 2, 0)
        self.nbComponent = QtGui.QLabel()
        box.addWidget(self.nbComponent, 2, 1)

        self.updateCB = QtGui.QCheckBox("Continuous update")
        box.addWidget(self.updateCB, 3, 2, 1, 2)
        self.updateCB.setTristate(False)

        self.timer = Qt.QTimer(self)
        self.timer.setInterval(500)
        self.timer.setSingleShot(False)

        self.timer.connect("timeout()", self.updateCount)
        self.updateCB.connect("stateChanged(int)", self.startStopTimer)

    def updateCount(self):
        try:
            self.nbNode.setNum(self.plugin.client.problem.numberNodes())
            self.nbEdge.setNum(self.plugin.client.problem.numberEdges())
            self.nbComponent.setNum(
                self.plugin.client.problem.numberConnectedComponents()
            )
        except Exception as e:
            self.plugin.main.logError(str(e))
            self.updateCB.setChecked(False)

    def startStopTimer(self, state):
        if state == Qt.Qt.Checked:
            self.timer.start()
        else:
            self.timer.stop()


class _StepByStepSolverTab(QtGui.QWidget):
    def __init__(self, parent):
        super(_StepByStepSolverTab, self).__init__(parent)
        self.plugin = parent
        box = QtGui.QVBoxLayout(self)

        b = QtGui.QPushButton(self)
        b.text = "Initialize step by step sequence"
        box.addWidget(b)
        b.connect("clicked()", self.prepareSolveStepByStep)

        w = QtGui.QWidget(self)
        hl = QtGui.QHBoxLayout(w)
        self.stepCount = QtGui.QSpinBox(w)
        self.stepCount.setRange(1, 1000)
        self.value = 1
        hl.addWidget(self.stepCount)
        b = QtGui.QPushButton(self)
        b.text = "Execute N step"
        hl.addWidget(b)
        b.connect("clicked()", self.executeOneStep)
        box.addWidget(w)

        b = QtGui.QPushButton(self)
        b.text = "Finalize"
        box.addWidget(b)
        b.connect("clicked()", self.finishSolveStepByStep)

    def prepareSolveStepByStep(self):
        if self.plugin.client.problem.prepareSolveStepByStep():
            self.plugin.main.log("Problem is solved")

    def executeOneStep(self):
        for i in range(self.stepCount.value):
            if self.plugin.client.problem.executeOneStep():
                self.plugin.main.log("Problem is solved")
                break

    def finishSolveStepByStep(self):
        self.plugin.client.problem.finishSolveStepByStep()


class Plugin(QtGui.QDockWidget):
    """Extra HPP functionalities for the Gepetto Viewer GUI"""

    def __init__(self, mainWindow, flags=None):
        title = "HPP extra functionalities"
        if flags is None:
            super(Plugin, self).__init__(title, mainWindow)
        else:
            super(Plugin, self).__init__(title, mainWindow, flags)
        self.setObjectName("hpp.gui.plugin")
        self.main = mainWindow
        self.hppPlugin = self.main.getFromSlot("getHppIIOPurl")
        # self.resetConnection()
        self.widgetToRefresh = list()
        self.osg = None
        # Initialize the widget
        self.tabWidget = QtGui.QTabWidget(self)
        self.setWidget(self.tabWidget)
        self.tabWidget.addTab(_PathTab(self), "Path")
        self.tabWidget.addTab(_RoadmapTab(self), "Roadmap")
        self.tabWidget.addTab(_StepByStepSolverTab(self), "Step by step solver")
        self.tabWidget.addTab(GraspFinder(self), "Grasp Finder")
        self.tabWidget.addTab(_PathManagement(self), "Paths management")
        self.tabWidget.addTab(InspectBodies(self), "Inspector")
        self.tabWidget.addTab(CollisionPairs(self), "Collision pairs")
        self.tabWidget.addTab(Parameters(self), "Parameters")

    def resetConnection(self):
        self.client = Client(
            url=str(self.hppPlugin.getHppIIOPurl()),
            context=str(self.hppPlugin.getHppContext()),
        )
        self.resetRobot()
        self.gui = GuiClient()

    def resetRobot(self):
        try:
            self.robot = Robot(client=self.client)
        except Exception:
            self.robot = None

    def osgWidget(self, osg):
        if self.osg is None:
            self.osg = osg

    def refreshInterface(self):
        self.resetRobot()
        for w in self.widgetToRefresh:
            w.refresh()
