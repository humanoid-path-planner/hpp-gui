#
#  Copyright (c) CNRS
#  Author: Heidy Dallard
#

import re

from PythonQt.QtGui import (
    QCheckBox,
    QDockWidget,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QListWidget,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from hpp.corbaserver.manipulation import ConstraintGraph
from hpp.corbaserver.manipulation.robot import Robot


class GraphUtility(QWidget):
    def __init__(self, parent):
        super().__init__(parent)
        self.plugin = parent
        self.initWidget()
        self.edges = {}
        self.nodes = {}

    def updateNodes(self, status):
        if not status:
            self.nodes = {}
        else:
            self.nodes = self.cg.nodes
        self.applyFilters()

    def updateEdges(self, status):
        if not status:
            self.edges = {}
        else:
            self.edges = self.cg.edges
        self.applyFilters()

    def eraseInvalid(self, r, names):
        tempNames = names.copy()
        for n in tempNames:
            if r.match(n) is None:
                del names[n]
        return names

    def applyFilters(self):
        regex = ".*"
        if self.grippersList.currentItem() is not None:
            regex += self.grippersList.currentItem().text()
        else:
            regex += ".*"
        if self.graspCheck.isChecked():
            regex += " grasps "
            if self.handlesList.currentItem() is not None:
                regex += self.handlesList.currentItem().text() + ".*"
            else:
                regex += ".*"
        elif self.pregraspCheck.isChecked():
            if self.handlesList.currentItem() is not None:
                regex += (
                    "(?:<|>)" + self.handlesList.currentItem().text() + ".*pregrasp .*"
                )
            else:
                regex += ".*pregrasp.*"
        names = self.eraseInvalid(re.compile(regex), self.nodes.copy())
        self.resultList.clear()
        for n in names:
            self.resultList.addItem(n)

    def fillGripper(self):
        self.grippersList.addItems(
            self.plugin.r.client.manipulation.problem.getAvailable("gripper")
        )

    def fillHandles(self):
        self.handlesList.addItems(
            self.plugin.r.client.manipulation.problem.getAvailable("handle")
        )

    def applyRegex(self):
        names = self.eraseInvalid(
            re.compile(self.textEdit.text()), self.cg.nodes.copy()
        )
        self.resultList.clear()
        for n in names:
            self.resultList.addItem(n)

    def refresh(self):
        self.cg = ConstraintGraph(self.plugin.r, "", False)
        try:
            self.fillGripper()
            self.fillHandles()
            self.applyFilters()
        except Exception:
            pass

    def addConnection(self):
        self.graspCheck.connect("clicked()", self.applyFilters)
        self.pregraspCheck.connect("clicked()", self.applyFilters)
        self.graspCheck.connect("toggled(bool)", self.pregraspCheck.setDisabled)
        self.pregraspCheck.connect("toggled(bool)", self.graspCheck.setDisabled)
        self.grippersList.connect("itemSelectionChanged()", self.applyFilters)
        self.handlesList.connect("itemSelectionChanged()", self.applyFilters)

    def initWidget(self):
        # Main layout
        mainBox = QVBoxLayout(self)

        # Add text edit to write regex by hand
        box = QVBoxLayout()
        mainBox.addLayout(box)
        self.textEdit = QLineEdit(self)
        box.addWidget(QLabel("You can write your regexp here", self))
        box.addWidget(self.textEdit)
        button = QPushButton("Find", self)
        button.connect("clicked()", self.applyRegex)
        box.addWidget(button)

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

        # Add button to refresh
        button = QPushButton(self)
        button.setText("Refresh")
        button.connect("clicked()", self.refresh)
        mainBox.addWidget(button)

        # Get the grippers and handles names
        # Connect the widget to functions
        self.addConnection()


class Plugin(QDockWidget):
    def __init__(self, mainWindow, flags=None):
        if flags is None:
            super().__init__("Graph &utility", mainWindow)
        else:
            super().__init__("Graph &utility", mainWindow, flags)
        self.mainWindow = mainWindow
        self.graphUtility = GraphUtility(self)
        self.setWidget(self.graphUtility)
        self.resetConnection()

    def resetConnection(self):
        self.r = Robot("", "", "", False)

    def refreshInterface(self):
        print("refresh")
        self.graphUtility.refresh()
