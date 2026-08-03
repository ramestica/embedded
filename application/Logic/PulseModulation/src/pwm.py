#
# System stuff
#
from sys import _getframe
from collections import deque

#
# RAM stuff
#
from embedded.serial.application_commands import ApplicationCommands

#-------------------------------------------------------------------------------
class PWM(ApplicationCommands):
    #----------------------------------------------------------------------------
    def __init__(self, port = '/dev/xbee', baud = 115200, timeout = 5):
        #
        # base class provides serial commanding interface
        #
        ApplicationCommands.__init__(self, port, baud, timeout)

        #
        # device commands
        #
        self.add_commands(
            'MPU6050',
            ApplicationCommands.commands['MPU6050'].recvr_id,
            ApplicationCommands.commands['MPU6050'].commands)

        self.nslaves = self.send_recv(
            self.commands['MPU6050'].recvr_id[0],
            self.commands['MPU6050'].commands['GETNSLAVES'],
            [],
            ['B'])[0]
        
    #----------------------------------------------------------------------------
    def READOUT2(self):
        return self.send_recv(
            12,
            0,
            [],
            ['f'] * 3)
    #['>h']*6)
    def READOUT(self):
        return self.send_recv(
            self.commands['MPU6050'].recvr_id[0],
            self.commands['MPU6050'].commands[_getframe().f_code.co_name],
            [],
            ['f'] * (1 + self.nslaves) * 3 * 2)
    #['>h']*6)

    #----------------------------------------------------------------------------
    def READ_SLAVE_REGISTER(self, addr, reg):
        ret = self.send_recv(
            self.recvs['MPU6050'],
            self.cmds[self.recvs['MPU6050']][_getframe().f_code.co_name][0],
            [addr, reg],
            ['B'])
        if True:
            print('i2c register readout')
            print('addr/reg', [hex(i) for i in [addr, reg]])
            print([bin(i) for i in ret])
        return ret

    #----------------------------------------------------------------------------
    def WRITE_SLAVE_REGISTER(self, addr, reg, val):
        self.send_recv(
            self.recvs['MPU6050'],
            self.cmds[self.recvs['MPU6050']][_getframe().f_code.co_name][0],
            [addr, reg, val],
            [])

    #----------------------------------------------------------------------------
    def SET_ACCEL_CALIB(self):
        print(_getframe().f_code.co_name)

    #----------------------------------------------------------------------------
    def TOGGLE_CALIB_ONOFF(self):
        return self.send_recv(self.recvs['MPU6050'], self.cmds[self.recvs['MPU6050']][_getframe().f_code.co_name][0], [], ['b'])

    #----------------------------------------------------------------------------
    def telemetry(self, filename, period, count):
        #
        # make sure that the calibration scaling/offseting is disabled
        #
        if self.TOGGLE_CALIB_ONOFF()[0] != 0:
            if self.TOGGLE_CALIB_ONOFF()[0] != 0:
                raise Exception("failed to disable calibration")

        #
        # collect data for the given lapse of time
        #
        super(MPU6050, self).telemetry(
            filename,
            self.recvs['MPU6050'],
            self.cmds[self.recvs['MPU6050']]['READOUT'][0],
            [],
            ['f'] * 18,
            period,
            count)

        #
        # make sure that the calibration scaling/offseting is now enabled
        #
        if self.TOGGLE_CALIB_ONOFF()[0] != 1:
            if self.TOGGLE_CALIB_ONOFF()[0] != 1:
                raise Exception("failed to enable calibration")

    #----------------------------------------------------------------------------
    def readTelemetry(self, filename):
        #
        # read telemetry file and extract all three accelerometers
        #
        self.rgb = []
        self.rgb.append(AccelerometerEllipsoid(filename, 0, 3))
        self.rgb.append(AccelerometerEllipsoid(filename, 7, 10))
        self.rgb.append(AccelerometerEllipsoid(filename, 13, 16))

    #----------------------------------------------------------------------------
    def ellipsoid(self, filename):
        #
        # read telemetry
        #
        self.readTelemetry(filename)

        #
        # print out ellipsoid fitting
        #
        for i in self.rgb:
            print(i.plsq)

        #
        # plot telemetry data
        #
        for i in self.rgb:
            i.plot()

    #----------------------------------------------------------------------------
    def ellipsoid2calibration(self, plsq):
        """
        Given ellipsoid parameters convert them to scale and offsets as used by
        the sensor software.
        """
        ret = []
        ret.append(1 / plsq[0])
        ret.append(1 / plsq[1])
        ret.append(1 / plsq[2])
        ret.append(ret[0] * plsq[3])
        ret.append(ret[1] * plsq[4])
        ret.append(ret[2] * plsq[5])
        return ret

    #----------------------------------------------------------------------------
    def apply(self, filename):
        #
        # read telemetry
        #
        self.readTelemetry(filename)

        #
        # send accelerometer scale and offset for all three sensors
        #
        self.send_recv(1, 2, [0]    + self.ellipsoid2calibration(self.rgb[0].plsq[0]), [])
        self.send_recv(1, 2, [0x68] + self.ellipsoid2calibration(self.rgb[1].plsq[0]), [])
        self.send_recv(1, 2, [0x69] + self.ellipsoid2calibration(self.rgb[2].plsq[0]), [])

    #----------------------------------------------------------------------------
    def plot(self):
        from embedded.data.gui import GUI

        plotLength = 600
        accels = [[0, 1, 2], [6, 7, 8], [12, 13, 14]]
        gyros = [[3, 4, 5], [9, 10, 11], [15, 16, 17]]
        items = []
        for i in range(3):
            items.append(
                GUI.PlotStructure(
                    "Accelerometer #%d" % (i + 1),
                    i, 0,
                    "time",
                    "?",
                    None,
                    [GUI.PlottedVariableStructure(
                        None,
                        deque([float(0)] * plotLength),
                        accels[i][0],
                        'y'),
                     GUI.PlottedVariableStructure(
                         None,
                         deque([float(0)] * plotLength),
                         accels[i][1],
                         'g'),
                     GUI.PlottedVariableStructure(
                         None,
                         deque([float(0)] * plotLength),
                         accels[i][2],
                         'm')]))
            items.append(
                GUI.PlotStructure(
                    "Gyroscope #%d" % (i + 1),
                    i, 1,
                    "time",
                    "?",
                    None,
                    [GUI.PlottedVariableStructure(
                        None,
                        deque([float(0)] * plotLength),
                        gyros[i][0],
                        'y'),
                     GUI.PlottedVariableStructure(
                         None,
                         deque([float(0)] * plotLength),
                         gyros[i][1],
                         'g'),
                     GUI.PlottedVariableStructure(
                         None,
                         deque([float(0)] * plotLength),
                         gyros[i][2],
                         'm')]))
        gui = GUI(items, self.READOUT, 100)
        gui.execute()

    #----------------------------------------------------------------------------
    @staticmethod
    def test(mpu):
        from time import sleep

        i2cAddr = 0x68
        MPU6050_RA_USER_CTRL = 0x6A
        MPU6050_RA_PWR_MGMT_1 = 0x6B
        MPU6050_RA_I2C_MST_DELAY_CTRL = 0x67
        MPU6050_ADDRESS_AD0_LOW = 0x68
        MPU6050_I2C_MST_EN = 0b00100000
        MPU6050_RA_INT_PIN_CFG = 0x37
        MPU6050_I2C_BYPASS_EN = 0b00000010
        MPU6050_CLOCK_PLL_XGYRO = 0x1
        MPU6050_PWR_MGMT_1_SLEEP = 0b01000000
        MPU6050_PWR_MGMT_1_CYCLE = 0b00100000
        MPU6050_DELAY_ES_SHADOW = 0b10000000

        #
        # enable master mode on auxiliary i2c bus
        #
        mpu.I2C_SET_BIT_MASK(
            i2cAddr,
            MPU6050_RA_USER_CTRL,
            MPU6050_I2C_MST_EN)

        #
        # disable i2c pass-through
        #
        mpu.I2C_SET_BIT_MASK(
            i2cAddr,
            MPU6050_RA_INT_PIN_CFG,
            MPU6050_I2C_BYPASS_EN)

        print('run...')

        sleep(5)

        mpu.READ_SLAVE_REGISTER(0x68, MPU6050_RA_PWR_MGMT_1)

#___oOo___
