# Unpack archive's object in directory and recreate the archive as
# thin, linking to those objects in the new directory.

def unpack(arch):
    import re
    from os.path import dirname
    from os.path import basename
    from subprocess import call, check_call, check_output, CalledProcessError
    path = dirname(arch)
    lib = re.compile('lib(.*)\.a').match(basename(arch))
    if lib is None:
        raise Exception('unexpected archive name')
    lib = lib.group(1)
    content = re.split('\n', check_output(["arm-none-eabi-ar", "-t", arch]).decode('utf-8'))
    objects = [j for j in content if len(j) > 2 and j[-2:] == '.o']
    archives = [j for j in content if len(j) > 2 and j[-2:] == '.a']
    check_call(["mkdir", "-p", path + '/' + lib])
    check_call(['arm-none-eabi-ar', '-x', arch], cwd = path + '/' + lib)
    check_call(["rm", arch])
    check_call(['arm-none-eabi-ar', '-Trcs'] + ['lib' + lib + '.a'] + [lib + '/' + obj for obj in objects] + archives, cwd = path)

if __name__ == "__main__":
    import sys

    for i in sys.argv[1:]:
        unpack(i)

#___oOo___
