#
# System stuff
#
import os
import numpy
import collections

#
# Utils stuff
#
from embedded.utils.time import timestamp
from embedded.utils.formatting import data_to_csv
from embedded.utils.formatting import data_to_vcd

#
# Mathematics stuff
#
from embedded.mathematics.adc import binarize

#
# Embedded stuff
#
from embedded.data.defs import SCOPE_PATH

#--------------------------------------------------------------------------------
class USBTMC():
    #----------------------------------------------------------------------------
    def __init__(self, port = '/dev/rigol1102e'):
        self.ParamsStruct = collections.namedtuple(
            'ParamsStruct',
            ['volt_scale',
             'volt_offset',
             'volt_attn',
             'chan_coup',
             'acq_samp'])
        self.DataStruct = collections.namedtuple(
            'DataStruct',
            ['time_stamp',
             'sample_rate',
             'channels'])
        self.fd = os.open(port, os.O_RDWR)
        self.send("*IDN?")
        self.id = os.read(self.fd, 300)

        #os.write(fd, ':RUN')
        #os.write(fd, ':STOP')

    #----------------------------------------------------------------------------
    def __del__(self):
        os.close(self.fd)

    #----------------------------------------------------------------------------
    def receive(self, size):
        return os.read(self.fd, size)

    #----------------------------------------------------------------------------
    def send(self, data):
        os.write(self.fd, bytes(data, 'utf8'))

    #----------------------------------------------------------------------------
    def read_params(self, channels):
        self.params = {}

        for ch in channels:
            self.send(":CHAN%d:SCAL?" % (ch))

            volt_scale = float(self.receive(20))

            self.send(":CHAN%d:OFFS?" % (ch))

            volt_offset = float(self.receive(20))

            self.send(":CHAN%d:PROB?" % (ch))

            volt_attn = float(self.receive(20))

            self.send(":CHAN%d:COUP?" % (ch))

            chan_coup = self.receive(20)

            self.send(":ACQ:SAMP? CHAN%d" % (ch))

            acq_samp = float(self.receive(20))

            self.params[ch] = self.ParamsStruct(volt_scale,
                                                volt_offset,
                                                volt_attn,
                                                chan_coup,
                                                acq_samp)

        # Get the timescale
        self.send(":TIM:SCAL?")
        self.timescale = float(self.receive(20))

        # Get the timescale offset
        self.send(":TIM:OFFS?")
        self.timeoffset = float(self.receive(20))

    #----------------------------------------------------------------------------
    def printout(self):
        for ch in self.params.keys():
            print('        volt scale channel[%d] = %f' % (ch, self.params[ch].volt_scale))
        for ch in self.params.keys():
            print('       volt offset channel[%d] = %f' % (ch, self.params[ch].volt_offset))
        for ch in self.params.keys():
            print('volt attn. factor channel[%d]  = %f' % (ch, self.params[ch].volt_attn))
        for ch in self.params.keys():
            print('            sampling rate[%d]  = %f' % (ch, self.params[ch].acq_samp))
        print('time scale  = ', self.timescale)
        print('time offset = ', self.timeoffset)

    #----------------------------------------------------------------------------
    def read(self, channels, mode):
        """
        Read scope for given channels' data. The scope should be in stop
        condition. The returned data structure contains a timestamp which was
        taken before reading the scope.

        A typical synopsis:
        usbt=USBTMC()
        d=usbt.read([1,2], 'RAW')
        """

        if type(channels) == int:
            chans = [channels]
        else:
            chans = channels

        self.send(":WAV:POIN:MODE %s" % (mode))

        data = {}

        ts = timestamp()

        for ch in chans:
            self.send(":WAV:DATA? CHAN%d" % (ch))
            # first ten bytes are header, so skip
            # http://scruss.com/blog/tag/rigol/
            data[ch] = numpy.frombuffer(self.receive(524288), 'B')[10:]

        self.read_params(chans)

        # convert data from (inverted) bytes to an array of scaled floats
        # this magic from Matthew Mets

        for ch in chans:
            data[ch] = 255 - data[ch]
            data[ch] = (data[ch] - 130. - self.params[ch].volt_offset / self.params[ch].volt_scale * 25.) / 25. * self.params[ch].volt_scale

        #
        # if both channels do not have the same sample rate then how to handle
        # that?
        #
        sa_rate = set()
        for ch in chans:
            sa_rate.add(self.params[ch].acq_samp)
        if len(sa_rate) != 1:
            raise Exception("sample rate not unique across channels")

        return self.DataStruct(ts, sa_rate.pop(), data)

    #----------------------------------------------------------------------------
    def save(self, channels, threadholds, mode, comment):
        """
        Read scope and save to disk.

        A typical synopsis:
        usbt=USBTMC()
        usbt.save_vcd([1,2], [1.0, 1.0], 'RAW', 'test')
        """

        data = self.read(channels, mode)
        fn = os.path.join(SCOPE_PATH, data.time_stamp) + '.' + comment
        data_to_csv(
            fn + '.csv',
            [data.channels[channels[i]] for i in range(len(channels))],
            header='100000000\nscl,sda')
        data_to_vcd(
            fn + '.vcd',
            data.sample_rate,
            [binarize(data.channels[channels[i]], threshold = threadholds[i])
             for i in range(len(channels))])

        return fn

#___oOo___
