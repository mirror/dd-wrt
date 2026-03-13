'''
Created on Dec 6, 2009

@author: napier
'''
#import logging
import os
import struct

from atomsearch import find_path, findall_path

#log = logging.getLogger("mp4file")

class EndOFFile(Exception):
    def __init_(self):
        Exception.__init__(self)

def read64(file):
    data = file.read(8)
    if (data is None or len(data) <> 8):
        raise EndOFFile()
    return struct.unpack(">Q", data)[0]

def read32(file):
    data = file.read(4)
    if (data is None or len(data) <> 4):
        raise EndOFFile()
    return struct.unpack(">I", data)[0]

def read16(file):
    data = file.read(2)
    if (data is None or len(data) <> 2):
        raise EndOFFile()
    return struct.unpack(">H", data)[0]

def read8(file):
    data = file.read(1)
    if (data is None or len(data) <> 1):
        raise EndOFFile()
    return struct.unpack(">B", data)[0]

def type_to_str(data):
    a = (data >> 0) & 0xff
    b = (data >> 8) & 0xff
    c = (data >> 16) & 0xff
    d = (data >> 24) & 0xff

    return '%c%c%c%c' % (d, c, b, a)

def parse_atom(file):
    try:
        offset = file.tell()
        size = read32(file)
        type = type_to_str(read32(file))
        if (size == 1):
            size = read64(file)
            
        return create_atom(size, type, offset, file)
    except EndOFFile:
        return None

ATOM_TYPE_MAP = { '\xa9too': 'encoder',
                  '\xa9nam': 'title',
                  '\xa9alb': 'album',
                  '\xa9ART': 'artist',
                  '\xa9art': 'artist',
                  '\xa9cmt': 'comment',
                  '\xa9gen': 'genre',
                  'gnre': 'genre',
                  '\xa9day': 'year',
                  'trkn': 'tracknum',
                  'disk': 'disknum',
                  '\xa9wrt': 'composer',
                  'tmpo': 'bpm',
                  'cptr': 'copyright',
                  'cpil': 'compilation',
                  'covr': 'coverart',
                  'rtng': 'rating',
                  '\xa9grp': 'grouping',
                  'pcst': 'podcast',
                  'catg': 'category',
                  'keyw': 'keyword',
                  'purl': 'podcasturl',
                  'egid': 'episodeguid',
                  'desc': 'description',
                  'ldes': 'long_description',
                  '\xa9lyr': 'lyrics',
                  'tvnn': 'tvnetwork',
                  'tvsh': 'tvshow',
                  'tven': 'tvepisodenum',
                  'tvsn': 'tvseason',
                  'tves': 'tvepisode',
                  'purd': 'purcahsedate',
                  'pgap': 'gapless',
                  }

# There are a lot of atom's with children.  No need to create
# special classes for all of them
ATOM_WITH_CHILDREN = [ 'stik', 'moov', 'trak',
                       'udta', 'ilst', '\xa9too',
                       '\xa9nam', '\xa9alb', '\xa9ART', '\xa9art',
                       '\xa9cmt', '\xa9gen', 'gnre',
                       '\xa9day', 'trkn', 'disk',
                       '\xa9wrt', 'tmpo', 'cptr',
                       'cpil', 'covr', 'rtng',
                       '\xa9grp', 'pcst', 'catg',
                       'keyw', 'purl', 'egid',
                       'desc', 'ldes', '\xa9lyr', 'tvnn',
                       'tvsh', 'tven', 'tvsn',
                       'tves', 'purd', 'pgap',
                      ]

def create_atom(size, type, offset, file):
    clz = type.lower()
    # Possibly remap atom types that aren't valid
    # python variable names
    if (ATOM_TYPE_MAP.has_key(type)):
        clz = ATOM_TYPE_MAP[type]
    if type in ATOM_WITH_CHILDREN:
        return AtomWithChildren(size, type, clz, offset, file)
    try:
        # Try and eval the class into existance
        return eval("%s(size, type, clz, offset, file)" % clz)
    except (NameError, SyntaxError, TypeError):
        # Not defined, use generic Atom
        return Atom(size, type, clz, offset, file)

def parse_atoms(file, maxFileOffset):
    atoms = []
    while file.tell() < maxFileOffset:
        atom = parse_atom(file)

        if not atom or atom.size == 0:
          break

        atoms.append(atom)

        # Seek to the end of the atom
        file.seek(atom.offset + atom.size, os.SEEK_SET)

    return atoms

class Atom(object):
    def __init__(self, size, type, name, offset, file):
        self.size = size
        self.type = type
        self.name = name
        self.offset = offset
        self.file = file
        self.children = []
        self.attrs = {}

    def _set_attr(self, key, value):
        self.attrs[key] = value

    def _set_children(self, children):
        # Tell the children who their parents are
        for child in children:
            child.parent = self
        self.children = children

    def get_attribute(self, key):
        return self.attrs[key]

    def get_atoms(self):
        return self.children

    def find(self, path):
        return find_path(self, path)

    def findall(self, path):
        return findall_path(self, path)

class AtomWithChildren(Atom):
    def __init__(self, size, type, name, offset, file):
        Atom.__init__(self, size, type, name, offset, file)
        self._set_children(parse_atoms(file, offset + size))

class ftyp(Atom):
    def __init__(self, size, type, name, offset, file):
        Atom.__init__(self, size, type, name, offset, file)
        self._set_attr('major_version', type_to_str(read32(file)))
        self._set_attr('minor_version', read32(file))

class meta(Atom):
    def __init__(self, size, type, name, offset, file):
        Atom.__init__(self, size, type, name, offset, file)
        # meta has an extra null after the atom header.  consume it here
        read32(file)
        self._set_children(parse_atoms(file, offset + size))

class data(Atom):
    def __init__(self, size, type, name, offset, file):
        Atom.__init__(self, size, type, name, offset, file)
        
        # Mask off the version field
        self.type = read32(file) & 0xFFFFFF
        
        data = None
        if self.type == 1:
            data = self.parse_string()
            self._set_attr("data", data)
        elif self.type == 21 or self.type == 0:
            # Another random null padding
            read32(self.file)
            data = read32(self.file)
            
            # If this looks big-endian, swap it; I would assume there's an 
            # atom or something that indicates this, but I can't find it.
            #
            if (data & 0xff000000) != 0 and (data & 0xff) == 0:
              data = (data & 0xff000000) >> 24

            self._set_attr("data", data)
        elif self.type == 13 or self.type == 14:
            # Another random null padding
            read32(self.file)
            data = self.file.read(self.size - 16)
            self._set_attr("data", data)
        elif self.type == 22:
            # uint8.
            read32(self.file)
            data = read8(self.file)
            self._set_attr("data", data)
        else:
            print "UNKNOWN TYPE", self.type

    def parse_string(self):
        # consume extra null?
        read32(self.file)
        howMuch = self.size - 16
        return unicode(self.file.read(howMuch), "utf-8", errors='ignore')