from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
import pyqtgraph.opengl as gl
from collections import deque
import numpy as np

data1 = deque(np.random.normal(size=100))
data2 = deque(np.random.normal(size=100))

app = QtGui.QApplication([])
w = QtGui.QWidget()
layout = QtGui.QGridLayout()
w.setLayout(layout)

#pg.PlotWidget.sizeHint = gl.GLViewWidget.sizeHint = lambda s: pg.QtCore.QSize(100, 100)

plot = pg.PlotWidget(title='hola')
plotCurve1 = plot.plotItem.plot(pen='y')
plotCurve2 = plot.plotItem.plot(pen='g')
plotCurve1.setData(data1)
plotCurve2.setData(data2)

view = gl.GLViewWidget()
view.setSizePolicy(pg.QtGui.QSizePolicy.Expanding, pg.QtGui.QSizePolicy.Expanding)
xgrid = gl.GLGridItem()
view.addItem(xgrid)
layout.addWidget(plot, 0, 0)
layout.addWidget(view, 0, 1)
#plot.sizeHint = view.sizeHint = lambda: pg.QtCore.QSize(100, 100)
#view.setSizePolicy(plot.sizePolicy())
#w.resize(1000,600)
w.show()

def update():
    global data1, data2, plotCurve1, plotCurve2
    data1.rotate(-1)
    data1[-1] = np.random.random()
    data2.rotate(-1)
    data2[-1] = np.random.random()
    plotCurve1.setData(data1)
    plotCurve2.setData(data2)
    xgrid.rotate(2, 1, 0, 0)

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(1000)

QtGui.QApplication.instance().exec_()
