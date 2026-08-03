#
# Would I ever have a declaration statement? 
#

if __name__ == "__main__":
    from optparse import OptionParser

    parser = OptionParser(usage="usage: %prog -c <command> filename")

    parser.add_option('-c', '--command', type=str, help='command to execute to produce output that contains the entry point address.')

    (options, args) = parser.parse_args()
    
    if options.command is None:
        parser.error("missing option")

    if len(args) != 1:
        parser.error("wrong number of arguments")

    import sys
    import subprocess
    import re

    rex = re.compile('.*Entry point address.*0x([0-9a-fA-F]+).*$')
    cmd = subprocess.Popen(options.command + ' ' + args[0], shell=True, stdout=subprocess.PIPE)
    for l in cmd.stdout:
        res = rex.match(l)
        if res is None:
            continue
        print('0x' + res.group(1))
        sys.exit(0)

    #
    # not found then return with error
    #
    sys.exit(1)
    
# ___oOo___
