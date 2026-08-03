import time
import re
import sys
import os
import signal
from serial import Serial
import struct
from threading import Thread
from threading import Event
from threading import Lock
from time import sleep

#-------------------------------------------------------------------------------
class SerialMonitor(Serial):
    """ Connect to serial port and write or read data from it. """

    #-------------------------------------------------------------------------------
    def __init__(self, port = '/dev/ttyUSB0', baud = 115200, timeout = 5):
        Serial.__init__(self, port=port, baudrate = baud, timeout = timeout)

    #-------------------------------------------------------------------------------
    def __del__(self):
        """
        Shutdown whatever is still left there.
        """
        self.close()

    #-------------------------------------------------------------------------------
    def flush(self, input = True, output = True):
        if input:
            self.flushInput()
        if output:
            self.flushOutput()

    #-------------------------------------------------------------------------------
    def send_recv(self, recv, cmd, params, types):
        """Send command and wait for eventual reply.

        Parameters are expected to be 8 bit integers or single
        precision floating point values.

        Keyword arguments:
        recv -- command reciver id
        cmd -- command to execute
        params -- list of command parameters (can be empty)
        types -- list of type for each expected return value (can be empty)

        """
        
        frameSize = 0

        #
        # figure out the reply size and the type of each scalar in that reply
        # FIXME: use struct.calcsize
        #
        for t in types:
            frameSize += struct.calcsize(t)

        #
        # make sure that older stuff is flushed from input and output buffers
        #
        self.flush()

        #
        # send receiver id
        #
        self.write([recv])

        #
        # send command
        #
        self.write([cmd])

        #
        # send command parameters
        #
        # FIXME: wouldn't be more efficient to pack all params in one
        # single bytes string and call 'write' only once?
        #
        for p in params:
            #
            # note that by using issubclass a floating point number of 64 will
            # be converetd to the expected 32 bits automatically
            #
            if issubclass(type(p), float):
                self.write(struct.pack('f', p))
            else:
                self.write([p])
        #params_buffer = bytes()
        #for p in params:
        #    params_buffer += struct.pack('f', p) if issubclass(type(p), float) else struct.pack('B', p)
        #if len(params_buffer) > 0:
        #    self.write(params_buffer)

        #
        # first reply byte must identify the same receiver
        #
        rrecv = ord(self.read(1))
        if recv != rrecv:
            raise Exception('unmatched receiver id in reply (recv/rrecv=%d/%d)' % (recv, rrecv))

        #
        # second reply byte must identify the same command
        #
        rcmd = ord(self.read(1))
        if cmd != rcmd:
            raise Exception('unmatched command id in reply (cmd/rcmd=%d/%d)' % (cmd, rcmd))

        #
        # check reply status
        #
        status = ord(self.read(1))
        if status == '':
            raise Exception('reply status timeout (' + str(getTimeout()) + ' seconds)' )
        elif status != 0:
            raise Exception('reply status not okay (status/rrecv/rcmd=%d/%d/%d)' % (status, rrecv, rcmd))

        #
        # read cmd reply (if any reply actually expected)
        #
        if frameSize > 0:
            l = self.read(frameSize)
            
            if len(l) != frameSize:
                raise Exception('unmatched reply buffer size or timeout (expected/recv=%d/%d)' % (frameSize, len(l)))

            pos = 0

            data = []

            for t in types:
                data.append(struct.unpack(t, l[pos:pos + struct.calcsize(t)])[0])
                pos += struct.calcsize(t)

            return data

        #
        # note the asymmetry, if a reply is expected then return that
        # otherwise return the command reply status
        #
        return status

#-------------------------------------------------------------------------------
class SerialMonitorThreadBase(Thread):
    """ Connect to serial port and print or other tasks... """
    globalObj = None

    #-------------------------------------------------------------------------------
    def __init__(self, port = '/dev/ttyUSB0', baud = 115200, useCtrlC = False):
        Thread.__init__(self)

        self.MAINLOOP_CMD_TIMER = 0
        self.MAINLOOP_CMD_UPTIME = 1
        self.MAINLOOP_CMD_STATS = 2
        self.MAINLOOP_CMD_I2C_READ = 3
        self.MAINLOOP_CMD_I2C_WRITE = 4

        self.readObject = None
        self.port = port
        self.baud = baud
        self.event = Event()
        self.lock = Lock()
        self.runFlag = True
        self.rex = None
        self.defaultInterrupt = None

        #
        # if request then use ctrl-c signal to stop current operation
        #
        if useCtrlC:
            #
            # only one instance can use the global object
            #
            if SerialMonitorThreadBase.globalObj != None:
                raise Exception('global object already in use')

            #
            # flag to be use when enebling some monitor task
            #
            self.useCtrlC = True

            #
            # gloabl object tracks this current instance
            #
            SerialMonitorThreadBase.globalObj = self
        else:
            self.useCtrlC = False

        self.doReadLines = False
        self.doReadDataStr = False

        self.readObject = None

        #
        # instantiate serial reading object, if it fails at this point is not
        # important, in that case we will try again later
        #
        self.readObjectCreate(False)

        #
        # start the run loop
        #
        self.start()

    #-------------------------------------------------------------------------------
    def __del__(self):
        """
        Shutdown whatever is still left there.
        """
        self.runFlag = False
        self.doReadLines = False
        self.doReadDataStr = False
        print('setting')
        self.event.set()
        print('joining')
        self.join()
        print('joined')

        #
        # if ctrl-c in place then uninstal handler
        #
        if self.defaultInterrupt != None:
            signal.signal(signal.SIGINT, self.defaultInterrupt)
            SerialMonitorThreadBase.globalObj = None

    #-------------------------------------------------------------------------------
    def stop(self):
        """
        Stop thread.
        """
        self.runFlag = False

        self.doReadLines = False

        self.doReadDataStr = False

        self.event.set()

    #-------------------------------------------------------------------------------
    def open(self):
        self.readObject.open()

    #-------------------------------------------------------------------------------
    def close(self):
        self.readObject.close()

    #-------------------------------------------------------------------------------
    def flush(self, input = True, output = True):
        if input:
            self.readObject.flushInput()
        if output:
            self.readObject.flushOutput()

    #-------------------------------------------------------------------------------
    def readObjectCreate(self, isErrorImportant = True):
        if self.readObject != None:
            return

        try:
            #
            # create serial port reading object, the connection is created closed
            #
            self.readObject = serial.Serial()

            #
            # set connection parameters
            #
            self.readObject.setPort(self.port)
            self.readObject.setBaudrate(self.baud)

            #
            # would I need to tune it for different cases?
            #
            self.readObject.setTimeout(5)

        except Exception as e:
            #
            # we will try again later
            #
            self.readObject = None

            #
            # if error important then rethrow exception
            #
            if isErrorImportant:
                raise e

    #-------------------------------------------------------------------------------
    def readLines(self):
        #
        # stop by means of ctrl-c
        #
        if self.useCtrlC:
            self.defaultInterrupt = signal.signal(signal.SIGINT, self.handler)

        self.doReadLines = True

        self.event.set()

    #-------------------------------------------------------------------------------
    def readDataStr(self, regexp):
        #
        # stop by means of ctrl-c
        #
        if self.useCtrlC:
            self.defaultInterrupt = signal.signal(signal.SIGINT, self.handler)

        self.rex = re.compile(regexp)

        self.doReadDataStr = True

        self.event.set()

    #-------------------------------------------------------------------------------
    def sendRecv_missing_receiver_id(self, cmd, params, types, isPrint = False):
        dataTypes = []

        frameSize = 0

        for t in types:
            if t == 'b' or t == 'B':
                dataTypes.append((t, 1))
                frameSize += 1
            elif t == 'h':
                dataTypes.append((t, 2))
                frameSize += 2
            elif t == 'f':
                dataTypes.append((t, 4))
                frameSize += 4
            else:
                raise Exception('data type not supported (' + t + ')')

        self.lock.acquire()

        #
        # make sure that older stuff is flushed from input and output buffers
        #
        self.flush()

        #
        # send command
        #
        self.readObject.write([cmd])

        #
        # send command parameters
        #
        for p in params:
            self.readObject.write(chr(p))

        #
        # first reply byte must identify the same command
        #
        rcmd = ord(self.readObject.read(1))
        if cmd != rcmd:
            self.lock.release()
            raise Exception('unmatched command id in reply (cmd/rcmd=%d/%d)' % (cmd, rcmd))

        #
        # check reply status
        #
        status = ord(self.readObject.read(1))
        if status != 0:
            self.lock.release()
            raise Exception('reply status not okay (' + chr(status) + ')' )

        l = self.readObject.read(frameSize)

        if len(l) != frameSize:
            self.lock.release()
            raise Exception('unmatched reply buffer size or timeout (expected/recv=%d/%d)' % (frameSize, len(l)))

        pos = 0

        data = []

        for dt in dataTypes:
            data.append(struct.unpack(dt[0], l[pos:pos + dt[1]])[0])
            pos += dt[1]

        self.lock.release()

        if isPrint:
            if cmd == self.MAINLOOP_CMD_UPTIME:
                print('uptime [s]:', data[0])
            elif cmd == self.MAINLOOP_CMD_STATS:
                print('main loop timing statistics')
                print('loop     :', data[0:3])
                print('algorithm:', data[3:6])
                print('comm     :', data[6:9])
            elif cmd == self.MAINLOOP_CMD_I2C_READ:
                print('i2c register readout')
                print('addr/reg/size', [hex(i) for i in params])
                print([bin(i) for i in data])

        return data

    #-------------------------------------------------------------------------------
    def readDataBinary(self, cmd, types):
        dataTypes = []

        frameSize = 0

        for t in types:
            if t == 'h':
                dataTypes.append((t, 2))
                frameSize += 2
            elif t == 'f':
                dataTypes.append((t, 4))
                frameSize += 4
            else:
                raise Exception('data type not supported (' + t + ')')

        data = []

        self.lock.acquire()

        self.readObject.flushInput()

        self.readObject.write(chr(cmd))

        l = self.readObject.read(frameSize)

        pos = 0

        for dt in dataTypes:
            data.append(struct.unpack(dt[0], l[pos:pos + dt[1]])[0])
            pos += dt[1]

        self.lock.release()

        return data

    #-------------------------------------------------------------------------------
    def getData(self):
        self.lock.acquire()

        data = self.data[:]

        self.lock.release()

        return data

    #-------------------------------------------------------------------------------
    def idle(self):
        self.doReadLines = False

        self.doReadDataStr = False

        os.kill(os.getppid(), signal.SIGINT)

    #-------------------------------------------------------------------------------
    def run(self):
        while self.runFlag:
            #
            # wait until something is commanded to happen
            #
            self.event.wait()
            self.event.clear()

            try:
                #
                # make sure the reading selfect is okay
                #
                self.readObjectCreate(True)

                #
                # open port
                #
                self.readObject.open()

                #
                # discard current data
                #
                self.readObject.flush()

            except Exception as e:
                print('failed to create or open connection')
                self.doReadLines = False
                continue

            #
            # loop reading new lines until stopped
            #
            while self.doReadLines or self.doReadDataStr:
                try:
                    l = self.readObject.readline()
                    if self.doReadLines:
                        sys.stdout.write(l)
                    if self.doReadDataStr:
                        self.lock.acquire()
                        self.data = self.rex.findall(l)
                        self.lock.release()
                except Exception as e:
                    self.doReadLines = False

            #
            # disconnect port
            #
            self.readObject.close()

        print("thread has stopped")

    #-------------------------------------------------------------------------------
    @staticmethod
    def handler(signum, frame):
        """
        stop monitoring on ctrl-c
        """
        if signum == signal.SIGINT:
            signal.signal(signal.SIGINT, SerialMonitorThreadBase.globalObj.defaultInterrupt)
            SerialMonitorThreadBase.globalObj.defaultInterrupt = None
            SerialMonitorThreadBase.globalObj.idle()

#___oOo___
