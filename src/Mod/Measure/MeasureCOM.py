#***************************************************************************
#*   Copyright (c) 2014 Juergen Riegel <FreeCAD@juergen-riegel.net>        *
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU Lesser General Public License (LGPL)    *
#*   as published by the Free Software Foundation; either version 2 of     *
#*   the License, or (at your option) any later version.                   *
#*   for detail see the LICENCE text file.                                 *
#*                                                                         *
#*   This program is distributed in the hope that it will be useful,       *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU Library General Public License for more details.                  *
#*                                                                         *
#*   You should have received a copy of the GNU Library General Public     *
#*   License along with this program; if not, write to the Free Software   *
#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
#*   USA                                                                   *
#*                                                                         *
#***************************************************************************

import FreeCAD
from FreeCAD import Units, Placement
from UtilsMeasure import MeasureBasePython
from PySide.QtCore import QT_TRANSLATE_NOOP



__title__="Measure Center of Mass Object"
__author__ = "David Friedli"
__url__ = "http://www.freecad.org"



""" 
    The Measure cpp object defines a result and a placement property. The Python measure type
    adds it's own specific properties. Once the object is recomputed the parent properties are updated
    based on the specific python properties. 

    We'll need some kind of interface for the measure command which exposes "parseSelection", "isValidSelection" etc.

"""



def makeMeasureCOM(name="CenterOfMass"):
    '''makeMeasureCOM(name): make a CenterofMass measurement'''
    obj = FreeCAD.ActiveDocument.addObject("Measure::MeasurePython", name)
    MeasureCOM(obj)
    return obj


class MeasureCOM(MeasureBasePython):
    "The MeasureCOM object"
    
    def __init__(self, obj):
        obj.Proxy = self
        self.measureObject = obj

        obj.addProperty("App::PropertyLinkSub", "Element", "", QT_TRANSLATE_NOOP("App::PropertyLinkSub", "Element to measure"))
        obj.addProperty("App::PropertyPosition", "Result", "", QT_TRANSLATE_NOOP("App::PropertyVector", "The result location"))

    @classmethod
    def isValidSelection(cls, selection):
        return bool(len(selection))

    @classmethod
    def isPrioritySelection(cls, selection):
        return True

    def parseSelection(self, selection):
        sel = selection[0]
        o = FreeCAD.ActiveDocument.getObject(sel[0])
        self.measureObject.Element = (o, sel[1])

    def getResultString(self):
        values = [Units.Quantity(v, Units.Length).getUserPreferred()[0] for v in self.measureObject.Result]
        return "\n".join(values)

    def execute(self, obj):
        element = obj.Element
        if not element:
            return

        ob, subName = element
        if not hasattr(ob, "Shape"):
            return
        
        shape = ob.Shape
        if not hasattr(shape, "CenterOfMass"):
            return

        com = shape.CenterOfMass
        obj.Result = com

        placement = Placement()
        placement.Base = com
        obj.Placement = placement

    def onChanged(self, obj, prop):
        '''Do something when a property has changed'''

        if prop == "Element":
            self.execute(obj)

    def __getstate__(self):
        """When saving the document this object gets stored using Python's json module.\
                Since we have some un-serializable parts here -- the pointer to the measure feature "measureObject" -- we must define this method\
                to return a tuple of all serializable objects or None."""
        return None

    def __setstate__(self, state):
        """When restoring the serialized object from document we have the chance to set some internals here.\
                Since no data were serialized nothing needs to be done here."""
        return None


    def getPlacement(self):
        plm = Placement()
        if not self.measureObject:
            return plm
        plm.Base = self.measureObject.Result
        return plm

