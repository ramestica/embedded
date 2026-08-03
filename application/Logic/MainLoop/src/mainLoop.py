#
# System stuff
#

#
# RAM stuff
#
from embedded.serial.application_commands import ApplicationCommands

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Main-loop commands.')
    parser.add_argument('port', type=str,  nargs='?', default='/dev/xbee', help='serial communication port')
    app = ApplicationCommands(parser.parse_args().port, baud = 115200, timeout = 1)
    app.STATS(0)
    
#___oOo___
