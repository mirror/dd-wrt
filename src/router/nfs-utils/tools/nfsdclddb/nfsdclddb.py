#!/usr/bin/python3
"""Tool for manipulating the nfsdcld sqlite database
"""

__copyright__ = """
Copyright (C) 2019 Scott Mayhew <smayhew@redhat.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
"""

import argparse
import os
import sqlite3
import sys


class CldDb():
    def __init__(self, path):
        self.con = sqlite3.connect(path)
        self.con.row_factory = sqlite3.Row
        for row in self.con.execute('select value from parameters '
                                    'where key = "version"'):
            self.version = int(row['value'])
        for row in self.con.execute('select * from grace'):
            self.current = int(row['current'])
            self.recovery = int(row['recovery'])

    def __del__(self):
        self.con.close()

    def __str__(self):
        return ('Schema version: {self.version} '
                'current epoch: {self.current} '
                'recovery epoch: {self.recovery}'.format(self=self))

    def _print_clients(self, epoch):
        if epoch:
            for row in self.con.execute('select * from "rec-{:016x}"'
                                        .format(epoch)):
                if self.version >= 4:
                    if row['princhash'] is not None:
                        princhash = row['princhash'].hex()
                    else:
                        princhash = "(null)"
                    print('id = {}, princhash = {}'
                          .format(row['id'].decode(), princhash))
                else:
                    print('id = {}'.format(row['id'].decode()))

    def print_current_clients(self):
        print('Clients in current epoch:')
        self._print_clients(self.current)

    def print_recovery_clients(self):
        if self.recovery:
            print('Clients in recovery epoch:')
            self._print_clients(self.recovery)

    def check_bad_table_names(self):
        bad_names = []
        for row in self.con.execute('select name from sqlite_master '
                                    'where type = "table" '
                                    'and name like "%rec-%" '
                                    'and length(name) < 20'):
            bad_names.append(row['name'])
        return bad_names

    def fix_bad_table_names(self):
        try:
            self.con.execute('begin exclusive transaction')
            bad_names = self.check_bad_table_names()
            for bad_name in bad_names:
                epoch = int(bad_name.split('-')[1], base=16)
                if epoch == self.current or epoch == self.recovery:
                    if epoch == self.current:
                        which = 'current'
                    else:
                        which = 'recovery'
                    print('found invalid table name {} for {} epoch'
                          .format(bad_name, which))
                    self.con.execute('alter table "{}" '
                                     'rename to "rec-{:016x}"'
                                     .format(bad_name, epoch))
                    print('renamed to rec-{:016x}'.format(epoch))
                else:
                    print('found invalid table name {} for unknown epoch {}'
                          .format(bad_name, epoch))
                    self.con.execute('drop table "{}"'.format(bad_name))
                    print('dropped table {}'.format(bad_name))
        except sqlite3.Error:
            self.con.rollback()
        else:
            self.con.commit()

    def has_princ_data(self):
        if self.version < 4:
            return False
        for row in self.con.execute('select count(*) '
                                    'from "rec-{:016x}" '
                                    'where princhash not null'
                                    .format(self.current)):
            count = row[0]
        if self.recovery:
            for row in self.con.execute('select count(*) '
                                        'from "rec-{:016x}" '
                                        'where princhash not null'
                                        .format(self.current)):
                count = count + row[0]
        if count:
            return True
        return False

    def _downgrade_table_v4_to_v3(self, epoch):
        if not self.con.in_transaction:
            raise sqlite3.Error
        try:
            self.con.execute('create table "new_rec-{:016x}" '
                             '(id blob primary key)'.format(epoch))
            self.con.execute('insert into "new_rec-{:016x}" '
                             'select id from "rec-{:016x}"'
                             .format(epoch, epoch))
            self.con.execute('drop table "rec-{:016x}"'.format(epoch))
            self.con.execute('alter table "new_rec-{:016x}" '
                             'rename to "rec-{:016x}"'
                             .format(epoch, epoch))
        except sqlite3.Error:
            raise

    def downgrade_schema_v4_to_v3(self):
        try:
            self.con.execute('begin exclusive transaction')
            for row in self.con.execute('select value from parameters '
                                        'where key = "version"'):
                version = int(row['value'])
            if version != self.version:
                raise sqlite3.Error
            for row in self.con.execute('select * from grace'):
                current = int(row['current'])
                recovery = int(row['recovery'])
            if current != self.current:
                raise sqlite3.Error
            if recovery != self.recovery:
                raise sqlite3.Error
            self._downgrade_table_v4_to_v3(current)
            if recovery:
                self._downgrade_table_v4_to_v3(recovery)
            self.con.execute('update parameters '
                             'set value = "3" '
                             'where key = "version"')
            self.version = 3
        except sqlite3.Error:
            self.con.rollback()
            print('Downgrade failed')
        else:
            self.con.commit()
            print('Downgrade successful')


def nfsdcld_active():
    rc = os.system('ps -C nfsdcld >/dev/null 2>/dev/null')
    if rc == 0:
        return True
    return False


def fix_table_names_command(db, args):
    if nfsdcld_active():
        print('Warning: nfsdcld is running!')
        ans = input('Continue? ')
        if ans.lower() not in ['y', 'yes']:
            print('Operation canceled.')
            return
    bad_names = db.check_bad_table_names()
    if not bad_names:
        print('No invalid table names found.')
        return
    db.fix_bad_table_names()


def downgrade_schema_command(db, args):
    if nfsdcld_active():
        print('Warning: nfsdcld is running!')
        ans = input('Continue? ')
        if ans.lower() not in ['y', 'yes']:
            print('Operation canceled')
            return
    if db.version != 4:
        print('Cannot downgrade database from schema version {}.'
              .format(db.version))
        return
    if args.version != 3:
        print('Cannot downgrade to version {}.'.format(args.version))
        return
    bad_names = db.check_bad_table_names()
    if bad_names:
        print('Invalid table names detected.')
        print('Please run "{} fix-table-names" before downgrading the schema.'
              .format(sys.argv[0]))
        return
    if db.has_princ_data():
        print('Warning: database has principal data, which will be erased.')
        ans = input('Continue? ')
        if ans.lower() not in ['y', 'yes']:
            print('Operation canceled')
            return
    db.downgrade_schema_v4_to_v3()


def print_command(db, args):
    print(str(db))
    if not args.summary:
        bad_names = db.check_bad_table_names()
        if bad_names:
            print('Invalid table names detected.')
            print('Please run "{} fix-table-names".'.format(sys.argv[0]))
            return
        db.print_current_clients()
        db.print_recovery_clients()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--path',
                        default='/var/lib/nfs/nfsdcld/main.sqlite',
                        help='path to the database '
                        '(default: /var/lib/nfs/nfsdcld/main.sqlite)')
    subparsers = parser.add_subparsers(help='sub-command help')
    fix_parser = subparsers.add_parser('fix-table-names',
                                       help='fix invalid table names')
    fix_parser.set_defaults(func=fix_table_names_command)
    downgrade_parser = subparsers.add_parser('downgrade-schema',
                                             help='downgrade database schema')
    downgrade_parser.add_argument('-v', '--version', type=int, choices=[3],
                                  default=3,
                                  help='version to downgrade to')
    downgrade_parser.set_defaults(func=downgrade_schema_command)
    print_parser = subparsers.add_parser('print',
                                         help='print database info')
    print_parser.add_argument('-s', '--summary', default=False,
                              action='store_true',
                              help='print summary only')
    print_parser.set_defaults(func=print_command)
    args = parser.parse_args()
    if not os.path.exists(args.path):
        return parser.print_usage()
    clddb = CldDb(args.path)
    return args.func(clddb, args)


if __name__ == '__main__':
    if len(sys.argv) == 1:
        sys.argv.extend(['print', '--summary'])
    main()
