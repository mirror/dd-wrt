#!/usr/bin/python

import os, re, sys
from io import open

def list_whence():
    with open('WHENCE', encoding='utf-8') as whence:
        for line in whence:
            match = re.match(r'(?:File|Source):\s*"(.*)"', line)
            if match:
                yield match.group(1)
                continue
            match = re.match(r'(?:File|Source):\s*(\S*)', line)
            if match:
                yield match.group(1)
                continue
            match = re.match(r'Licen[cs]e: (?:.*\bSee (.*) for details\.?|(\S*))\n',
                             line)
            if match:
                if match.group(1):
                    for name in re.split(r', | and ', match.group(1)):
                        yield name
                    continue
                if match.group(2):
                    # Just one word - may or may not be a filename
                    if not re.search(r'unknown|distributable', match.group(2),
                                     re.IGNORECASE):
                        yield match.group(2)
                        continue

def list_git():
    with os.popen('git ls-files') as git_files:
        for line in git_files:
            yield line.rstrip('\n')

def main():
    ret = 0
    whence_list = list(list_whence())
    known_files = set(name for name in whence_list if not name.endswith('/')) | \
                  set(['check_whence.py', 'configure', 'Makefile',
                       'README', 'copy-firmware.sh', 'WHENCE'])
    known_prefixes = set(name for name in whence_list if name.endswith('/'))
    git_files = set(list_git())

    for name in sorted(list(known_files - git_files)):
        sys.stderr.write('E: %s listed in WHENCE does not exist\n' % name)
        ret = 1

    for name in sorted(list(git_files - known_files)):
        # Ignore subdirectory changelogs and GPG detached signatures
        if (name.endswith('/ChangeLog') or
            (name.endswith('.asc') and name[:-4] in known_files)):
            continue

        # Ignore unknown files in known directories
        for prefix in known_prefixes:
            if name.startswith(prefix):
                break
        else:
            sys.stderr.write('E: %s not listed in WHENCE\n' % name)
            ret = 1
    return ret

if __name__ == '__main__':
    sys.exit(main())
