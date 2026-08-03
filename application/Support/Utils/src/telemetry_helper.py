import copy
import numpy as np

from embedded.data.defs import TELEMETRY_PATH

class Telemetry:

    def __init__(self, queue_length = 1000):
        """
        Class to keep telemetry samples and to store them as a labeled
        data set in a file.  It also makes possible to inspect files
        containing many of those data sets and retrieve them.

        Use numpy load/save to handle data in/out the file with pickle
        type enabled.

        A telemetry file contains only one object: a dictionary of
        different measurement sessions.  The key can be anything, and
        it is defined at the moment data is dumped into the file.

        The length of the queue limits the number of samples that are
        kept in memory and, therefore, the number of samples that
        can be kept on disk.

        @type queue_length: number
        @param queue_length: length of queue where samples are kept.
        """

        self.data = None
        self.file_name = None
        self.queue = [None] * queue_length
        self.queue_counter = 0

    def read_file(self, file_name):
        """
        Read file content into class variable.  If the file does not
        exist then create it with an empty dictionary content.

        @type file_name: string
        @param file_name: name of file under TELEMETRY_PATH.
        """
        self.file_name = TELEMETRY_PATH + '/' + file_name
        try:
            self.data = np.load(self.file_name + '.npy', allow_pickle = True).item()
        except FileNotFoundError:
            self.data = {}
            np.save(self.file_name, self.data, allow_pickle = True)
        if type(self.data) != dict:
            self.file_name = None
            raise Exception('object in file is not a dictionary (%s)' % type(self.data))
        
    def reset_queue(self):
        """
        Bring queue to an empty state.  That is, empty the queue and
        reset the counter to zero.
        """
        self.queue = [None] * len(self.queue)
        self.queue_counter = 0

    def enqueue_sample(self, sample):
        """
        Add sample to the queue.  The queue works as a circular
        buffer, samples 'older' than the queue's length are replaced
        by new ones.

        @type sample: any type.
        @param sample: sample value to push into the queue.
        """
        self.queue[self.queue_counter % len(self.queue)] = copy.copy(sample)
        self.queue_counter += 1
        
    def get_queue(self):
        """
        Reorganize the circular queue such that the first entry is the
        oldest collected value and return that.  Items in the queue
        with value None are skipped, because they are slots that never
        received any data.

        @rtype list
        @return list of collected value.
        """
        ret = []
        if self.queue_counter == 0:
            return ret
        for i in range(len(self.queue)):
            idx = (self.queue_counter + i) % len(self.queue)
            if self.queue[idx] is None:
                continue
            ret.append(self.queue[idx])
        return ret

    def get_last_sample(self):
        """
        Get most recent sample. If the queue is empty then throw an exception.

        @rtype same as samples
        @return last sample.
        """
        if self.queue_counter == 0:
            raise Exception('queue is empty')
        return copy.copy(self.queue[(self.queue_counter - 1) % len(self.queue)])
    
    def dump(self, file_name, key, replace = False):
        """
        Dump current queue content to file.

        If the key already exists in the file, then, an exception is
        thrown and the file is left unchanged.

        @type file_name: string
        @param file_name: name of file without path.

        @type key: anything
        @param key: key to individualize this measurement object

        @type replace: boolean
        @param replace: if true and key already exists then replace
            its value
        """
        self.read_file(file_name)
        if key in self.data and replace is False:
            raise Exception('key already exists')
        self.data[key] = self.get_queue()
        np.save(self.file_name, self.data, allow_pickle = True)

    def keys(self):
        if self.file_name is None:
            raise Exception('not connected to file on disk')
        return self.data.keys()
    
    def get(self, key, start = None, stop = None, relative_start_stop = True, ps = None):
        """
        Start and stop parameters are assumed to be an offset respect
        the first sample for the given key, unless absolute_start_stop
        is True.

        If the sampling period (ps) is specifed then interpolate
        missing samples in the returned range.

        @type start: a number
        @param start: starting point to include data.

        @type stop: a number
        @param stop: stopping point to include data.

        @type relative_start_stop: boolean
        @param relative_start_stop: relative or absolute start/stop
            values.

        @type ps: a number
        @param ps: sampling period.
        """
        from scipy.interpolate import splrep, splev
        if self.file_name is None:
            raise Exception('not connected to file on disk')
        if start == None and stop == None:
            data = self.data[key]
            start = self.data[key][0][0]
            stop = self.data[key][-1][0]
        else:
            if start == None:
                if relative_start_stop:
                    start += self.data[key][0][0]
                else:
                    start = self.data[key][0][0]
            if stop == None:
                if relative_start_stop:
                    stop += self.data[key][-1][0]
                else:
                    stop = self.data[key][-1][0]
        data = [i for i in self.data[key] if i[0] >= start and i[0] <= stop]
        if type(ps) is not float and type(ps) is not int:
            return np.array(data)
        dx = float(self.data[key][-1][0] - self.data[key][0][0])
        x = [i[0] for i in self.data[key]]
        xp = np.linspace(self.data[key][0][0], self.data[key][-1][0], dx / ps)
        datap = None
        # for each data column adjust a spline
        for i in range(1, len(self.data[key][0])):
            s = splrep(x, [j[i] for j in self.data[key]], k = 3)
            if datap is None:
                datap = splev(xp, s)
            else:
                datap = np.vstack([datap, splev(xp, s)])
        return np.insert(np.transpose(datap), 0, xp, 1)
            
#___oOo___
