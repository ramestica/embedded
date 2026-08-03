from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time
from numpy import *
from socket import *
import time

app = QtGui.QApplication([])

plot_param1 = [0,2,4,6,8,10,12,14,16,18];
plot_param2 = [0,3,6,9,12,15,18,21,24,27];

samplesize = 10;
samples     = range(0,samplesize)
framecount = 0;

pg.mkQApp()
pw = pg.PlotWidget()
pw.show()
p1 = pw.plotItem
p2 = pg.ViewBox()
p1.showAxis('right')
p1.scene().addItem(p2)
p2.setGeometry(p1.vb.sceneBoundingRect())
p1.getAxis('right').linkToView(p2)
p2.setXLink(p1)

def update():
    global pw, pg, framecount, plot_param1, plot_param2, p1, p2, samples, samplesize

    p1.plot(samples, plot_param1)
    p2.addItem(p1.plot(samples, plot_param2, pen='b'))

    pw.autoRange()

    p1.setXRange(framecount*samplesize, framecount*samplesize+samplesize)
    p2.setXRange(framecount*samplesize, framecount*samplesize+samplesize)

    if framecount == 0:
        flushloop = samplesize
    else:
        flushloop = samplesize+1

    for flush in range(1,flushloop):
        plot_param1.pop(0)
        plot_param2.pop(0)
        samples.pop(0)

    # below code is to prepare for next sample
    framecount = framecount + 1

    for update in range(framecount*samplesize, framecount*samplesize+samplesize):
        plot_param1.append(update*framecount*2)
        plot_param2.append(update*framecount*3)
        samples.append(update)

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(50)
