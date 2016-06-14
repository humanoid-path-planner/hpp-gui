from PythonQt.QtGui import QDockWidget, QWidget, QVBoxLayout, QHBoxLayout, QCheckBox, QListWidget
from hpp.corbaserver.manipulation.robot import Robot
from hpp.corbaserver.manipulation import ConstraintGraph
import re

class GraphUtility (QWidget):
    def __init__(self, parent):
        super(GraphUtility, self).__init__(parent)
        self.r = Robot("", "", "", False)
        self.initWidget()
        self.edges = {}
        self.nodes = {}

    def updateNodes(self, status):
        if (status == False):
            self.nodes = {}
        else:
            self.nodes = self.cg.nodes
        self.applyFilters()

    def updateEdges(self, status):
        if (status == False):
            self.edges = {}
        else:
            self.edges = self.cg.edges
        self.applyFilters()

    def eraseInvalid(self, r, names):
        tempNames = names.copy()
        for n in tempNames:
            if (r.match(n) is None):
                del names[n]
        return names

    def applyFilters(self):
        names = self.edges.copy()
        names.update(self.nodes)
        if (self.grippersList.currentItem() is not None):
            r = re.compile(r".*"+self.grippersList.currentItem().text()+".*")
            names = self.eraseInvalid(r, names)
        if (self.graspCheck.isChecked() == True):
            r = re.compile(r".*(?:>|grasps).*")
            names = self.eraseInvalid(r, names)
        elif (self.pregraspCheck.isChecked() == True):
            r = re.compile(r".*(?:<|pregrasp).*")
            names = self.eraseInvalid(r, names)
        if (self.handlesList.currentItem() is not None):
            r = re.compile(r".*"+self.handlesList.currentItem().text()+".*")
            names = self.eraseInvalid(r, names)
        self.resultList.clear()
        for n in names:
            self.resultList.addItem(n)

    def fillGripper(self):
        self.grippersList.addItems(self.r.client.manipulation.problem.getAvailable("gripper"))

    def fillHandles(self):
        self.handlesList.addItems(self.r.client.manipulation.problem.getAvailable("handle"))

    def refresh(self):
        self.cg = ConstraintGraph(self.r, "", False)
        try:
            self.fillGripper()
            self.fillHandles()
            self.applyFilters()
        except Exception as e:
            pass

    def addConnection(self):
        self.edgeCheck.connect("toggled(bool)", self.updateEdges)
        self.nodeCheck.connect("toggled(bool)", self.updateNodes)
        self.graspCheck.connect("clicked()", self.applyFilters)
        self.pregraspCheck.connect("clicked()", self.applyFilters)
        self.grippersList.connect("itemSelectionChanged()", self.applyFilters)
        self.handlesList.connect("itemSelectionChanged()", self.applyFilters)

    def initWidget(self):
        # Main layout
        mainBox = QVBoxLayout(self)

        # Sub layout for the graph type element
        typeBox = QHBoxLayout()
        mainBox.addLayout(typeBox)
        self.edgeCheck = QCheckBox("Edges", self)
        typeBox.addWidget(self.edgeCheck)
        self.nodeCheck = QCheckBox("Nodes", self)
        typeBox.addWidget(self.nodeCheck)

        # Add grippers list
        self.grippersList = QListWidget(self)
        mainBox.addWidget(self.grippersList)

        # Sub layout for the action type
        box = QHBoxLayout()
        mainBox.addLayout(box)
        self.graspCheck = QCheckBox("Grasp", self)
        box.addWidget(self.graspCheck)
        self.pregraspCheck = QCheckBox("Pregrasp", self)
        box.addWidget(self.pregraspCheck)

        # Add handle list
        self.handlesList = QListWidget(self)
        mainBox.addWidget(self.handlesList)

        # Add a table to display results
        self.resultList = QListWidget(self)
        mainBox.addWidget(self.resultList)

        # Get the grippers and handles names
        # Connect the widget to functions
        self.refresh()
        self.addConnection()

class Plugin(QDockWidget):
    def __init__(self, mainWindow, flags = None):
        if (flags == None):
            super(Plugin, self).__init__("Graph &utility", mainWindow)
        else:
            super(Plugin, self).__init__("Graph &utility", mainWindow, flags)
        self.mainWindow = mainWindow
        self.graphUtility = GraphUtility(self)
        self.setWidget(self.graphUtility)
        mainWindow.connect("refresh()", self.graphUtility.refresh)
