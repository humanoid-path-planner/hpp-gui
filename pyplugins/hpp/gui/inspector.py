#
#  Copyright (c) CNRS
#  Author: Joseph Mirabel
#

from __future__ import print_function

from hpp import Transform
from numpy import array
from PythonQt import QtCore, QtGui


def vec(v):
    return array([v.x(), v.y(), v.z()])


def vec2str(v):
    return str(v[0]) + ", " + str(v[1]) + ", " + str(v[2])


class InspectBodies(QtGui.QWidget):
    def __init__(self, parent):
        super(InspectBodies, self).__init__(parent)
        self.plugin = parent

        self.initWidget()

    def initWidget(self):
        box = QtGui.QVBoxLayout(self)

        setRefButton = QtGui.QPushButton("Set reference frame", self)
        setRefButton.connect("clicked()", self.setReference)
        box.addWidget(setRefButton)
        self.layout().addWidget(QtGui.QLabel("Current reference frame", self))
        self.refName = QtGui.QLabel("", self)
        self.layout().addWidget(self.refName)

        self.pointLabel = dict()
        self.normalLabel = dict()
        for t in ["global", "local", "reference"]:
            self.pointLabel[t] = self.addInfo("Point", t)
            self.normalLabel[t] = self.addInfo("Normal", t)

    def showEvent(self, event):
        self.plugin.main.bodyTree().connect(
            "bodySelected(SelectionEvent*)", self.selected
        )

    def hideEvent(self, event):
        self.plugin.main.bodyTree().disconnect(
            "bodySelected(SelectionEvent*)", self.selected
        )

    def addInfo(self, what, where):
        label = QtGui.QLabel(what + " in " + where + " frame", self)
        self.layout().addWidget(label)
        text = QtGui.QLabel("TBD", self)
        text.textInteractionFlags = QtCore.Qt.TextBrowserInteraction
        self.layout().addWidget(text)
        return text

    def setReference(self):
        self.refName.text = str(
            self.plugin.main.getFromSlot("getSelectedJoint").getSelectedJoint()
        )

    def selected(self, event):
        if event.hasIntersection():
            self.pointLabel["local"].text = vec2str(vec(event.point(True)))
            self.normalLabel["local"].text = vec2str(vec(event.normal(True)))
            self.pointLabel["global"].text = vec2str(vec(event.point(False)))
            self.normalLabel["global"].text = vec2str(vec(event.normal(False)))
            if len(self.refName.text) == 0:
                print(self.refName.text)
            else:
                T = Transform(
                    self.plugin.client.robot.getJointPosition(str(self.refName.text))
                ).inverse()
                try:
                    self.pointLabel["reference"].text = vec2str(
                        T.transform(vec(event.point(False)))
                    )
                    self.normalLabel["reference"].text = vec2str(
                        T.quaternion.transform(vec(event.normal(False)))
                    )
                except ValueError as e:
                    print(e)
                    print(event.point(False))
                    print(vec(event.point(False)))
        event.done()
