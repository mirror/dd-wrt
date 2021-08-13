#!/usr/bin/env python
# id3v2.py Version 0.1 (work still in progress)
# $Header: /mnt/haven/Source/Cleaners/ID3v2.py,v 1.4 2004/01/23 08:02:28 elan Exp $
#
# This takes a list of mp3 filenames and spits out an alternative
# filename based on the files id3v2 tag.
# There is another script id3.py which uses the v1 tag.
# I'll integrate it in when I get the time. Shouldnt be too hard
# but I'm often stupidly optimistic like this.
#
# This script only reads id3v2 tags.
# I suppose next enhancement might be to write id3v2 tags.
# and maybe some interaction with CDDB.
# Hmmmm, might need to next write some code to calculate the CDDB id.
#
# Would be nice if u sent me any enhancements/suggestions.
# mailto:calcium@altavista.net
# http://www.ozemail.com.au/~calcium
#
# -----------------
# TODO
# -----------------
# Handle extended headers properly
# Ability to create id3v2 tags.
# Make it more robust.
#
# -----------------
# The documentation.
# -----------------
# I doubt this will be of much use to anyone apart from curiousity value.
# I guess if u want to enhance it to handle additional tags, u'll need
# to write a function called "processXXXX" where XXXX is the frameId.
#
# I also suspect u'll need to have the id3v2 spec to make sense of
# some of the code.
# See http://www.id3.org
# See http://www.python.org
# See http://www.jython.org
# That's it.
#
# Ciao,
# Chai in Melbourne, Australia.
#

import sys
import string
import struct
from UnicodeHelper import fixEncoding

_encodings = ['iso8859-1', 'utf-16', 'utf-16be', 'utf-8']

#
# This gets the id3v2 tag from the file specified.
#
class ID3v2:
	def __init__(self, filename, language=None):

		self.artist = ''
		self.album	= ''
		self.title	= ''
		self.year	= ''
		self.filename			= filename
		self.ok				= 0
		self.track = None
		self.TPE2 = None
		self.disk = None # None unless album has multiple disks
		self.language = language
		
		f				= open(self.filename, 'rb')

		# The header
		self.header			= f.read( 3 )
		if self.header != "ID3":
			return

		# The version is the next 2 bytes
		self.version	= struct.unpack('>bb', f.read(2))[0]

		# The flags. See the id3 v2 spec for details. Am ignoring it.
		self.flags		= f.read(1)

		# I guess I shouldnt ignore the flags but could nt find any test data.
		if ord( self.flags ) != 0:
			print "Hey! There is an extended header present in %s" % filename
			
		# The id3 Tag Size.
		b1, b2, b3, b4	= struct.unpack( '>bbbb', f.read( 4 ) )
		id3Size			= self.syncSafeInt( b1, b2, b3, b4 )

		'''
		# Not ready
		# If there is extended header.
		if ord( self.flags ) != 0:
		# The extended header Size.
		b1, b2, b3, b4		= struct.unpack( '>bbbb', f.read( 4 ) )
		self.extHeaderSize		= self.syncSafeInt( b1, b2, b3, b4 )
		self.extHeaderFlagBytes	= f.read( 1 )
		self.extHeaderExtendedFlags = f.read( 1 )
		print "reading" + str ( self.extHeaderSize )
		self.extHeaderData		= f.read( self.extHeaderSize )
		'''
		
		# Reading in the id3 frames
		while (1) :

			# Assume that the id3size specified in the header is correct.
			if f.tell() >= id3Size:
				break

			if self.version == 2:
				self.frameId		= f.read(3)
				size                = struct.unpack('>bbb', f.read(3))
				self.frameSize		= [size[0]*256*256+size[1]*256+size[2]]
			else:
				self.frameId		= f.read(4)
				self.frameSize		= struct.unpack('>l', f.read(4))

			# incase the id3 size is wrong, break anyway.
			if self.frameSize[0] == 0:
				break

			if self.version > 2:
				# read the frame header flags
				self.frameFlags	= f.read( 2 )
			else:
				self.frameFlags = 0

			blkSize	= self.frameSize[ 0 ]
			if blkSize < 0:
				#print ("[%s] Error in frame size(" + str( blkSize ) + ")") % filename
				break

			if blkSize > 1000000:
				print "Too many bytes (%d) in '%s', aborting read" % (blkSize, filename)
				return
				
			try:
				self.data = f.read( blkSize )
			except:
				print "Error reading %d bytes in %s." % (blkSize, filename)
				break

			# constructing the statement to process the header
			# passing the TAG, EXTFLAGS, DATA as parameters.
			pStr = "self.process" + self.frameId.replace(' ','') \
				   + "( self.frameId, self.frameFlags, self.data )"
			
			try:
				exec pStr
				self.ok			= 1
			except AttributeError:
				#print "Warning: process" + self.frameId + "() unimpl."
				continue
			except:
				print "Warning: strange ID3v2 tag in %s" % filename
				print pStr
				break

		f.close()

	#
	# Gets the filename
	#
	def getFilename( self ):
		return self.filename

	#
	# A guess as to whether file interrogation succeeded
	#
	def isOK( self ):
		return self.ok

	#
	# Gets the version
	#
	def getVersion( self ):
		return self.version

	#
	# Gets the flags
	#
	def getFlags( self ):
#	print "Flags='%x" % ( ord( self.flags ) )
		return self.flags
	#
	# Sets the album name
	#
	def processTALB( self, theString, theFlags, theValue ):
		self.album = fixEncoding( theValue, self.language )
	def processTAL(self, theString, theFlags, theValue):
		self.processTALB(theString, theFlags, theValue)

	def getAlbum( self ):
		return self.album

	#
	# Sets the artist name
	#
	def processTPE1( self, theString, theFlags, theValue ):
		self.artist = fixEncoding( theValue, self.language )
	def processTP1(self, s, f, v):
		self.processTPE1(s,f,v)
		
  #
  # Sets the TPE2
  #
	def processTPE2( self, theString, theFlags, theValue ):
		self.TPE2 = fixEncoding( theValue, self.language )
		
	#
	# Sets the disk
	#
	def processTPOS( self, theString, theFlags, theValue ):
		TPOS = fixEncoding( theValue, self.language )
		try:
			if TPOS == '1/1':
				return
			else:
				sp = TPOS.split('/')
				self.disk = int(sp[0])
		except:
			pass

	def getArtist( self ):
		return self.artist

	#
	# Sets the year.
	#
	def processTYER( self, theString, theFlags, theValue ):
		self.year = fixEncoding( theValue, self.language )
	def processTYE(self, s, f, v):
		self.processTYER(s,f,v)

	#
	# Sets the track
	#
	def processTRCK(self,s,f,v):
		track = fixEncoding(v, self.language)
		slash = track.find('/')
		if slash != -1:
			track = track[0:slash]
		self.track = int(track)
	def processTRK(self,s,f,v):
		self.processTRCK(s,f,v)

	#
	# Sets the title track name
	#
	def processTIT2( self, theString, theFlags, theValue ):
		self.title = fixEncoding( theValue, self.language )
	def processTT2( self, theString, theFlags, theValue ):
		self.processTIT2(theString, theFlags, theValue)

	def getSong( self ):
		return title.song

	def syncSafeInt( self, b1, b2, b3, b4 ):
		return ( b4 & 0xff ) + \
			   + ( ( b3 & 0xff ) << 7 ) \
			   + ( ( b2 & 0xff ) << 14 ) \
			   + ( ( b1 & 0xff ) << 21 ) 
	
import sys
if __name__ == '__main__':
 id3 = ID3v2(sys.argv[1])
 print id3.artist, id3.album, id3.title
