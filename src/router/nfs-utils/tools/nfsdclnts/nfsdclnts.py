#!/usr/bin/python3
# -*- python-mode -*-
'''
    Copyright (C) 2020
    Authors:    Achilles Gaikwad <agaikwad@redhat.com>
                Kenneth  D'souza <kdsouza@redhat.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
'''

import multiprocessing as mp
import os
import signal
import sys

try:
    import argparse
except ImportError:
    print('%s:  Failed to import argparse - make sure argparse is installed!'
        % sys.argv[0])
    sys.exit(1)
try:
    import yaml
except ImportError:
    print('%s:  Failed to import yaml - make sure python3-pyyaml is installed!'
        % sys.argv[0])
    sys.exit(1)

BBOLD = '\033[1;30;47m' #Bold black text with white background.
ENDC = '\033[m' #Rest to defaults

def init_worker():
    signal.signal(signal.SIGINT, signal.SIG_IGN)

# this function converts the info file to a dictionary format, sorta. 
def file_to_dict(path):
    client_info = {}
    try:
        with open(path) as f:
            for line in f:
                try:
                    (key, val) = line.split(':', 1)
                    client_info[key] = val.strip()
                    # FIXME: There has to be a better way of converting the info file to a dictionary.
                except ValueError as reason:
                    if verbose:
                        print('Exception occured, %s' % reason)

        if len(client_info) == 0 and verbose:
            print("Provided %s file is not valid" %path)
        return client_info

    except OSError as reason:
        if verbose:
            print('%s' % reason)

# this function gets the paths from /proc/fs/nfsd/clients/
# returns a list of paths for each client which has nfs-share mounted.
def getpaths():
    path = []
    try:
        dirs = os.listdir('/proc/fs/nfsd/clients/')
    except OSError as reason:
        exit('%s' % reason)
    if len(dirs) !=0:
	    for i in dirs:
                path.append('/proc/fs/nfsd/clients/' + i + '/states')
	    return (path)
    else:
        exit('Nothing to process')

# A single function to rule them all, in this function we gather all the data
# from already populated data_list and client_info.
def printer(data_list, argument):
    client_info_path = data_list.pop()
    client_info = file_to_dict(client_info_path)
    for i in data_list:
        for key in i:
            inode = i[key]['superblock'].split(':')[-1]
            # The ip address is quoted, so we dequote it.
            try:
                client_ip = client_info['address'][1:-1]
            except:
                client_ip = "N/A"
            try:
                # if the nfs-server reboots while the nfs-client holds the files open,
                # the nfs-server would print the filename as '/'. For such instaces we
                # print the output as disconnected dentry instead of '/'.
                if(i[key]['filename']=='/'):
                    fname = 'disconnected dentry'
                else:
                    fname = i[key]['filename'].split('/')[-1]
            except KeyError:
                # for older kernels which do not have the fname patch in kernel, they
                # won't be able to see the fname field. Therefore post it as N/A.
                fname = "N/A"
            otype = i[key]['type']
            try:
                access = i[key]['access']
            except:
                access = ''
            try:
                deny = i[key]['deny']
            except:
                deny = ''
            try:
                hostname = client_info['name'].split()[-1].split('"')[0]
                hostname =  hostname.split('.')[0]
                # if the hostname is too long, it messes up with the output being in columns,
                # therefore we truncate the hostname followed by two '..' as suffix.
                if len(hostname) > 20:
                    hostname = hostname[0:20] + '..'
            except:
                hostname = "N/A"
            try:
                clientid = client_info['clientid']
            except:
                clientid = "N/A"
            try:
                minorversion = "4." + client_info['minor version']
            except:
                minorversion = "N/A"

            otype = i[key]['type']
            # since some fields do not have deny column, we drop those if -t is either
            # layout or lock.
            drop = ['layout', 'lock']

            # Printing the output this way instead of a single string which is concatenated
            # this makes it better to quickly add more columns in future.
            if(otype == argument.type or  argument.type == 'all'):
                print('%-13s' %inode, end='| ')
                print('%-7s' %otype, end='| ')
                if (argument.type not in drop):
                    print('%-7s' %access, end='| ')
                if (argument.type not in drop and argument.type !='deleg'):
                    print('%-5s' %deny, end='| ')
                if (argument.hostname == True):
                    print('%-22s' %hostname, end='| ')
                else:
                   print('%-22s' %client_ip, end='| ')
                if (argument.clientinfo == True) :
                    print('%-20s' %clientid, end='| ')
                    print('%-5s' %minorversion, end='| ')
                print(fname)

def opener(path):
    try:
        with open(path, 'r') as nfsdata:
            try:
                data = yaml.load(nfsdata, Loader = yaml.BaseLoader)
                if data is not None:
                    clientinfo = path.rsplit('/', 1)[0] + '/info'
                    data.append(clientinfo)
                return data
            except:
                if verbose:
                    print("Exception occurred, Please make sure %s is a YAML file" %path)

    except OSError as reason:
        if verbose:
            print('%s' % reason)

def print_cols(argument):
    title_inode = 'Inode number'
    title_otype = 'Type'
    title_access = 'Access'
    title_deny = 'Deny'
    title_fname = 'Filename'
    title_clientID = 'Client ID'
    title_hostname = 'Hostname'
    title_ip = 'ip address'
    title_nfsvers = 'vers'

    drop = ['lock', 'layout']
    print(BBOLD, end='')
    print('%-13s' %title_inode, end='| ')
    print('%-7s' %title_otype, end='| ')
    if (argument.type not in drop):
        print('%-7s' %title_access, end='| ')
    if (argument.type not in drop and argument.type !='deleg'):
        print('%-5s' %title_deny, end='| ')
    if (argument.hostname == True):
        print('%-22s' %title_hostname, end='| ')
    else:
        print('%-22s' %title_ip, end='| ')
    if (argument.clientinfo == True):
        print('%-20s' %title_clientID, end='| ')
        print('%-5s' %title_nfsvers, end='| ')
    print(title_fname, end='')
    print(ENDC)

def nfsd4_show():

    parser = argparse.ArgumentParser(description = 'Parse the nfsd states and clientinfo files.')
    parser.add_argument('-t', '--type', metavar = 'type', type = str, choices = ['open',
        'deleg', 'lock', 'layout', 'all'],
        default = 'all',
        help = 'Input the type that you want to be printed: open, lock, deleg, layout, all')
    parser.add_argument('--clientinfo', action = 'store_true',
        help = 'output clients information, --hostname is implied.')
    parser.add_argument('--hostname', action = 'store_true',
        help = 'print hostname of client instead of its ip address. Longer hostnames are truncated.')
    parser.add_argument('-v', '--verbose', action = 'store_true',
        help = 'Verbose operation, show debug messages.')
    parser.add_argument('-f', '--file', nargs='+', type = str, metavar='',
        help = 'pass client states file, provided that info file resides in the same directory.')
    parser.add_argument('-q', '--quiet', action = 'store_true',
        help = 'don\'t print the header information')

    args = parser.parse_args()

    global verbose
    verbose = False
    if args.verbose:
        verbose = True

    if args.file:
        paths = args.file
    else:
        paths = getpaths()

    p = mp.Pool(mp.cpu_count(), init_worker)
    try:
        result = p.map(opener, paths)
        ### Drop None entries from list
        final_result = list(filter(None, result))
        p.close()
        p.join()

        if len(final_result) !=0 and not args.quiet:
            print_cols(args)

        for item in final_result:
            printer(item, args)

    except KeyboardInterrupt:
        print('Caught KeyboardInterrupt, terminating workers')
        p.terminate()
        p.join()

if __name__ == "__main__":
    nfsd4_show()
