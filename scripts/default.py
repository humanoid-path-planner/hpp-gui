from PythonQt.QtGui import *
from PythonQt.Qt import SLOT
from gepetto.corbaserver import Client
import sys

sys.argv = []

#cl = Client()

# message is the name of the body that has been selected
# have to explicitly convert message to string to be passed to cl functions
def doStuff (message):
    cl.gui.addLandmark(str(message), 0.5)

# connect doStuff function to the signal triggered 
# when clicking on an element inside osgViewer
osg.connect("selected(QString)", doStuff)
