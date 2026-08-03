from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
import pyqtgraph.opengl as gl
import numpy as np
import collections
from collections import deque
from math import sin, cos
from numpy import array
from bootstrap import *

#
# black magic copied from here
# http://stackoverflow.com/questions/3648442/python-how-to-define-a-structure-like-in-c
#
def Struct(name, fields):
    fields = fields.split()
    def init(self, *values):
        for field, value in zip(fields, values):
            self.__dict__[field] = value
    cls = type(name, (object,), {'__init__': init})
    return cls

#
# structures used to define a plot vs time of many scalar variables
#
PlottedVariableStructure = Struct('PlottedVariableStructure',
                                  'curveObj dataObj dataIdx color')
PlotStructure = Struct('PlotStructure',
                       'title xlayout ylayout xunits yunits plotObj variables')

#
# structure to draw a 3D view
#
ViewItemStructure = Struct('ViewItemStructure', 'itemObj xIdx yIdx zIdx')
ViewStructure = Struct('ViewStructure', 'title xlayout ylayout viewObj elements')

#
# global variables
#
app = None
win = None
items = []
plotsRaw = []
plotsFiltered = []
views = []
data = [deque(np.random.normal(size=1000))]
ac = None
timer = None
period = 100
plotLength = 600
isDataInitialized = False
roll = 0
pitch = 0
yaw = 0

#
# items to create
#
items.append(
    PlotStructure(
        "Accelerometer",
        0, 0,
        "time",
        "?",
        None,
        [PlottedVariableStructure(
            None,
            deque([float(0)] * plotLength),
            3,
            'y'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             4,
             'g'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             5,
             'm')]))

items.append(
    PlotStructure(
        "Gyroscope",
        0, 1,
        "time",
        "dps",
        None,
        [PlottedVariableStructure(
            None,
            deque([float(0)] * plotLength),
            0,
            'y'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             1,
             'g'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             2,
             'm')]))

items.append(
    PlotStructure(
        "Gyroscope Integrated",
        1, 1,
        "time",
        "dps",
        None,
        [PlottedVariableStructure(
            None,
            deque([float(0)] * plotLength),
            15,
            'y'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             16,
             'g'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             17,
             'm')]))

items.append(
    PlotStructure(
        "Magnetometer",
        0, 2,
        "time",
        "?",
        None,
        [PlottedVariableStructure(
            None,
            deque([float(0)] * plotLength),
            6,
            'y'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             7,
             'g'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             8,
             'm')]))

items.append(
    PlotStructure(
        "Accelerometer Low Pass",
        1, 0,
        "time",
        "?",
        None,
        [PlottedVariableStructure(
            None,
            deque([float(0)] * plotLength),
            9,
            'y'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             10,
             'g'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             11,
             'm')]))

items.append(
    PlotStructure(
        "Accelerometer Complementary",
        2, 0,
        "time",
        "?",
        None,
        [PlottedVariableStructure(
            None,
            deque([float(0)] * plotLength),
            12,
            'y'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             13,
             'g'),
         PlottedVariableStructure(
             None,
             deque([float(0)] * plotLength),
             14,
             'm')]))

items.append(
    ViewStructure(
        "Attitude",
        1, 2,
        None,
        [ViewItemStructure(gl.GLGridItem(), 0, 1, 2)]))

#
# read new data and update items with it
#
def update():
    from math import pi
    global data, items, ac, isDataInitialized, roll, pitch, yaw

    data = ac.READOUT()

    #print data[3], data[4], data[5]

    for i in items:
        if isinstance(i, PlotStructure):
            for v in i.variables:
                v.dataObj.rotate(-1)
                #data[-1] = np.random.random() * 0.1
                v.dataObj[-1] = data[v.dataIdx]
                v.curveObj.setData(v.dataObj)
        elif isinstance(i, ViewStructure):
            for e in i.elements:
                if isDataInitialized:
                    e.itemObj.rotate((data[12] - roll) * 180. / pi, 1, 0, 0)
                    e.itemObj.rotate((data[13] - pitch) * 180. / pi, 0, 1, 0)
                    e.itemObj.rotate((data[14] - yaw) * 180. / pi, 0, 0, 1)
                    roll = data[12]
                    pitch = data[13]
                    yaw = data[14]
        else:
            raise Exception('unexpected item type')

    isDataInitialized = True

def gui():
    global app, win, items, update, timer, period

    app = QtGui.QApplication([])
    win = QtGui.QWidget()
    layout = QtGui.QGridLayout()
    win.setLayout(layout)

    # Enable antialiasing for prettier plots
    pg.setConfigOptions(antialias=True)

    for i in items:
        if isinstance(i, PlotStructure):
            widgetObj = i.plotObj = pg.PlotWidget(title=i.title)
            for v in i.variables:
                v.curveObj = widgetObj.plotItem.plot(pen=v.color)
                #v.curveObj = p.plotObj.plot(pen=None, symbol='o', symbolSize=1)
            widgetObj.enableAutoRange('xy', True)
        elif isinstance(i, ViewStructure):
            widgetObj = i.viewObj = gl.GLViewWidget()
            widgetObj.setSizePolicy(pg.QtGui.QSizePolicy.Expanding, pg.QtGui.QSizePolicy.Expanding)
            for e in i.elements:
                widgetObj.addItem(e.itemObj)
        else:
            raise Exception('unmatched item type')
        layout.addWidget(widgetObj, i.xlayout, i.ylayout)
    win.show()
    timer = QtCore.QTimer()
    timer.timeout.connect(update)
    timer.start(period)

if __name__ == '__main__':
    import sys
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        try:
            ac=MPU6050('/dev/tm4c123gh6pm')
            gui()
            print('starting')
            QtGui.QApplication.instance().exec_()
            print('finished')
            ac.close()
        except Exception as e:
            print(e)
            ac.close()
