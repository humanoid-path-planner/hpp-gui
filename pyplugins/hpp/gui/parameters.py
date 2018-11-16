from PythonQt import QtGui, Qt, QtCore

_NAME=0
_VALUE=1
_TYPE=2
_DOCUMENTATION=3

class Parameters(QtGui.QWidget):
    def __init__(self, plugin):
        super(Parameters, self).__init__(plugin)
        self.plugin = plugin

        box = QtGui.QVBoxLayout(self)

        # Button to refresh
        button = QtGui.QPushButton(QtGui.QIcon.fromTheme("view-refresh"), "Refresh list", self)
        button.connect("clicked()", self.refresh)
        box.addWidget(button)

        # Table view to show the parameters
        self.tableWidget = QtGui.QTableWidget ()
        #self.tableWidget.setColumnCount(3)
        #self.tableWidget.setHorizontalHeaderLabels(("Name", "Description", "Value"))
        self.tableWidget.setColumnCount(3)
        self.tableWidget.setHorizontalHeaderLabels(("Name", "Value", "Type"))
        box.addWidget (self.tableWidget)

    def refresh(self):
        defaultParams = self.plugin.client.problem.getAvailable('defaultparameter')
        params        = self.plugin.client.problem.getAvailable('parameter')
        self.tableWidget.setRowCount (len(defaultParams))
        for i, p in enumerate(defaultParams):
            pdoc = self.plugin.client.problem.getParameterDoc(p)
            pval = self.plugin.client.problem.getParameter(p)
            if self.tableWidget.item(i, _NAME) is None:
                # initialize the row
                self.tableWidget.setItem(i, _NAME         , QtGui.QTableWidgetItem (p))
                #self.tableWidget.setItem(i, _DOCUMENTATION, QtGui.QTableWidgetItem (pdoc))
                self.tableWidget.setItem(i, _VALUE        , QtGui.QTableWidgetItem (str(pval.value())))
                self.tableWidget.setItem(i, _TYPE         , QtGui.QTableWidgetItem (str(pval.typecode())))
            else:
                self.tableWidget.item(i, _NAME         ).setText (p)
                #self.tableWidget.item(i, _DOCUMENTATION).text = pdoc
                self.tableWidget.item(i, _VALUE        ).setText (str(pval.value()))
                self.tableWidget.item(i, _TYPE         ).setText (str(pval.typecode()))
            self.tableWidget.item(i, _NAME         ).setToolTip (pdoc)
