import pickle
from sys import _getframe
import collections

from embedded.serial.serial_monitor import SerialMonitor
from embedded.data.telemetry_helper import Telemetry

#-------------------------------------------------------------------------------
class ApplicationCommands(SerialMonitor):
    #
    # The receiver identifier should individualize the specific receiver
    # embedded in the target. But for functionality like PWM there could be
    # several receivers of such functionality type. To cope with that in a
    # general way then recvr_id is assumed always to be a list. Functions
    # implemented below and dealing with only one specific receiver
    # (e.g. MAIN_LOOP must always refer to recvr_id[0] for their intended
    # receiver identifier. Other functions should allow for an input parameter
    # individualizing a module of that matching functionality.
    #
    # FIXME: these definitions should be somehow auto-created from c++
    # header files.
    #
    CommandStruct = collections.namedtuple(
        'CommandStruct',
        ['recvr_id', 'commands'])

    commands = {}

    #
    # Main-loop implements the following commands
    #
    commands['MAIN_LOOP'] = CommandStruct(
        0,
        ['SYSTICK',
         'UPTIME',
         'STATS',
         'READ_TEST',
         'WRITE_TEST'])

    #
    # i2c commands
    #
    commands['I2C'] = CommandStruct(
        1,
        ['I2C_PING',
         'I2C_READ',
         'I2C_WRITE',
         'I2C_SET_BIT_MASK',
         'I2C_UNSET_BIT_MASK'])

    #
    # pwm encoder commands
    #
    commands['PWM'] = CommandStruct(
        [2,3,4,5,6,7,8,9],
        ['PWM_SET_DUTY_CYCLE'])

    #
    # MPU-6050 sensor
    #
    commands['MPU6050'] = CommandStruct(
        11,
        ['GETNSLAVES',
         'READOUT',
         'READ_SLAVE_REGISTER',
         'WRITE_SLAVE_REGISTER',
         'SET_ACCEL_CALIB',
         'TOGGLE_CALIB_ONOFF'])

    #----------------------------------------------------------------------------
    def __init__(self, port = '/dev/ttyUSB0', baud = 115200, timeout = 5):
        """
        Synopsis:
        from embedded.serial import application_commands
        ac=application_commands.ApplicationCommands(port='/dev/xbee')
        ac.STATS(0)
        """
        
        #
        # base class provides the actual access to the serial port
        #
        SerialMonitor.__init__(self, port, baud, timeout)

        #
        # application commands
        #
        self.commands = {}

        #
        # add commands for basic modules that every application would normally
        # like to expose anyhow.
        #
        for recv in ['MAIN_LOOP', 'I2C', 'PWM']:
            self.add_commands(
                recv,
                ApplicationCommands.commands[recv].recvr_id,
                ApplicationCommands.commands[recv].commands)

        #
        # a telemetry queue to store readout samples. This is a common
        # feature for many applications, that's why makes sense to
        # have it here in a common place.
        #
        self.telemetry = Telemetry()
        
        #
        # invoke base class to open serial channel
        #
        #self.open()

    #----------------------------------------------------------------------------
    def add_commands(self, recvr_name, recvr_id, cmds):
        """
        Add a receiver identifier and its associated commands. Check that the
        receiver identifier or its integer tag are not repeated and also that
        the name of the given commands do not repeat as well.
        """
        if type(recvr_id) is not list:
            recvr_id = [recvr_id]
            
        if recvr_name in self.commands:
            raise Exception('repeated receiver name (%s/%d)' % (recvr_name, recvr_id))
        for i in self.commands:
            if len([j for j in self.commands[i].recvr_id if j in recvr_id]) != 0:
                raise Exception('repeated receiver id (%s/%d)' % (recvr_name, recvr_id))
        if len(cmds) != len(set(cmds)):
            raise Exception('repeated command name in list')

        #
        # instatiate a CommandStruct such that the commands fields is not just
        # simply a list but a dictionary to command indexes.
        #
        self.commands[recvr_name] = ApplicationCommands.CommandStruct(
            recvr_id,
            dict(zip(cmds, range(len(cmds)))))

        #
        # FIXME: why is that originally not only the command index was recorded
        # but also a reference to the function that implements the command?
        #
        #self.cmds[self.recvs[recvid]][cmd] = [len(self.cmds[self.recvs[recvid]]), getattr(self, cmd)]

    #----------------------------------------------------------------------------
    def i2cScan(self):
        print('scanning i2c bus for positive ping acknowledgment')
        for i in range(128):
            if self.I2C_PING(i) == 1:
                print('\taddress', hex(i))

    #----------------------------------------------------------------------------
    def telemetry_old_method(self, filename, key, recv, cmd, params, format, period, count):
        """
        period: seconds in floating point format.
        """
        from time import sleep
        td = Telemetry(filename)
        data = []
        for i in range(count):
            data.append(self.send_recv(recv, cmd, params, format))
            sleep(period)
        td.dump(key, data)

    #----------------------------------------------------------------------------
    def TIMER(self):
        print(_getframe().f_code.co_name)

    #----------------------------------------------------------------------------
    def UPTIME(self):
        ret = self.send_recv(
            self.commands['MAIN_LOOP'].recvr_id[0],
            self.commands['MAIN_LOOP'].commands[_getframe().f_code.co_name],
            [],
            ['f'] * 1)
        print('uptime [s]:', ret[0])

    #----------------------------------------------------------------------------
    def STATS(self, statsType = 3):
        if statsType == 0:
            types = ['I'] + ['f'] * 4 + ['I'] * 3 + ['f'] * 4 + ['I'] * 3 + ['f'] * 4 + ['I'] * 3
        else:
            types = ['I'] + ['f'] * 4 + ['I'] * 3

        ret = self.send_recv(
            self.commands['MAIN_LOOP'].recvr_id[0],
            self.commands['MAIN_LOOP'].commands[_getframe().f_code.co_name],
            [statsType],
            types)

        print('allocated dynamic memory: %d [bytes]' % ret[0])
        
        print('main loop timing statistics cnt/nmin/nmax mean/min/max [us] ')

        if statsType == 0:
            print('loop      :%d/%d/%d\t%.1f/%.1f/%.1f' % (ret[5], ret[6], ret[7],
                                                          ret[1], ret[2], ret[3]))
            print('algorithm :%d/%d/%d\t%.1f/%.1f/%.1f' % (ret[12], ret[13], ret[14],
                                                          ret[8], ret[9], ret[10]))
            print('exec      :%d/%d/%d\t%.1f/%.1f/%.1f' % (ret[19], ret[20], ret[21],
                                                           ret[15], ret[16], ret[17]))
        else:
            if statsType == 1:
                stype = 'loop'
            elif statsType == 2:
                stype = 'algorithm'
            else:
                stype = 'exec'

            print('%s :%d/%d/%d\t%.1f/%.1f/%.1f' % (stype,
                                                    ret[5], ret[6], ret[7],
                                                    ret[1], ret[2], ret[3]))

    #----------------------------------------------------------------------------
    def READ_TEST(self, size):
        """
        Note that 'read' means that the target is reading, therefore, here we
        are actually writing.
        """
        ret = self.send_recv(
            self.commands['MAIN_LOOP'].recvr_id[0],
            self.commands['MAIN_LOOP'].commands[_getframe().f_code.co_name],
            [size] + range(size),
            ['I'])
        print('sending to target')
        print('size  =', size, '[bytes]')
        print('lapse =', ret[-1], '[us]')
        print('rate  =', (size * 8) / (float(ret[-1]) * 1e-6), '[bps]')

    #----------------------------------------------------------------------------
    def WRITE_TEST(self, size):
        """
        Note that 'write' means that the target is writing, therefore, here we
        are actually reading.
        """
        ret = self.send_recv(
            self.commands['MAIN_LOOP'].recvr_id[0],
            self.commands['MAIN_LOOP'].commands[_getframe().f_code.co_name],
            [size],
            ['B'] * size + ['I'])
        #        ['B'] * size)
        for i in range(size):
            if i != ret[i]:
                raise Exception('received unexpected value (idx/val=%d/%d)' % (i, ret[i]))
        print('sending from target')
        print('size  =', size, '[bytes]')
        print('lapse =', ret[-1], '[us]')
        print('rate  =', (size * 8) / (float(ret[-1]) * 1e-6), '[bps]')

    #----------------------------------------------------------------------------
    def I2C_PING(self, addr):
        return self.send_recv(
            self.commands['I2C'].recvr_id[0],
            self.commands['I2C'].commands[_getframe().f_code.co_name],
            [addr],
            ['B'])[0]

    #----------------------------------------------------------------------------
    def I2C_READ(self, addr, reg, size, isPrint = False):
        ret = self.send_recv(
            self.commands['I2C'].recvr_id[0],
            self.commands['I2C'].commands[_getframe().f_code.co_name],
            [addr, reg, size],
            ['B'] * size)
        if isPrint:
            print('i2c register readout')
            print('addr/reg/size', [hex(i) for i in [addr, reg, size]])
            print([bin(i) for i in ret])
        return ret

    #----------------------------------------------------------------------------
    def I2C_WRITE(self, addr, reg, value):
        self.send_recv(
            self.commands['I2C'].recvr_id[0],
            self.commands['I2C'].commands[_getframe().f_code.co_name],
            [addr, reg, value],
            [])

    #----------------------------------------------------------------------------
    def I2C_SET_BIT_MASK(self, addr, reg, mask):
        self.send_recv(
            self.commands['I2C'].recvr_id[0],
            self.commands['I2C'].commands[_getframe().f_code.co_name],
            [addr, reg, mask],
            [])

    #----------------------------------------------------------------------------
    def I2C_UNSET_BIT_MASK(self, addr, reg, mask):
        self.send_recv(
            self.commands['I2C'].recvr_id[0],
            self.commands['I2C'].commands[_getframe().f_code.co_name],
            [addr, reg, mask],
            [])

    #----------------------------------------------------------------------------
    def PWM_SET_DUTY_CYCLE(self, generator, select, dc):
        self.send_recv(
            self.commands['PWM'].recvr_id[generator],
            self.commands['PWM'].commands[_getframe().f_code.co_name],
            [select, float(dc)],
            [])

#___oOo___
