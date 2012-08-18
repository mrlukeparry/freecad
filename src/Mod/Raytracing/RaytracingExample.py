# exampel how to use the scripting API of the drawing module
# 
# first of all you need the Part and the Drawing module:
import FreeCAD, Part, Drawing

# create a small sample part
Part.show(Part.makeBox(100,100,100).cut(Part.makeCylinder(80,100)).cut(Part.makeBox(90,40,100)).cut(Part.makeBox(20,85,100)))

# TODO create an example using python

