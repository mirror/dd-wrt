#!/usr/bin/env python

from random import random
from subprocess import call
import sys, os

def first_pass(fname):
    includes = []
    for line in file(fname).readlines():
        line = line.strip()
        lparts = line.split(None, 1)
        if len(lparts) < 2 or not lparts[0].startswith('#include'):
            continue
        includes.append(lparts[1])
    if len(includes) > 0:
        return tuple(includes)
    return None

def second_pass(fname_in, fname_out, filter):
    #print 'second_pass', fname_in, fname_out, filter
    fout = file(fname_out, 'w')
    for line in file(fname_in).readlines():
        line_s = line.strip()
        lparts = line_s.split(None, 1)
        if len(lparts) < 2 or not lparts[0].startswith('#include'):
            fout.write(line)
            continue
        if lparts[1] not in filter:
            fout.write(line)
            continue
        fout.write('#if 0\n')
        fout.write(line)
        fout.write('#endif\n')

if __name__ == '__main__':
    make = os.environ['SMAKE']
    make_flags = os.environ['SMAKEFLAGS'].split()
    always_ignore = ('<sys/types.h>', '"config.h"')
    fname = sys.argv[1]
    ignore = list(always_ignore)
    if fname.endswith('.c'):
        ignore.append('"%s.h"' % fname[:-2])
    print 'processing %s' % fname
    includes = first_pass(fname)
    if includes == None:
        print '  ...no includes found'
        sys.exit(0)
    includes = [x for x in includes if x not in ignore]
    includes.sort()
    devnull = file('/dev/null', 'w')
    print ' .collected %d "#include" statements' % len(includes)
    print ' .doing dry run'
    cargs = [make,]
    cargs.extend(make_flags)
    cargs.extend(['-DRTPP_DEBUG', 'clean', 'all'])
    rval = call(cargs, stdout = devnull)
    if rval != 0:
        print '  ...dry run failed'
        sys.exit(1)
    r = int(random() * 1000000.0)
    sfl_includes = []
    fname_bak = '%s.%.6d' % (fname, r)
    os.rename(fname, fname_bak)
    print ' ..renamed "%s" into "%s"' % (fname, fname_bak)
    while True:
        sfl_includes_bak = sfl_includes[:]
        for include in includes:
            if include in sfl_includes:
                continue
            i2 = sfl_includes[:]
            i2.append(include)
            second_pass(fname_bak, fname, i2)
            cargs = [make,]
            cargs.extend(make_flags)
            cargs.extend(['-DRTPP_DEBUG', 'clean', 'all'])
            #print (cargs)
#            rval = call(cargs)
            rval = call(cargs, stdout = devnull, \
              stderr = devnull)
            os.remove(fname)
            if rval == 0:
                sfl_includes.append(include)
                break
        if len(sfl_includes_bak) == len(sfl_includes):
            break
    os.rename(fname_bak, fname)
    for include in sfl_includes:
        print '"#include %s" is superfluous in %s' % (include, fname)
    else:
        sys.exit(0)
    sys.exit(1)

