"""
This example demonstrates many of the 2D plotting capabilities
in pyqtgraph. All of the plots may be panned/scaled by dragging with
the left/right mouse buttons. Right click on any plot to show a context menu.
"""

from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
import pyqtgraph.opengl as gl
from collections import deque
import SerialMonitor
import collections
from math import sin, cos
from numpy import array

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

def Rz(theta):
    return array([[cos(theta), -sin(theta),  0.0],
                  [sin(theta),  cos(theta),  0.0],
                  [0.0,         0.0,         1.0]])
def Ry(theta):
    return array([[cos(theta),  0.0,         sin(theta)],
                  [0.0,         1.0,         0.0],
                  [-sin(theta), 0.0,         cos(theta)]])
def Rx(theta):
    return array([[1.0,         0.0,         0.0],
                  [0.0,         cos(theta), -sin(theta)],
                  [0.0,         sin(theta),  cos(theta)]])

# a=0.;b=.3;c=0.;Rz(a).dot(Ry(b).dot(Rx(c).dot(array([1., 0., 0.]).T)))
#plt = gl.GLLinePlotItem(pos=pts, color=pg.glColor((i,n*1.3)), width=(i+1)/10., antialias=True)
#w.addItem(plt)
#a=gl.GLAxisItem(size=2)

app = None
win = None

PlottedVariableStructure = Struct('PlottedVariableStructure',
                                  'curveObj dataObj dataIdx color')
PlotStructure = Struct('PlotStructure',
                       'title xunits yunits plotObj variableObj')

ViewStructure = Struct('ViewStructure', 'title itemObj xIdx yIdx zIdx')

plotsRaw = []
plotsFiltered = []
views = []
data = [deque(np.random.normal(size=1000))]
ser = None
timer = None
plotLength = 600

def update():
    global data, plotsRaw, plotsFiltered, ser

    data = ser.readDataBinary(2, ['f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f'])

    for p in plotsRaw + plotsFiltered:
        for v in p.variableObj:
            v.dataObj.rotate(-1)
            #data[-1] = np.random.random() * 0.1
            v.dataObj[-1] = data[v.dataIdx]
            v.curveObj.setData(v.dataObj)

def gui():
    global app, win, plotsRaw, plotsFiltered, update, timer

    app = QtGui.QApplication([])

    win = pg.GraphicsWindow(title="Basic plotting examples")
    win.resize(1000,600)
    win.setWindowTitle('pyqtgraph example: Plotting')

    # Enable antialiasing for prettier plots
    pg.setConfigOptions(antialias=True)

    for plts in [plotsRaw, plotsFiltered]:
        for p in plts:
            p.plotObj = win.addPlot(title=p.title)
            for v in p.variableObj:
                v.curveObj = p.plotObj.plot(pen=v.color)
                #v.curveObj = p.plotObj.plot(pen=None, symbol='o', symbolSize=1)
            p.plotObj.enableAutoRange('xy', True)
        win.nextRow()

    timer = QtCore.QTimer()
    timer.timeout.connect(update)
    timer.start(50)

## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
    import sys
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        try:
            PlottedVariableStructure = Struct('PlottedVariableStructure', 'curveObj dataObj dataIdx color')

            plotsRaw.append(PlotStructure("Accelerometer",
                                       "time",
                                       "?",
                                       None,
                                       [PlottedVariableStructure(None,
                                                                 deque([float(0)] * plotLength),
                                                                 3,
                                                                 'y'),
                                        PlottedVariableStructure(None,
                                                                 deque([float(0)] * plotLength),
                                                                 4,
                                                                 'g'),
                                        PlottedVariableStructure(None,
                                                                 deque([float(0)] * plotLength),
                                                                 5,
                                                                 'm')]))

            plotsRaw.append(PlotStructure("Gyroscope",
                                       "time",
                                       "dps",
                                        None,
                                        [PlottedVariableStructure(None,
                                                                  deque([float(0)] * plotLength),
                                                                  0,
                                                                  'y'),
                                         PlottedVariableStructure(None,
                                                                  deque([float(0)] * plotLength),
                                                                  1,
                                                                  'g'),
                                         PlottedVariableStructure(None,
                                                                  deque([float(0)] * plotLength),
                                                                  2,
                                                                  'm')]))

            plotsRaw.append(PlotStructure("Magnetometer",
                                            "time",
                                            "?",
                                            None,
                                            [PlottedVariableStructure(None,
                                                                 deque([float(0)] * plotLength),
                                                                 6,
                                                                 'y'),
                                        PlottedVariableStructure(None,
                                                                 deque([float(0)] * plotLength),
                                                                 7,
                                                                 'g'),
                                        PlottedVariableStructure(None,
                                                                 deque([float(0)] * plotLength),
                                                                 8,
                                                                 'm')]))
            plotsFiltered.append(PlotStructure("Accelerometer Low Pass",
                                               "time",
                                               "?",
                                               None,
                                               [PlottedVariableStructure(None,
                                                                         deque([float(0)] * plotLength),
                                                                         9,
                                                                         'y'),
                                                PlottedVariableStructure(None,
                                                                         deque([float(0)] * plotLength),
                                                                         10,
                                                                         'g'),
                                                PlottedVariableStructure(None,
                                                                         deque([float(0)] * plotLength),
                                                                         11,
                                                                         'm')]))

            plotsFiltered.append(PlotStructure("Accelerometer Complementary",
                                               "time",
                                               "?",
                                               None,
                                               [PlottedVariableStructure(None,
                                                                         deque([float(0)] * plotLength),
                                                                         12,
                                                                         'y'),
                                                PlottedVariableStructure(None,
                                                                         deque([float(0)] * plotLength),
                                                                         13,
                                                                         'g'),
                                                PlottedVariableStructure(None,
                                                                         deque([float(0)] * plotLength),
                                                                         14,
                                                                         'm')]))

            views.append(ViewStructure("Aircraft Attitude",
                                       gl.GLGridItem(),
                                       12,
                                       13,
                                       14))

            ser=SerialMonitor.SerialMonitor()
            ser.open()
            gui()
            print('starting')
            QtGui.QApplication.instance().exec_()
            print('finished')
            ser.stop()
        except Exception as e:
            print(e)
            ser.stop()
