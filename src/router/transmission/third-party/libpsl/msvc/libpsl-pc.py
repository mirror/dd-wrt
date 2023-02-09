# !/usr/env/python
# Simple script to generate libpsl.pc from libpsl.pc.in
# for Visual Studio builds

import sys
import argparse

from replace import replace_multi
from pc_base import BasePCItems

def main(argv):
    parser = argparse.ArgumentParser(description='Setup basic libpsl.pc file info')
    parser.add_argument('--name', help='Name of package', required=True)
    parser.add_argument('--url', help='Package Home Page URL', required=True)

    base_pc = BasePCItems()
    base_pc.setup(argv, parser)
    args = parser.parse_args()

    pc_replace_items = {'@PACKAGE_NAME@': args.name,
                        '@PACKAGE_VERSION@': args.version,
                        '@PACKAGE_URL@': args.url}

    pc_replace_items.update(base_pc.base_replace_items)

    # Generate libpsl.pc
    replace_multi(base_pc.top_srcdir + '/libpsl.pc.in',
                  base_pc.srcdir + '/libpsl.pc',
                  pc_replace_items)

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
