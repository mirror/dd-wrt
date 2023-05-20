'''
Created on Dec 26, 2009

@author: napier
'''

import re

def path_compare(path, pattern):
    # Handle the simple case
    if pattern.find('*') == -1 and pattern.find('//') == -1:
        return path == pattern
    # Convert pattern into regexp
    regexp = pattern.replace('*', '[^/]+').replace('//', '.*')
    return re.match(regexp, path)

def find_path(atom, findpath):
    if findpath == '.':
        return atom
    if (findpath[0] != '.'):
        findpath = './' + findpath
    for child in atom.children:
        res = find_path_helper(child, findpath, '.', '.')
        if res:
            return res

def find_path_helper(atom, findpath,
                     typepath, namepath,
                     all=False):
    typepath = typepath + '/' + str(atom.type)
    namepath = namepath + '/' + atom.name
    if path_compare(typepath, findpath):
        if all:
            return [atom]
        return atom
    if path_compare(namepath, findpath):
        if all:
            return [atom]
        return atom
    all_res = []
    for child in atom.children:
        res = find_path_helper(child, findpath, typepath, namepath, all)
        if not all and res:
            return res
        if all:
            all_res += res
    if all:
        return all_res
    return None

def findall_path(atom, findpath):
    if findpath == '.':
        return atom
    if (findpath[0] != '.'):
        findpath = './' + findpath
    all_res = []
    for child in atom.children:
        all_res += find_path_helper(child, findpath, '.', '.', True)
    return all_res