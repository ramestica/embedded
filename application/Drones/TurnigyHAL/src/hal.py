#
# System stuff
#
from sys import _getframe
from collections import deque

#
# RAM stuff
#
from embedded.serial.application_commands import ApplicationCommands
from embedded.data.accelerometer_ellipsoid import AccelerometerEllipsoid
from embedded.utils.time import timestamp

#-------------------------------------------------------------------------------
class Application(ApplicationCommands):
    #----------------------------------------------------------------------------
    def __init__(self, port = '/dev/xbee', baud = 115200, timeout = 0.2):
        #
        # base class provides serial commanding interface
        #
        ApplicationCommands.__init__(self, port, baud, timeout)

        #
        # application commands
        #
        self.add_commands('APPLICATION', 12, ['READOUT', 'PIDPARAMS']) 
        
    #----------------------------------------------------------------------------
    def READOUT(self):
        self.telemetry.enqueue_sample(
            self.send_recv(
                self.commands['APPLICATION'].recvr_id[0],
                self.commands['APPLICATION'].commands[_getframe().f_code.co_name],
                [],
                ['I'] + ['f'] * 9))
        return self.telemetry.get_last_sample()

    #----------------------------------------------------------------------------
    def PIDPARAMS(self, controller, kp, ki, kd, scale):
        self.send_recv(
            self.commands['APPLICATION'].recvr_id[0],
            self.commands['APPLICATION'].commands[_getframe().f_code.co_name],
            [int(controller), float(kp), float(ki), float(kd), float(scale)],
            [])

    #----------------------------------------------------------------------------
    def plot(self, file_name = None):
        from embedded.data.gui import GUI

        time_stamp = timestamp()
        plotLength = 600
        items = []
        items.append(
            GUI.PlotStructure(
                "angle, velocity and reference",
                0, 0,
                "time",
                "?",
                None,
                [GUI.PlottedVariableStructure(
                    None,
                    deque([float(0)] * plotLength),
                    1,
                    'y',
                    'reference'),
                 GUI.PlottedVariableStructure(
                     None,
                     deque([float(0)] * plotLength),
                     2,
                     'g',
                     'angle'),
                 GUI.PlottedVariableStructure(
                     None,
                     deque([float(0)] * plotLength),
                     3,
                     'b',
                     'velocity')]))
        items.append(
            GUI.PlotStructure(
                "throttle and control action",
                0, 1,
                "time",
                "?",
                None,
                [GUI.PlottedVariableStructure(
                    None,
                    deque([float(0)] * plotLength),
                    4,
                    'y',
                    'throttle'),
                 GUI.PlottedVariableStructure(
                     None,
                     deque([float(0)] * plotLength),
                     5,
                     'g',
                     'control action')]))
        items.append(
            GUI.PlotStructure(
                "motor action",
                0, 2,
                "time",
                "?",
                None,
                [GUI.PlottedVariableStructure(
                    None,
                    deque([float(0)] * plotLength),
                    6,
                    'y',
                    'U1'),
                 GUI.PlottedVariableStructure(
                    None,
                     deque([float(0)] * plotLength),
                     7,
                     'g',
                     'U2'),
                 GUI.PlottedVariableStructure(
                     None,
                     deque([float(0)] * plotLength),
                     8,
                     'b',
                     'U3'),
                 GUI.PlottedVariableStructure(
                     None,
                     deque([float(0)] * plotLength),
                     9,
                     'r',
                     'U4')]))
        self.telemetry.reset_queue()
        gui = GUI(items, self.READOUT, 100)
        gui.execute()
        if file_name is not None:
            self.telemetry.dump(file_name, time_stamp)

if __name__ == '__main__':
    app = Application()
    app.STATS(0)
    app.plot('hal.pid')
    
#___oOo___
