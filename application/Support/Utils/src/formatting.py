#
# System stuff
#
import csv
import os
import subprocess

#
# RAM stuff
#
import embedded.utils
import embedded.mathematics.adc

#-------------------------------------------------------------------------------
def object_to_file(
    file_name,
    obj):
    """
    Using pickle dump python object to file.
    """
    import pickle

    fd = open(file_name, 'wb')

    pickle.dump(obj, fd)

    fd.close()

#-------------------------------------------------------------------------------
def file_to_object(file_name):
    """
    Using pickle read dump file into python object.
    """
    import pickle

    fd = open(file_name, 'rb')

    obj = pickle.load(fd)

    fd.close()
    
    return obj

#-------------------------------------------------------------------------------
def data_to_csv(
    file_name,
    data,
    header = None):
    """
    Second parameter is interpreted as a dictionary, keyed by an integer
    representing the channel number (like 'read' output) and each value a
    list of values to write into the output file. All value lists must have
    sane length.
    """

    #
    # if the first element in data is iterable then assume a list of lists and
    # check that all them has the same length
    #
    try:
        iterator = iter(next(iter(data)))
        dim = len(next(iter(data)))
        dlocal = []
        for i in data:
            if len(i) != dim:
                raise Exception("all data items do not have same length")
            dlocal.append(i)
    except TypeError:
        dim = len(data)
        dlocal = [data]

    with open(file_name, 'w') as fd:
        csvwriter = csv.writer(fd, delimiter = ',')
        if header != None:
            fd.write(header + '\n')
        for j in range(dim):
            csvwriter.writerow([i[j] for i in dlocal])
        fd.close()

#-------------------------------------------------------------------------------
def data_to_vcd(
        file_name,
        samplerate,
        data,
        header = None):
    """
    Write data to disk in VCD format using sigrok to do the actual formatting.
    """

    #
    # create a temprary file with data in csv format. This file is used below to
    # let sigrok do final formatting to vcd.
    #
    tmp = utils.time.timestamp() + '.data2vcd.tmp'
    data_to_csv(os.path.join('/tmp', tmp), data, header)

    #
    # sigrok-cli command
    #
    cmd = "sigrok-cli -I csv:samplerate=100000000" + (":header=true" if header != None else "") + " -i " + os.path.join('/tmp', tmp) + " -O vcd -o " + file_name
    proc = subprocess.Popen(cmd.split(), stderr=subprocess.PIPE)
    retc = proc.wait()

    if retc != 0:
        raise Exception('sigrok-cli has returned error code (%d)' % (retc))

    estr = proc.stderr.read()

    if estr:
        raise Exception(estr)

#___oOo___
