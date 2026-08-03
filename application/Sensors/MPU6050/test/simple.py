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
    def __init__(self, port = '/dev/xbee', baud = 115200, timeout = 5):
        #
        # base class provides serial commanding interface
        #
        ApplicationCommands.__init__(self, port, baud, timeout)

        #
        # application commands
        #
        self.add_commands('APPLICATION', 12, ['READOUT', 'READOUT_ATTITUDE', 'READOUT_ACCELEROMETER']) 
        
    #----------------------------------------------------------------------------
    def READOUT(self):
        self.telemetry.enqueue_sample(
            self.send_recv(
                self.commands['APPLICATION'].recvr_id[0],
                self.commands['APPLICATION'].commands[_getframe().f_code.co_name],
                [],
                ['f'] * 5))
        return self.telemetry.get_last_sample()

    #----------------------------------------------------------------------------
    def READOUT_ATTITUDE(self):
        return self.send_recv(
            self.commands['APPLICATION'].recvr_id[0],
            self.commands['APPLICATION'].commands[_getframe().f_code.co_name],
            [],
            ['f'] * 2)

    #----------------------------------------------------------------------------
    def accel_ellipsoide_measure(self, delayms = 100, count = 100, plot = True):
        """Collect accelerometer data and fit an ellipsoid.

        Default parameters run for 10 seconds recording.

        Keyword arguments:
        delayms -- delay between samples in milliseconds
        count -- number of samples to take

        """

        from time import sleep

        ts = timestamp()
        self.telemetry(
            self.sensor_name,
            ts,
            12,
            2,
            [],
            ['f'] * 3,
            delayms / 1000.,
            count)
        print('%s[%s]' % (self.sensor_name, ts))
        if plot:
            self.accel_ellipsoide_plot(ts)
            
    #----------------------------------------------------------------------------
    def accel_ellipsoide_plot(self, ts):
        ae = AccelerometerEllipsoid(self.telemetry_data.get(ts))
        print(ae.parameters())
        ae.plot('%s[%s]' % (self.sensor_name, ts))
        
    #----------------------------------------------------------------------------
    def plot(self, file_name = None):
        from embedded.data.gui import GUI
        time_stamp = timestamp()
        plotLength = 600
        items = []
        items.append(
            GUI.PlotStructure(
                "Roll and pitch filtered angles",
                0, 0,
                "time",
                "?",
                None,
                [GUI.PlottedVariableStructure(
                    None,
                    deque([float(0)] * plotLength),
                    0,
                    'y',
                    'roll'),
                 GUI.PlottedVariableStructure(
                     None,
                     deque([float(0)] * plotLength),
                     1,
                     'g',
                     'pitch')]))
        items.append(
            GUI.PlotStructure(
                "Accelerometer",
                1, 0,
                "time",
                "?",
                None,
                [GUI.PlottedVariableStructure(
                    None,
                    deque([float(0)] * plotLength),
                    2,
                    'y',
                    'gyro X'),
                GUI.PlottedVariableStructure(
                    None,
                    deque([float(0)] * plotLength),
                    3,
                    'g',
                    'gyro Y'),
                GUI.PlottedVariableStructure(
                    None,
                    deque([float(0)] * plotLength),
                    4,
                    'b',
                    'gyro Z')]))
        self.telemetry.reset_queue()
        gui = GUI(items, self.READOUT, 100)
        gui.execute()
        if file_name is not None:
            self.telemetry.dump(file_name, time_stamp)

if __name__ == '__main__':
    app = Application()
    app.STATS(0)
    app.plot('mpu6050.chip1')
    
#___oOo___
