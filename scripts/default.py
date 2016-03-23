from PythonQt.QtGui import *
from PythonQt.Qt import SLOT
from gepetto.corbaserver import Client
import sys

sys.argv = []

cl = Client()

# message is the name of the TO_COMPLETE that has been selected
def doStuff (message):
    print message

# connect doStuff function to the signal triggered 
# when clicking on an element inside osgViewer
osg.connect("selected(QString)", doStuff)
