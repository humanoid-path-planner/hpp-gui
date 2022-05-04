#
#  Copyright (c) CNRS
#  Author: Joseph Mirabel
#

from PythonQt import QtGui, Qt
import xml.etree.ElementTree as ET

import re


def _makeCheckBox(active):
    item = QtGui.QCheckBox()
    item.checked = active
    return item


def _makeTableWidgetItem(lbl, checkable, editable):
    item = QtGui.QTableWidgetItem(lbl)
    f = Qt.Qt.ItemIsEnabled | Qt.Qt.ItemIsSelectable
    if editable:
        f = f | Qt.Qt.ItemIsEditable
    if checkable:
        f = f | Qt.Qt.ItemIsUserCheckable
    item.setFlags(f)
    return item


def _bodyNameToUrdfLinkName(n):
    return re.sub("_[0-9]+$", "", n)


class _Pair:
    def __init__(self, l1, l2):
        self.l1 = l1
        self.l2 = l2
        if l1 > l2:
            self.key = (l1, l2)
        else:
            self.key = (l2, l1)
        self.hash = hash(self.key)

    def __eq__(x, y):
        return x.key == y.key

    def __hash__(self):
        return self.hash


class TableWidget(QtGui.QTableWidget):
    def keyPressEvent(self, event):
        if event.key() == Qt.Qt.Key_Space:
            lastRow = -1
            print("Selecting items")
            for item in self.selectedItems():
                if lastRow != item.row():
                    self.cellWidget(item.row(), CollisionPairs.ACTIVE).toggle()
                    lastRow = item.row()
        else:
            QtGui.QTableWidget.keyPressEvent(self, event)


class CollisionPairs(QtGui.QWidget):
    ACTIVE = 0
    LINK_1 = 1
    LINK_2 = 2
    REASON = 3
    CURRENT_CONFIG = 4
    PERCENTAGE = 5

    def __init__(self, parent):
        super(CollisionPairs, self).__init__(parent)
        self.plugin = parent
        # parent.widgetToRefresh.append(self)
        self.orderedPairs = list()
        self.pairToRow = dict()
        box = QtGui.QVBoxLayout(self)

        button = QtGui.QPushButton(
            "Toggle between collision and visual robot bodies", self
        )
        button.checkable = True
        button.connect("clicked(bool)", self.toggleVisual)
        box.addWidget(button)

        button = QtGui.QPushButton(
            QtGui.QIcon.fromTheme("view-refresh"), "Refresh list", self
        )
        button.connect("clicked()", self.refresh)
        box.addWidget(button)

        # Create table
        self.table = TableWidget(0, 6)
        self.table.setHorizontalHeaderLabels(
            [
                "Active",
                "Link 1",
                "Link 2",
                "Reason",
                "Current configuration",
                "% of collision",
            ]
        )
        if Qt.qVersion().startswith("4"):
            self.table.horizontalHeader().setResizeMode(QtGui.QHeaderView.Interactive)
        else:
            self.table.horizontalHeader().setSectionResizeMode(
                QtGui.QHeaderView.Interactive
            )
        self.table.selectionBehavior = QtGui.QAbstractItemView.SelectRows
        self.table.connect(
            "currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)",
            self.currentItemChanged,
        )
        box.addWidget(self.table)

        # "Number of random configuration (log scale)"
        self.sliderRandomCfg = QtGui.QSlider(Qt.Qt.Horizontal, self)
        self.sliderRandomCfg.setRange(20, 60)
        self.sliderRandomCfg.setValue(30)
        box.addWidget(self.sliderRandomCfg)
        button = QtGui.QPushButton("Compute percentage of collision", self)
        button.connect("clicked()", self.computePercentageOfCollision)
        box.addWidget(button)

        button = QtGui.QPushButton("Save to file...", self)
        button.connect("clicked()", self.writeToFile)
        box.addWidget(button)

        obj = self.plugin.main.getFromSlot("getHppIIOPurl")
        if obj is not None:
            obj.connect(
                "configurationValidationStatus(QStringList)",
                self.currentBodyInCollisions,
            )
        else:
            print("Could not find obj")

    def showEvent(self, event):
        self.plugin.main.connect("configurationValidation()", self.configValidation)

    def hideEvent(self, event):
        self.plugin.main.disconnect("configurationValidation()", self.configValidation)

    def statusChanged(self, row, checked):
        row[self.REASON].setText("User defined")
        self.plugin.client.robot.setAutoCollision(
            str(row[self.LINK_1].text()), str(row[self.LINK_2].text()), checked
        )

    def currentItemChanged(self, current, previous):
        if current is not None and (current.column() == 1 or current.column() == 2):
            self.plugin.main.bodyTree().selectBodyByName(
                self.robotName + "/" + current.text()
            )

    def configValidation(self):
        collide = self.plugin.client.robot.autocollisionCheck()
        for p, c in zip(self.orderedPairs, collide):
            if c:
                self.pairToRow[p][self.CURRENT_CONFIG].setText("Collision")
            else:
                self.pairToRow[p][self.CURRENT_CONFIG].setText("Free")

    def setCollisionPair(self, r, l1, l2, active, reason):
        pair = _Pair(l1, l2)
        if pair in self.pairToRow:
            self.pairToRow[pair][self.ACTIVE].checked = active
            self.pairToRow[pair][self.REASON].setText(reason)
            return
        row = (
            _makeCheckBox(active),
            _makeTableWidgetItem(l1, True, False),
            _makeTableWidgetItem(l2, False, False),
            _makeTableWidgetItem(reason, False, True),
            _makeTableWidgetItem("NA", False, False),
            _makeTableWidgetItem("NA", False, False),
        )
        self.table.setCellWidget(r, self.ACTIVE, row[self.ACTIVE])
        self.table.setItem(r, self.LINK_1, row[self.LINK_1])
        self.table.setItem(r, self.LINK_2, row[self.LINK_2])
        self.table.setItem(r, self.REASON, row[self.REASON])
        self.table.setItem(r, self.CURRENT_CONFIG, row[self.CURRENT_CONFIG])
        self.table.setItem(r, self.PERCENTAGE, row[self.PERCENTAGE])
        row[self.ACTIVE].connect("toggled(bool)", lambda c: self.statusChanged(row, c))
        self.pairToRow[pair] = row
        self.orderedPairs.append(pair)

    def currentBodyInCollisions(self, bodies):
        self.bodies = bodies

    def computePercentageOfCollision(self):
        from math import pow

        ncfg = int(pow(10, self.sliderRandomCfg.value / 10))
        nbCol = [0] * len(self.pairToRow)
        r = self.plugin.client.robot
        for i in range(ncfg):
            cfg = r.shootRandomConfig()
            r.setCurrentConfig(cfg)
            collide = r.autocollisionCheck()
            for i in range(len(collide)):
                if collide[i]:
                    nbCol[i] += 1

        for p, n in zip(self.orderedPairs, nbCol):
            self.pairToRow[p][self.PERCENTAGE].setData(
                Qt.Qt.DisplayRole, n * 100.0 / ncfg
            )

    def refresh(self):
        self.pairToRow.clear()
        self.orderedPairs = list()
        import time

        start_time = time.time()
        self.robotName = self.plugin.client.robot.getRobotName()
        inner, outer, active = self.plugin.client.robot.autocollisionPairs()
        self.table.sortingEnabled = False
        self.table.setRowCount(len(inner))
        for r, l1, l2, a in zip(range(len(inner)), inner, outer, active):
            self.setCollisionPair(r, l1, l2, a, "From SRDF")
        self.table.sortingEnabled = True
        print(time.time() - start_time)

    def toggleVisual(self, visual):
        rn = self.plugin.client.robot.getRobotName()
        for n in self.plugin.gui.gui.getGroupNodeList(rn):
            self.plugin.gui.gui.setBoolProperty(n, "ShowVisual", visual)

    def writeToFile(self):
        _ = _bodyNameToUrdfLinkName
        filename = QtGui.QFileDialog.getSaveFileName(
            self, "SRDF file", "", "SRDF files (*.srdf)"
        )
        file = Qt.QFileInfo(filename)
        if file.exists():
            tree = ET.parse(filename)
        else:
            tree = ET.ElementTree(ET.Element("robot", {"name": "name"}))
            tree.getroot().append(ET.Comment("Generated by hpp-gui Python plugin"))

        robot = tree.getroot()
        pairsInFile = dict()
        for dc in robot.findall("disable_collisions"):
            p = _Pair(dc.attrib["link1"], dc.attrib["link2"])
            pairsInFile[p] = dc

        for p, row in self.pairToRow.items():
            if row[self.ACTIVE].checked:
                continue
            reason = str(row[self.REASON].text())
            pp = _Pair(_(p.l1), _(p.l2))
            if pp in pairsInFile:
                dc.attrib["reason"] = reason
            else:
                attrib = dict()
                attrib["link1"] = pp.l1
                attrib["link2"] = pp.l2
                attrib["reason"] = reason
                el = ET.Element("disable_collisions", attrib=attrib)
                if len(robot.getchildren()) > 0:
                    robot.getchildren()[-1].tail = "\n  "
                robot.append(el)

        tree.write(filename)
