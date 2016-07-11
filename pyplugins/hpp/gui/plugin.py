from PythonQt import QtGui, Qt
from hpp.corbaserver import Client
from gepetto.corbaserver import Client as GuiClient
from directpath import DirectPathBox
from findGrasp import GraspFinder

class _PathTab(QtGui.QWidget):
    def __init__ (self, parent):
        super(_PathTab, self).__init__ (parent)
        self.plugin = parent
        box = QtGui.QVBoxLayout(self)

        # Create group
        box.addWidget(DirectPathBox(self, self.plugin))

class _ConcatenatePath(QtGui.QWidget):
    def __init__(self, parent):
        super(_ConcatenatePath, self).__init__(parent)
        self.plugin = parent

        box = QtGui.QVBoxLayout(self)
        
        box.addWidget(QtGui.QLabel("Choose the paths you want to concatenate.\n"
                                   "All the paths will be concatenate in the first choose."))
        box.addWidget(QtGui.QLabel("List of path:"))
        self.paths = QtGui.QListWidget()
        box.addWidget(self.paths)
        self.paths.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
        
        self.button = QtGui.QPushButton("Concatenate")
        box.addWidget(self.button)

        self.button.connect("clicked()", self.concatenate)

    def refresh(self):
        selected = self.paths.selectedItems()
        self.paths.clear()

        nbPaths = self.plugin.client.problem.numberPaths()
        for i in range(0, nbPaths):
            self.paths.addItem(str(i))
            if (len(selected) > 0):
                self.paths.setCurrentItem(selected, QtGui.QItemSelectionModel.Select)

    def concatenate(self):
        selected = self.paths.selectedItems()
        if (len(selected) > 1):
            first = int(selected[0].text())
            for i in range(1, len(selected)):
                print "Concatenate %s and %s" % (first, int(selected[i].text()))
                self.plugin.client.problem.concatenatePath(first, int(selected[i].text()))

class _RoadmapTab(QtGui.QWidget):
    def __init__ (self, parent):
        super(_RoadmapTab, self).__init__ (parent)
        self.plugin = parent
        box = QtGui.QGridLayout(self)

        box.addWidget(QtGui.QLabel("Number of nodes:"), 0, 0)
        self.nbNode = QtGui.QLabel()
        box.addWidget(self.nbNode, 0, 1)

        box.addWidget(QtGui.QLabel("Number of edges:"), 1, 0)
        self.nbEdge = QtGui.QLabel()
        box.addWidget(self.nbEdge, 1, 1)

        self.updateCB = QtGui.QCheckBox("Continuous update")
        box.addWidget(self.updateCB, 2, 2, 1, 2)
        self.updateCB.setTristate(False)

        self.timer = Qt.QTimer(self)
        self.timer.setInterval(500)
        self.timer.setSingleShot(False)

        self.timer.connect("timeout()", self.updateCount)
        self.updateCB.connect("stateChanged(int)", self.startStopTimer)

    def updateCount(self):
        try:
            self.nbNode.setNum (self.plugin.client.problem.numberNodes())
            self.nbEdge.setNum (self.plugin.client.problem.numberEdges())
        except Exception as e:
            self.plugin.main.logError (str(e))
            self.updateCB.setChecked(False)

    def startStopTimer(self, state):
        if state == Qt.Qt.Checked:
            self.timer.start()
        else:
            self.timer.stop()

class _StepByStepSolverTab(QtGui.QWidget):
    def __init__ (self, parent):
        super(_StepByStepSolverTab, self).__init__ (parent)
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
            self.plugin.main.log ("Problem is solved")

    def executeOneStep(self):
        for i in xrange(self.stepCount.value):
            if self.plugin.client.problem.executeOneStep():
                self.plugin.main.log ("Problem is solved")
                break

    def finishSolveStepByStep(self):
        self.plugin.client.problem.finishSolveStepByStep()

class Plugin(QtGui.QDockWidget):
    """ Extra HPP functionalities for the Gepetto Viewer GUI """
    def __init__ (self, mainWindow, flags = None):
        title = "HPP extra functionalities"
        if flags is None:
            super(Plugin, self).__init__ (title, mainWindow)
        else:
            super(Plugin, self).__init__ (title, mainWindow, flags)
        self.client = Client()
        self.gui = GuiClient()
        self.main = mainWindow
        self.osg = None
        # Initialize the widget
        self.tabWidget = QtGui.QTabWidget(self)
        self.setWidget (self.tabWidget)
        self.nodeCreator = _PathTab(self)
        self.concatenateWidget = _ConcatenatePath(self)
        self.tabWidget.addTab (self.nodeCreator, "Path")
        self.tabWidget.addTab (_RoadmapTab(self), "Roadmap")
        self.tabWidget.addTab (_StepByStepSolverTab(self), "Step by step solver")
        self.tabWidget.addTab (GraspFinder(self), "Grasp Finder")
        self.tabWidget.addTab (self.concatenateWidget, "Concatenate paths")

    def resetConnection(self):
        self.client = Client()
        self.gui = GuiClient()

    def osgWidget(self,osg):
        if self.osg is None:
            self.osg = osg

    def refreshInterface(self):
        self.concatenateWidget.refresh()
