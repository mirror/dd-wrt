'''
Created on Dec 6, 2009

@author: napier
'''
from atom import parse_atoms, AtomWithChildren
#import logging
import os

#log = logging.getLogger("mp4file")

def getFileSize(file):
    file.seek(0, os.SEEK_END)
    endFile = file.tell()
    file.seek(0, os.SEEK_SET)
    return endFile

class Mp4File(AtomWithChildren):
    def __init__(self, filename):
        file = open(filename, "rb")
        self.atoms = parse_atoms(file, getFileSize(file))
        AtomWithChildren.__init__(self, getFileSize(file),
                                  '', '', 0, file)