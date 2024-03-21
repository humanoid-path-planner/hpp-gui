#
#  Copyright (c) CNRS
#  Author: Joseph Mirabel
#


import numpy as np
from gepetto import Color, Quaternion
from PythonQt import QtGui

z = np.array([0, 0, 1])
I3 = np.identity(3)


def _angleAxisToQuat(u, theta):
    q = [
        np.sin(theta / 2),
    ]
    q[1:4] = np.cos(theta / 2) * u
    return q


def _angleAxisToRotationMatrix(u, theta):
    cross = np.zeros((3, 3))
    cross[0, 1] = -u[2]
    cross[1, 0] = u[2]
    cross[0, 2] = u[1]
    cross[2, 0] = -u[1]
    cross[2, 1] = u[0]
    cross[1, 2] = -u[0]
    ct = np.cos(theta)
    R = (
        ct * I3
        + np.sin(theta) * cross
        + (1 - ct) * u.reshape((3, 1)).dot(u.reshape((1, 3)))
    )
    return R


class GraspFinder(QtGui.QWidget):
    groupName = "hpp-gui/findGrasp"

    def __init__(self, parent):
        super().__init__(parent)
        self.plugin = parent
        box = QtGui.QVBoxLayout(self)

        # Create group
        self.instructions = QtGui.QLabel("Click on Run")
        box.addWidget(self.instructions)
        runButton = self.makeButton("Run", True)
        runButton.connect("toggled(bool)", self.run)
        box.addWidget(runButton)
        optimize = self.makeButton("Optimize", False)
        optimize.connect("clicked()", self.optimize)
        box.addWidget(optimize)

        self.P = []
        self.Q = []
        self.names = []

    def makeButton(self, name, check):
        b = QtGui.QPushButton(self)
        b.text = name
        b.checkable = check
        return b

    def run(self, checked):
        if checked:
            self.instructions.text = "Select a fixed point."
            self.P = []
            self.Q = []
            self.names = []
            # self.P = [
            # (1.0217747688293457, 0.3965822756290436, -0.01692081056535244),
            # (1.0248892307281494, 0.4981599450111389, 0.019463790580630302),
            # ]
            # self.Q = [
            # (0.8140766620635986, 0.31978103518486023, -0.008532078936696053),
            # (0.9297375082969666, 0.29411521553993225, -0.003007239894941449),
            # ]
            # self.names = [""]
            self.plugin.gui.gui.createGroup(self.groupName)
            self.plugin.osg.connect("clicked(QString,QVector3D)", self.selected)
        else:
            self.plugin.osg.disconnect("clicked(QString,QVector3D)", self.selected)
            self.P = []
            self.Q = []
            self.names = []
            self.plugin.gui.gui.deleteNode(self.groupName, True)

    def optimize(self):
        n = len(self.P)
        if not n == len(self.Q):
            print("P and Q must be of the same size")
            return
        P = np.array(self.P).transpose()
        Q = np.array(self.Q).transpose()
        centroidP = P.mean(axis=1).reshape((3, 1)).repeat(n, axis=1)
        centroidQ = Q.mean(axis=1).reshape((3, 1)).repeat(n, axis=1)
        H = (P - centroidP).dot((Q - centroidQ).transpose())
        svd = np.linalg.svd(H)
        R = svd[2].dot(svd[0].transpose())
        if np.linalg.det(R) < 0:
            R[:, 2] *= -1
        t = centroidQ[:, 0] - R.dot(centroidP[:, 0])
        T = [0, 0, 0, 1, 0, 0, 0]
        T[0:3] = t.tolist()
        T[3:7] = Quaternion(R).toTuple()
        print(self.names[0], T)
        self.instructions.text = (
            "Current transform of\n"
            + self.names[0]
            + "\nis\n"
            + str(T)
            + "\n\nSelect a fixed point."
        )
        self.plugin.gui.gui.applyConfiguration(self.names[0], T)
        self.plugin.gui.gui.refresh()
        for i in range(n):
            self.setLineTransform(i)
        self.plugin.gui.gui.refresh()

    def selected(self, name, posInWorldFrame):
        v = (posInWorldFrame.x(), posInWorldFrame.y(), posInWorldFrame.z())
        if len(self.P) == len(self.Q):
            self.Q.append(v)
            inst = "Select a moving point."
        else:
            self.names.append(str(name))
            # Compute local coordinate
            T = self.plugin.gui.gui.getNodeGlobalTransform(str(name))
            R = Quaternion(T[3:7]).toRotationMatrix()
            t = np.array(T[0:3])
            self.P.append(R.transpose().dot(np.array(v) - t))
            index = len(self.P) - 1
            self.setLineTransform(index, True)
            self.plugin.gui.gui.refresh()
            inst = "Select a fixed point."
        text = inst + "\nNb selected points: " + str(len(self.P))
        self.instructions.text = text

    def setLineTransform(self, index, new=False):
        name = self.groupName + "/pointPair_" + str(index)
        if not new:
            self.plugin.gui.gui.deleteNode(name, False)
        LFname = self.names[index]
        P = np.array(self.P[index])  # Moving
        Q = np.array(self.Q[index])  # Fixed
        T = self.plugin.gui.gui.getNodeGlobalTransform(LFname)
        R = Quaternion(T[3:7]).toRotationMatrix()
        t = np.array(T[0:3])
        Pwf = R.dot(P) + t
        self.plugin.gui.gui.addLine(name, Pwf.tolist(), Q.tolist(), Color.yellow)
