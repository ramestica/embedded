#
# System stuff
#
import numpy as np
from scipy.optimize import leastsq

#
# RAM stuff
#

#-------------------------------------------------------------------------------
class AccelerometerEllipsoid:
    #-------------------------------------------------------------------------------
    def __init__(self, data, p0 = None, fullout = 0):
        import numpy as np
        self.tl = np.array(data)
        if p0 == None:
            self.p0 = [1., 1., 1., 0., 0., 0.]
        else:
            self.p0 = p0
        self.plsq = leastsq(self.residuals, self.p0, args=(self.tl), full_output = fullout)

    def ellipsoid(self, x, a, b, c, oa, ob, oc):
        return ((x[:,0] - oa) / a) ** 2 + ((x[:,1] - ob) / b) ** 2 + ((x[:,2] - oc) / c) ** 2

    def parameters(self):
        ret = list(self.plsq[0])
        ret[0] = 1 / ret[0]
        ret[1] = 1 / ret[1]
        ret[2] = 1 / ret[2]
        ret[3] = ret[3] * ret[0]
        ret[4] = ret[4] * ret[1]
        ret[5] = ret[5] * ret[2]
        return (list(self.plsq[0]), ret)

    def residuals(self, p, y):
        return 1. - self.ellipsoid(y, p[0], p[1], p[2], p[3], p[4], p[5])

    def plot_ellipsoid_grid(self, center, radii, rotation, ax, plotAxes=False, cageColor='m', cageAlpha=0.2):
        """
        Plot an ellipsoid, copied from the following url:
        https://github.com/minillinim/ellipsoid/blob/master/ellipsoid.py
        """
        u = np.linspace(0.0, 2.0 * np.pi, 100)
        v = np.linspace(0.0, np.pi, 100)

        # cartesian coordinates that correspond to the spherical angles:
        x = radii[0] * np.outer(np.cos(u), np.sin(v))
        y = radii[1] * np.outer(np.sin(u), np.sin(v))
        z = radii[2] * np.outer(np.ones_like(u), np.cos(v))
        # rotate accordingly
        for i in range(len(x)):
            for j in range(len(x)):
                [x[i,j],y[i,j],z[i,j]] = np.dot([x[i,j],y[i,j],z[i,j]], rotation) + center

        if plotAxes:
            # give axes some color
            colors = ['r', 'g', 'b']

            # make some purdy axes
            axes = np.array([[radii[0],0.0,0.0],
                             [0.0,radii[1],0.0],
                             [0.0,0.0,radii[2]]])

            # colors and axes in the same variable
            caxes = []

            # rotate accordingly
            for i in range(len(axes)):
                caxes.append([colors[i], np.dot(axes[i], rotation)])

            # plot axes
            for p in caxes:
                X3 = np.linspace(0, p[1][0], 100) + center[0]
                Y3 = np.linspace(0, p[1][1], 100) + center[1]
                Z3 = np.linspace(0, p[1][2], 100) + center[2]
                ax.plot(X3, Y3, Z3, color=p[0])

        # plot ellipsoid
        ax.plot_wireframe(x, y, z, rstride=4, cstride=4, color=cageColor, alpha=cageAlpha)

    def plot(self, title):
        from mpl_toolkits.mplot3d import Axes3D
        import matplotlib.pyplot as plt
        from mpl_toolkits.mplot3d import axes3d
        import seaborn as sns
        axis_color='black'
        sns.set(rc={'text.color': axis_color, 'axes.labelcolor': axis_color})

        params = self.parameters()[1]
        fig = plt.figure(facecolor='gray')
        fig.suptitle(title)
        ax_raw = fig.add_subplot(121, projection='3d', title='Raw gravity readings')
        ax_raw.scatter(self.tl[:,0], self.tl[:,1], self.tl[:,2], c='g', marker='*')
        self.plot_ellipsoid_grid(self.plsq[0][3:], self.plsq[0][:3], 1, ax_raw, True)
        ax_raw.set_xlabel('X [counts]')
        ax_raw.set_ylabel('Y [counts]')
        ax_raw.set_zlabel('Z [counts]')
        ax_gravity = fig.add_subplot(122, projection='3d', title='Calibrated gravity readings')
        ax_gravity.scatter(
            [i * params[0] - params[3] for i in self.tl[:,0]],
            [i * params[1] - params[4] for i in self.tl[:,1]],
            [i * params[2] - params[5] for i in self.tl[:,2]],
            c='g',
            marker='*')
        self.plot_ellipsoid_grid([0, 0, 0], [1, 1, 1], 1, ax_gravity, True)
        plt.show()

def parse(fd, n):
    rexstr = '\['
    for i in range(n):
        rexstr += '([\-\.0-9]+)'
        if i != n - 1:
            rexstr += ', '
    rexstr += '\]'
    rex = re.compile(rexstr)
    ret = None
    for l in fd:
        r = rex.match(l)
        if r != None:
            x = []
            for i in range(n):
                x.append(float(r.group(i + 1)))
            if ret == None:
                ret = array([x])
            else:
                ret = append(ret, [x], axis = 0)
    return ret


if __name__ == '__main__':
    import sys

    d = parse(open(sys.argv[1], int(sys.argv[2])))

    plot(d[:,0], d[:,1], d[:,2])

#___oOo___
