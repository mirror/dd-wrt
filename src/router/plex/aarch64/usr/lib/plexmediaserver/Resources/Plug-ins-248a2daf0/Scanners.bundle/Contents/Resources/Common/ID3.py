# ID3.py version 1.0

# Module for manipulating ID3 informational tags in MP3 audio files
# $Id: ID3.py,v 1.1 2002/09/10 21:04:52 elan Exp $

# Written 2 May 1999 by Ben Gertzfield <che@debian.org>
# This work is released under the GNU GPL, version 2 or later.

# Modified 10 June 1999 by Arne Zellentin <arne@unix-ag.org> to
# fix bug with overwriting last 128 bytes of a file without an
# ID3 tag

# Patches from Jim Speth <speth@end.com> and someone whose email
# I've forgotten at the moment (huge apologies, I didn't save the
# entire mail, just the patch!) for so-called ID3 v1.1 support,
# which makes the last two bytes of the comment field signify a
# track number. If the first byte is null but the second byte
# is not, the second byte is assumed to signify a track number.

# Also thanks to Jim for the simple function to remove nulls and
# whitespace from the ends of ID3 tags. I'd like to add a boolean
# flag defaulting to false to the ID3() constructor signifying whether
# or not to remove whitespace, just in case old code depended on the
# old behavior for some reason, but that'd make any code that wanted
# to use the stripping behavior not work with old ID3.py. Bleh.

# This is the first thing I've ever written in Python, so bear with
# me if it looks terrible. In a few years I'll probably look back at
# this and laugh and laugh..

# Constructor:
#
#   ID3(filename)
#     Opens filename and tries to parse its ID3 header. If the ID3 header
#     is invalid or the file access failed, raises InvalidTagError.
#
#     When object is deconstructed, if any of the class data (below) have
#     been changed, opens the file again read-write and writes out the
#     new header. If the header is to be deleted, truncates the last
#     128 bytes of the file.
#
#     Note that if ID3 cannot write the tag out to the file upon
#     deconstruction, InvalidTagError will be raised and ignored
#     (as we are in __del__, and exceptions just give warnings when
#     raised in __del__.)

# Class Data of Interest:
#
#   Note that all ID3 fields, unless otherwise specified, are a maximum of
#   30 characters in length. If a field is set to a string longer than
#   the maximum, it will be truncated when it's written to disk.
#
#   ID3.title
#     Title of the song.
#   ID3.artist
#     Artist/creator of the song.
#   ID3.album
#     Title of the album the song is from.
#   ID3.year
#     Year the song was released. Maximum of 4 characters (Y10K bug!)
#   ID3.genre
#     Genre of the song. Integer value from 0 to 255. Genre specification
#     comes from (sorry) WinAMP. http://mp3.musichall.cz/id3master/faq.htm
#     has a list of current genres; I spell-checked this list against
#     WinAMP's by running strings(1) on the file Winamp/Plugins/in_mp3.dll 
#     and made a few corrections.
#   ID3.comment
#     Comment about the song.
#   ID3.track
#     Track number of the song. None if undefined.
#
#   ID3.genres
#     List of all genres. ID3.genre above is used to index into this
#     list. ID3.genres is current as of WinAMP 1.92.

# Methods of Interest:
#
#   write()
#     If the class data above have changed, opens the file given
#     to the constructor read-write and writes out the new header.
#     If the header is flagged for deletion (see delete() below)
#     truncates the last 128 bytes of the file to remove the header.
#
#     NOTE: write() is called from ID3's deconstructor, so it's technically
#     unnecessary to call it. However, write() can raise an InvalidTagError,
#     which can't be caught during deconstruction, so generally it's 
#     nicer to call it when writing is desired.
#   
#   delete()
#     Flags the ID3 tag for deletion upon destruction of the object
#   
#   find_genre(genre_string)
#     Searches for the numerical value of the given genre string in the
#     ID3.genres table. The search is performed case-insensitively. Returns
#     an integer from 0 to len(ID3.genres).
#

import string
import re

def lengthen(string, num_spaces):
    string = string[:num_spaces]
    return string + ('\0' * (num_spaces - len(string)))

# We would normally use string.rstrip(), but that doesn't remove \0 characters.
def strip_padding(s):
    try:
      s = s.decode('iso-8859-1').encode('utf-8')
    except:
      pass
  
    while len(s) > 0 and s[-1] in (string.whitespace + "\0"):
        s = s[:-1]

    # Get rid of everything after \0...the above code doesn't always
    # work.
    #
    rx = re.compile("\000.*");
    s = rx.sub("", s)

    # Change \222 to ' (must be strange keyboard)
    s = s.replace("\222", "'")

    return s
        
class InvalidTagError:
    def __init__(self, msg):
	self.msg = msg
    def __str__(self):
	return self.msg

class ID3:

    genres = [ 
	"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", 
	"Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other", 
	"Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial", 
	"Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack", 
	"Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk", "Fusion", 
	"Trance", "Classical", "Instrumental", "Acid", "House", "Game", 
	"Sound Clip", "Gospel", "Noise", "Alt. Rock", "Bass", "Soul", 
	"Punk", "Space", "Meditative", "Instrum. Pop", "Instrum. Rock", 
	"Ethnic", "Gothic", "Darkwave", "Techno-Indust.", "Electronic", 
	"Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy", 
	"Cult", "Gangsta", "Top 40", "Christian Rap", "Pop/Funk", "Jungle", 
	"Native American", "Cabaret", "New Wave", "Psychadelic", "Rave", 
	"Showtunes", "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", 
	"Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock", "Folk", 
	"Folk/Rock", "National Folk", "Swing", "Fusion", "Bebob", "Latin", 
	"Revival", "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock", 
	"Progress. Rock", "Psychadel. Rock", "Symphonic Rock", "Slow Rock", 
	"Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", 
	"Speech", "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony", 
	"Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam", 
	"Club", "Tango", "Samba", "Folklore", "Ballad", "Power Ballad", 
	"Rhythmic Soul", "Freestyle", "Duet", "Punk Rock", "Drum Solo", 
	"A Capella", "Euro-House", "Dance Hall", "Goa", "Drum & Bass", 
	"Club-House", "Hardcore", "Terror", "Indie", "BritPop", "Negerpunk", 
	"Polsk Punk", "Beat", "Christian Gangsta Rap", "Heavy Metal", 
	"Black Metal", "Crossover", "Contemporary Christian", "Christian Rock",
	"Merengue", "Salsa", "Thrash Metal", "Anime", "Jpop", "Synthpop" 
	]

    def __init__(self, filename):
	self.filename = filename
	self.delete_tag = 0
	self.zero()
	self.modified = 0
	self.has_tag = 0
	self.had_tag = 0
	
	try:
	    self.file = open(filename, 'rb')
	    self.file.seek(-128, 2)

	except IOError, msg:
	    self.modified = 0
	    raise InvalidTagError("Can't open %s: %s" % (filename, msg))
	    return

	try:
	    if self.file.read(3) == 'TAG':
		self.has_tag = 1
		self.had_tag = 1
		self.title = self.file.read(30)
		self.artist = self.file.read(30)
		self.album = self.file.read(30)
		self.year = self.file.read(4)
		self.comment = self.file.read(30)

                if ord(self.comment[-2]) == 0 and ord(self.comment[-1]) != 0:
                    self.track = ord(self.comment[-1])
                    self.comment = self.comment[:-2]
                else:
                    self.track = None

		self.genre = ord(self.file.read(1))
		self.file.close()

                self.title = strip_padding(self.title)
                self.artist = strip_padding(self.artist)
                self.album = strip_padding(self.album)
                self.year = strip_padding(self.year)
                self.comment = strip_padding(self.comment)
                
	except IOError, msg:
	    self.modified = 0
	    raise InvalidTagError("Invalid ID3 tag in %s: %s" % (filename, msg))
	self.modified = 0

    def delete(self):
	self.zero()
	self.delete_tag = 1
	self.has_tag = 0

    def zero(self):
	self.title = ''
	self.artist = ''
	self.album = ''
	self.year = ''
	self.comment = ''
        self.track = None
	self.genre = 0
        
    def find_genre(self, genre_to_find):
	i = 0
	find_me = string.lower(genre_to_find)

	for genre in self.genres:
	    if string.lower(genre) == find_me:
		break
	    i = i + 1
	if i == len(self.genres):
	    return -1
	else:
	    return i

    def write(self):
	if self.modified:
	    try:
		self.file = open(self.filename, 'rb+')
		if self.had_tag:
                    self.file.seek(-128, 2)
                else:
                    self.file.seek(0, 2) # a new tag is appended at the end
		if self.delete_tag and self.had_tag:
		    self.file.truncate()
                    self.had_tag = 0
		elif self.has_tag:
                    go_on = 1
                    if self.had_tag:
                        #if self.file.read(3) == "TAG":
                        self.file.seek(-128, 2)
                        #else:
                            # someone has changed the file in the mean time
                        #    go_on = 0
                        #    raise IOError("File has been modified, losing tag changes")
                    if go_on:
		        self.file.write('TAG')
		        self.file.write(lengthen(self.title, 30))
		        self.file.write(lengthen(self.artist, 30))
		        self.file.write(lengthen(self.album, 30))
		        self.file.write(lengthen(self.year, 4))

                        comment = lengthen(self.comment, 30)

                        if self.track < 0 or self.track > 255:
                            self.track = None

                        if self.track != None:
                            comment = comment[:-2] + "\0" + chr(self.track)

                        self.file.write(comment)

		        if self.genre < 0 or self.genre > 255:
			    self.genre = 255
		        self.file.write(chr(self.genre))
                        self.had_tag = 1
		self.file.close()
	    except IOError, msg:
		raise InvalidTagError("Cannot write modified ID3 tag to %s: %s" % (self.filename, msg))
	    else:
		self.modified = 0

    def __del__(self):
	self.write()

    def __str__(self):
	if self.has_tag:
	    if self.genre != None and self.genre > 0 and self.genre < len(self.genres):
		genre = self.genres[self.genre]
	    else:
		genre = 'Unknown'

            if self.track != None:
                track = str(self.track)
            else:
                track = 'Unknown'

	    return "File   : %s\nTitle  : %-30.30s  Artist: %-30.30s\nAlbum  : %-30.30s  Track : %s  Year: %-4.4s\nComment: %-30.30s  Genre : %s (%i)" % (self.filename, self.title, self.artist, self.album, track, self.year, self.comment, genre, self.genre)
	else:
	    return "%s: No ID3 tag." % self.filename

    # intercept setting of attributes to set self.modified
    def __setattr__(self, name, value):
	if name in ['title', 'artist', 'album', 'year', 'comment',
                    'track', 'genre']:
	    self.__dict__['modified'] = 1
	    self.__dict__['has_tag'] = 1
	self.__dict__[name] = value

import sys
if __name__ == '__main__':
  id3 = ID3(sys.argv[1])
  print id3.artist, id3.album, id3.title