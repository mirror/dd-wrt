#
# Copyright (c) 2019 Plex Development Team. All rights reserved.
#

Languages = [Locale.Language.English, Locale.Language.Arabic, Locale.Language.Bulgarian, Locale.Language.Chinese, Locale.Language.Croatian,
             Locale.Language.Czech, Locale.Language.Danish, Locale.Language.Dutch, Locale.Language.Finnish, Locale.Language.French,
             Locale.Language.German, Locale.Language.Greek, Locale.Language.Hungarian, Locale.Language.Indonesian, Locale.Language.Italian,
             Locale.Language.Japanese, Locale.Language.Korean, Locale.Language.NorwegianNynorsk, Locale.Language.Polish,
             Locale.Language.Portuguese, Locale.Language.Romanian, Locale.Language.Russian, Locale.Language.Serbian, Locale.Language.Slovak,
             Locale.Language.Spanish, Locale.Language.Swedish, Locale.Language.Thai, Locale.Language.Turkish, Locale.Language.Vietnamese,
             Locale.Language.Unknown]

def Start():
  pass

##################################################################################
class MusicbrainzArtistAgent(Agent.Artist):
  name = 'Musicbrainz'
  languages = Languages

  def search(self, results, media, lang='en', manual=False, tree=None, primary=True):
    pass

  def update(self, metadata, media, lang):
    pass

##################################################################################
class PlexMusicAlbumAgent(Agent.Album):
  name = 'Musicbrainz'
  languages = Languages

  def search(self, results, media, lang, manual=False, tree=None, primary=False):
    pass

  def update(self, metadata, media, lang):
    pass
