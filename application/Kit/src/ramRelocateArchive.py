# 
# Every object in a MODROOT archive is associated to a library in
# LIBRARIES. This script receives a library name and associated
# objects names. Each object in the archive (thin) is replaced by the
# same one but in INTROOT.
#
def relocate(iroot, mroot, arch, lib, objects):
    import re
    from os.path import dirname
    from os.path import basename
    from subprocess import call, check_call, check_output, CalledProcessError

    for i in objects:
        check_output(["arm-none-eabi-ar", "-Tra", mroot + '/' + i + '.o', iroot + '/' + 'lib' + arch + '.a', iroot + '/' + lib + '/' + i + '.o'])

if __name__ == "__main__":
    import sys

    for i in sys.argv[4:]:
        relocate(sys.argv[1], sys.argv[2], sys.argv[3], i.split()[0], i.split()[1:])
    
#___oOo___
