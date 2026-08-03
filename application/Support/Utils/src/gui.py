#import PySide
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
import pyqtgraph.opengl as gl
import numpy as np
import collections
from collections import deque
from math import sin, cos
from numpy import array

#--------------------------------------------------------------------------------
class GUI:
    """
    A pyqtgraph based class to laydown data plots and more...
    """
    #----------------------------------------------------------------------------
    @staticmethod
    def Struct(name, fields):
        """
        black magic copied from here, let's me define a structure as I
        understand them in C

        http://stackoverflow.com/questions/3648442/python-how-to-define-a-structure-like-in-c
        """
        fields = fields.split()
        def init(self, *values):
            for field, value in zip(fields, values):
                self.__dict__[field] = value
        cls = type(name, (object,), {'__init__': init})
        return cls

    #
    # structures used to define a plot vs time of many scalar variables
    #
    PlottedVariableStructure = Struct.__func__('PlottedVariableStructure',
                                               'curveObj dataObj dataIdx color label')
    PlotStructure = Struct.__func__('PlotStructure',
                                    'title xlayout ylayout xunits yunits plotObj variables')

    #
    # structure to draw a 3D view
    #
    ViewItemStructure = Struct.__func__('ViewItemStructure', 'itemObj xIdx yIdx zIdx')
    ViewStructure = Struct.__func__('ViewStructure', 'title xlayout ylayout viewObj elements')

    #----------------------------------------------------------------------------
    def __init__(self, items, func, period):
        """
        Initialize pyqtgraph stuff, populate plotted items from input parameter
        and setup timer to update plots periodically.
        items: list of data structure instances describing variables to plot.
        func: function to use to update values of plotted variables.
        period: update period in milliseconds.
        """
        #
        # checks if QApplication already exists and create QApplication only if
        # it doesnt exist (http://stackoverflow.com/a/10900523/1139967)
        #
        self.app = QtGui.QApplication.instance()
        if not self.app:
            self.app = QtGui.QApplication([])

        self.win = QtGui.QWidget()
        self.layout = QtGui.QGridLayout()
        self.win.setLayout(self.layout)

        # Enable antialiasing for prettier plots
        pg.setConfigOptions(antialias=True)

        # user function to update data
        self.updateFunc = func

        self.items = items

        for i in self.items:
            print('setting up item', i.title)
            if isinstance(i, self.__class__.PlotStructure):
                widgetObj = i.plotObj = pg.PlotWidget(title=i.title)
                i.plotObj.addLegend()
                for v in i.variables:
                    v.curveObj = widgetObj.plotItem.plot(pen=v.color, name=v.label)
                    #v.curveObj = p.plotObj.plot(pen=None, symbol='o', symbolSize=1)
                widgetObj.enableAutoRange('xy', True)
            elif isinstance(i, self.__class__.ViewStructure):
                widgetObj = i.viewObj = gl.GLViewWidget()
                widgetObj.setSizePolicy(pg.QtGui.QSizePolicy.Expanding, pg.QtGui.QSizePolicy.Expanding)
                for e in i.elements:
                    widgetObj.addItem(e.itemObj)
            else:
                raise Exception('unmatched item type')
            self.layout.addWidget(widgetObj, i.xlayout, i.ylayout)
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update)
        self.period = period

        # why this deque thing is better than otherwise?
        self.data = [deque(np.random.normal(size=1000))]

    #
    # read new data and update items with it
    #
    def update(self):
        from math import pi

        try:
            self.data = self.updateFunc()
        except Exception as ex:
            print('failed to update, continuing... ' + str(ex))
            return

        #print data[3], data[4], data[5]

        for i in self.items:
            if isinstance(i, self.__class__.PlotStructure):
                for v in i.variables:
                    v.dataObj.rotate(-1)
                    #data[-1] = np.random.random() * 0.1
                    v.dataObj[-1] = self.data[v.dataIdx]
                    v.curveObj.setData(v.dataObj)
            elif isinstance(i, self.__class__.ViewStructure):
                for e in i.elements:
                    if e.lastAttitude is not None:
                        e.itemObj.rotate((self.data[e.xIdx] - e.lastAttitude[0]) * 180. / pi, 1, 0, 0)
                        e.itemObj.rotate((self.data[e.yIdx] - e.lastAttitude[1]) * 180. / pi, 0, 1, 0)
                        e.itemObj.rotate((self.data[e.zIdx] - e.lastAttitude[2]) * 180. / pi, 0, 0, 1)
                    e.lastAttitude = [self.data[e.xIdx], self.data[e.yIdx], self.data[e.zIdx]]
            else:
                raise Exception('unexpected item type')

    #----------------------------------------------------------------------------
    def execute(self):
        print('gui has started')
        self.win.show()
        self.timer.start(self.period)
        QtGui.QApplication.instance().exec_()
        self.timer.stop()
        print('gui has finished')

#___oOo___
