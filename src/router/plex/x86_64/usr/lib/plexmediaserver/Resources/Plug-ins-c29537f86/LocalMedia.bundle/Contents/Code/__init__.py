#local media assets agent
import os, string, hashlib, base64, re, plistlib, unicodedata
import config
import helpers
import localmedia
import audiohelpers
import videohelpers
from collections import defaultdict

from mutagen import File
from mutagen.mp4 import MP4
from mutagen.id3 import ID3
from mutagen.flac import FLAC
from mutagen.flac import Picture
from mutagen.oggvorbis import OggVorbis

PERSONAL_MEDIA_IDENTIFIER = "com.plexapp.agents.none"

GENERIC_ARTIST_NAMES = ['various artists', '[unknown artist]', 'soundtrack', 'ost', 'original sound track', 'original soundtrack', 'original broadway cast']

BLANK_FIELD = '\x7f'

def CleanFilename(path):
  s = os.path.splitext(os.path.basename(path))[0]
  s = CleanString(s)
  s = re.sub(r'^[0-9 \._-]+', '', s)
  return s

def CleanString(s):
  return str(s).strip('\0 ')

def StringOrBlank(s):
  if s is not None:
    s = CleanString(s)
    if len(s) == 0:
      s = BLANK_FIELD
  else:
    s = BLANK_FIELD
  return s

#####################################################################################################################

@expose
def ReadTags(f):
  try:
    return dict(File(f, easy=True))
  except Exception, e:
    Log('Error reading tags from file: %s' % f)
    return {}

#####################################################################################################################

class localMediaMovie(Agent.Movies):
  name = 'Local Media Assets (Movies)'
  languages = [Locale.Language.NoLanguage]
  primary_provider = False
  persist_stored_files = False
  contributes_to = ['com.plexapp.agents.imdb', 'com.plexapp.agents.none']
  
  def search(self, results, media, lang):
    results.Append(MetadataSearchResult(id = 'null', score = 100))
    
  def update(self, metadata, media, lang):

    # Clear out the title to ensure stale data doesn't clobber other agents' contributions.
    metadata.title = None

    part = media.items[0].parts[0]
    path = os.path.dirname(part.file)
    
    # Look for local media.
    try: localmedia.findAssets(metadata, media.title, [path], 'movie', media.items[0].parts)
    except Exception, e: 
      Log('Error finding media for movie %s: %s' % (media.title, str(e)))

    # Look for subtitles
    for item in media.items:
      for part in item.parts:
        localmedia.findSubtitles(part)

    # If there is an appropriate VideoHelper, use it.
    video_helper = videohelpers.VideoHelpers(part.file)
    if video_helper:
      video_helper.process_metadata(metadata)

#####################################################################################################################

def FindUniqueSubdirs(dirs):
  final_dirs = {}
  for dir in dirs:
    final_dirs[dir] = True
    try: 
      parent = os.path.split(dir)[0]
      final_dirs[parent] = True
      try: final_dirs[os.path.split(parent)[0]] = True
      except: pass
    except: pass
    
  if final_dirs.has_key(''):
    del final_dirs['']
  return final_dirs

class localMediaTV(Agent.TV_Shows):
  name = 'Local Media Assets (TV)'
  languages = [Locale.Language.NoLanguage]
  primary_provider = False
  persist_stored_files = False
  contributes_to = ['com.plexapp.agents.thetvdb', 'com.plexapp.agents.none']

  def search(self, results, media, lang):
    results.Append(MetadataSearchResult(id = 'null', score = 100))

  def update(self, metadata, media, lang):

    # Clear out the title to ensure stale data doesn't clobber other agents' contributions.
    metadata.title = None

    # Look for media, collect directories.
    dirs = {}
    for s in media.seasons:
      Log('Creating season %s', s)
      metadata.seasons[s].index = int(s)
      for e in media.seasons[s].episodes:
        
        # Make sure metadata exists, and find sidecar media.
        episodeMetadata = metadata.seasons[s].episodes[e]
        episodeMedia = media.seasons[s].episodes[e].items[0]
        dir = os.path.dirname(episodeMedia.parts[0].file)
        dirs[dir] = True
        
        try: localmedia.findAssets(episodeMetadata, media.title, [dir], 'episode', episodeMedia.parts)
        except Exception, e: 
          Log('Error finding media for episode: %s' % str(e))
        
    # Figure out the directories we should be looking in.
    try: dirs = FindUniqueSubdirs(dirs)
    except: dirs = []
    
    # Look for show images.
    Log("Looking for show media for %s.", metadata.title)
    try: localmedia.findAssets(metadata, media.title, dirs, 'show')
    except: Log("Error finding show media.")
    
    # Look for season images.
    for s in metadata.seasons:
      Log('Looking for season media for %s season %s.', metadata.title, s)
      try: localmedia.findAssets(metadata.seasons[s], media.title, dirs, 'season')
      except: Log("Error finding season media for season %s" % s)
        
    # Look for subtitles for each episode.
    for s in media.seasons:
      # If we've got a date based season, ignore it for now, otherwise it'll collide with S/E folders/XML and PMS
      # prefers date-based (why?)
      if int(s) < 1900 or metadata.guid.startswith(PERSONAL_MEDIA_IDENTIFIER):
        for e in media.seasons[s].episodes:
          for i in media.seasons[s].episodes[e].items:

            # Look for subtitles.
            for part in i.parts:
              localmedia.findSubtitles(part)

              # If there is an appropriate VideoHelper, use it.
              video_helper = videohelpers.VideoHelpers(part.file)
              if video_helper:
                video_helper.process_metadata(metadata, episode = metadata.seasons[s].episodes[e])
      else:
        # Whack it in case we wrote it.
        #del metadata.seasons[s]
        pass

#####################################################################################################################

class localMediaArtistCommon(object):
  name = 'Local Media Assets (Artists)'
  languages = [Locale.Language.NoLanguage]
  primary_provider = False
  persist_stored_files = False

  def update(self, metadata, media, lang, prefs):

    if shouldFindExtras():
      extra_type_map = getExtraTypeMap()

      artist_file_dirs = []
      artist_extras = {}

      metadata.genres.clear()
      album_genres = []
      # First look for track extras.
      checked_tag = False
      for album in media.children:
        for track in album.children:
          part = helpers.unicodize(track.items[0].parts[0].file)
          findTrackExtra(album, track, part, extra_type_map, artist_extras)
          artist_file_dirs.append(os.path.dirname(part))

          audio_helper = audiohelpers.AudioHelpers(track.items[0].parts[0].file)
          if media.title.lower() not in GENERIC_ARTIST_NAMES:
            if audio_helper and hasattr(audio_helper, 'get_track_genres'):
              genres = audio_helper.get_track_genres(prefs)
              for genre in genres:
                if genre not in album_genres:
                  album_genres.append(genre)

          # Look for artist/artist sort field from first track.
          # TODO maybe analyse all tracks and only add title_sort if they are the same.
          if checked_tag == False:
            checked_tag = True
            if audio_helper and hasattr(audio_helper, 'get_artist_title'):
              artist_title = audio_helper.get_artist_title()
              if artist_title:
                # Take a bit of care here to never clear an artist name.
                artist_title = CleanString(artist_title)
                if len(artist_title) > 0:
                  metadata.title = artist_title

            if audio_helper and hasattr(audio_helper, 'get_artist_sort_title'):
              artist_sort_title = audio_helper.get_artist_sort_title()
              metadata.title_sort = ''
              if artist_sort_title and hasattr(metadata, 'title_sort'):
                metadata.title_sort = CleanString(artist_sort_title)

      if prefs['genres'] == 2:
        for genre in album_genres:
          metadata.genres.add(genre)

      # Now go through this artist's directories looking for additional extras and local art.
      checked_artist_path = False
      for artist_file_dir in set(artist_file_dirs):
        path = helpers.unicodize(artist_file_dir)
        findArtistExtras(path, extra_type_map, artist_extras, media.title)

        parentdir = os.path.split(os.path.abspath(path[:-1]))[0]
        name_parentdir = os.path.basename(parentdir)
        artist_has_own_dir = False
        path_to_use = path

        if normalizeArtist(name_parentdir) == normalizeArtist(media.title):
          artist_has_own_dir = True
          path_to_use = parentdir

        if checked_artist_path is False:
          checked_artist_path = True
          path_files = {}

          if len(path_to_use) > 0:
            for p in os.listdir(path_to_use):
              path_files[p.lower()] = p

          # Look for posters and art
          valid_posters = []
          valid_art = []

          # Allow posters named for artist.
          extra_names = [media.title, media.title.replace(' ', '')]

          valid_file_names = getValidFileNamesForArt(config.ARTIST_POSTER_FILES + extra_names, config.ARTIST_PREFIX, artist_has_own_dir)
          for file in valid_file_names:
            if file in path_files.keys():
              data = Core.storage.load(os.path.join(path_to_use, path_files[file]))
              poster_name = hashlib.md5(data).hexdigest()
              valid_posters.append(poster_name)
              if poster_name not in metadata.posters:
                metadata.posters[poster_name] = Proxy.Media(data)
            
          valid_file_names = getValidFileNamesForArt(config.ART_FILES, config.ARTIST_PREFIX, artist_has_own_dir)
          for file in valid_file_names:
            if file in path_files.keys():
              data = Core.storage.load(os.path.join(path_to_use, path_files[file]))
              art_name = hashlib.md5(data).hexdigest()
              valid_art.append(art_name)
              if art_name not in metadata.art:
                metadata.art[art_name] = Proxy.Media(data)

          metadata.art.validate_keys(valid_art)
          metadata.posters.validate_keys(valid_posters)

      for extra in sorted(artist_extras.values(), key = lambda v: (getExtraSortOrder()[type(v)], v.title)):
        metadata.extras.add(extra)


class localMediaArtistLegacy(localMediaArtistCommon, Agent.Artist):
  contributes_to = ['com.plexapp.agents.discogs', 'com.plexapp.agents.lastfm', 'com.plexapp.agents.plexmusic', 'com.plexapp.agents.none', 'tv.plex.agents.music', 'org.musicbrainz.agents.music']

  def search(self, results, media, lang):
    results.Append(MetadataSearchResult(id = 'null', name=media.artist, score = 100))


class localMediaArtistModern(localMediaArtistCommon, Agent.Artist):
  version = 2
  contributes_to = ['com.plexapp.agents.plexmusic']

  def search(self, results, tree, hints, lang='en', manual=False):
    results.add(SearchResult(id='null', type='artist', parentName=hints.artist, score=100))

  def update(self, metadata, media, lang='en', child_guid=None):
    super(localMediaArtistModern, self).update(metadata, media, lang)


class localMediaAlbum(Agent.Album):
  name = 'Local Media Assets (Albums)'
  languages = [Locale.Language.NoLanguage]
  primary_provider = False
  persist_stored_files = False
  contributes_to = ['com.plexapp.agents.discogs', 'com.plexapp.agents.lastfm', 'com.plexapp.agents.plexmusic', 'com.plexapp.agents.none', 'tv.plex.agents.music', 'org.musicbrainz.agents.music']

  def search(self, results, media, lang):
    results.Append(MetadataSearchResult(id = 'null', score = 100))

  def update(self, metadata, media, lang, prefs):
    find_extras = shouldFindExtras()
    extra_type_map = getExtraTypeMap() if find_extras else None
    updateAlbum(metadata, media, lang, find_extras, artist_extras=[], extra_type_map=extra_type_map, prefs = prefs)

def addAlbumImage(meta_set, meta_type, data_file, root_file, data, digest, order):
  if not meta_set.hasSortOrder(digest):
    meta_set[digest] = Proxy.Media(data, order)
    Log('Local asset image added (%s): %s, for file: %s', meta_type, data_file, root_file)
  else:
    Log("Skipping local %s (%s) for file %s since it's already added", meta_type, data_file, root_file)

def updateAlbum(metadata, media, lang, find_extras=False, artist_extras={}, extra_type_map=None, prefs = {}):
      
  # clear out genres for this album so we will get genres for all tracks in audio_helper.process_metadata(metadata)
  metadata.genres.clear()

  valid_posters = []
  valid_art = []
  valid_keys = defaultdict(list)
  valid_track_keys = []
  path = None

  for index, track in enumerate(media.children):
    track_key = track.id or index
    valid_track_keys.append(track_key)

    for item in track.items:
      for part in item.parts:
        filename = helpers.unicodize(part.file)
        path = os.path.dirname(filename)
        (file_root, fext) = os.path.splitext(filename)

        path_files = {}

        if len(path) > 0:
          for p in os.listdir(path):
            path_files[p.lower()] = p

        # Look for posters
        poster_files = config.ALBUM_POSTER_FILES + [ os.path.basename(file_root), helpers.splitPath(path)[-1] ]
        path_file_keys = path_files.keys()
        order = 1
        while len(path_file_keys) > 0:
          data_file = path_file_keys.pop(0)
          if data_file in config.ALBUM_POSTER_DIRS and os.path.isdir(os.path.join(path, path_files[data_file])):
            Log('Searching art subdir %s for file %s', os.path.join(path, path_files[data_file]), filename)
            for p in os.listdir(os.path.join(path, path_files[data_file])):
              p = os.path.join(path_files[data_file], p)
              path_files[p.lower()] = p
              path_file_keys.append(p.lower())
            continue
          poster_match = False
          art_match = False
          (art_base, art_ext) = os.path.splitext(data_file)
          if not art_ext[1:] in config.ART_EXTS:
            continue
          if os.path.dirname(data_file) in config.ALBUM_POSTER_DIRS:
            poster_match = True
          if not poster_match:
            for name in poster_files:
              if art_base.startswith(name):
                poster_match = True
                break
          if not poster_match:
            for name in config.ART_FILES:
              if art_base.startswith(name):
                art_match = True
                break

          # If we only want posters from the cloud, ignore anything we find.
          if prefs['albumPosters'] == 2:
            poster_match = False

          if poster_match or art_match:
            data = Core.storage.load(os.path.join(path, path_files[data_file]))
            digest = hashlib.md5(data).hexdigest()
            (valid_posters if poster_match else valid_art).append(digest)
            addAlbumImage(metadata.posters if poster_match else metadata.art,
                          'poster' if poster_match else 'art',
                          data_file, filename, data, digest, order)
            order = order + 1
        # If there is an appropriate AudioHelper, use it.
        audio_helper = audiohelpers.AudioHelpers(part.file)
        if audio_helper != None:
          try:
            valid_posters = valid_posters + audio_helper.process_metadata(metadata, prefs)
            # Album title (making sure not to blank it out).
            if hasattr(audio_helper, 'get_album_title'):
              album_title = audio_helper.get_album_title()
              if album_title:
                album_title = CleanString(album_title)
                if len(album_title) > 0:
                  metadata.title = album_title

            # Album sort title.
            if hasattr(audio_helper, 'get_album_sort_title'):
              metadata.title_sort = ''
              album_sort_title = audio_helper.get_album_sort_title()
              if album_sort_title and hasattr(metadata, 'title_sort'):
                metadata.title_sort = CleanString(album_sort_title)

            # Album summary
            if hasattr(audio_helper, 'get_album_summary'):
              metadata.summary = ''
              album_summary = audio_helper.get_album_summary()
              if album_summary:
                metadata.summary = CleanString(album_summary)

            if hasattr(audio_helper, 'get_track_sort_title'):
              track_sort_title = audio_helper.get_track_sort_title()
              metadata.tracks[track_key].title_sort = ''
              if track_sort_title and hasattr(metadata.tracks[track_key], 'title_sort'):
                metadata.tracks[track_key].title_sort = CleanString(track_sort_title)

            # Track title
            if hasattr(audio_helper, 'get_track_title'):
              track_title = audio_helper.get_track_title()
              metadata.tracks[track_key].title = ''
              if track_title is not None:
                metadata.tracks[track_key].title = CleanString(track_title)
              else:
                metadata.tracks[track_key].title = CleanFilename(part.file)

            # Track index.
            if hasattr(audio_helper, 'get_track_index'):
              track_index = audio_helper.get_track_index()
              if track_index is not None:
                metadata.tracks[track_key].track_index = track_index

            # Track parent index.
            if hasattr(audio_helper, 'get_track_parent_index'):
              track_parent_index = audio_helper.get_track_parent_index()
              if track_parent_index is not None:
                metadata.tracks[track_key].disc_index = track_parent_index

            # Track artist.
            if hasattr(audio_helper, 'get_track_artist'):
              track_artist = audio_helper.get_track_artist()
              metadata.tracks[track_key].original_title = BLANK_FIELD
              if track_artist is not None:
                metadata.tracks[track_key].original_title = StringOrBlank(track_artist)
          except:
            Log('Exception reading tags.')

        # Look for a video extra for this track.
        if find_extras:
          track_video = findTrackExtra(media, track, helpers.unicodize(part.file), extra_type_map)
          if track_video is not None:
            metadata.tracks[track_key].extras.add(track_video)
        
        # Look for lyrics.
        LYRIC_EXTS = ['txt', 'lrc']
        for ext in LYRIC_EXTS:
          file = (file_root + '.' + ext)
          if os.path.exists(file):
            Log('Found a lyric in %s', file)
            metadata.tracks[track_key].lyrics[file] = Proxy.LocalFile(file, format=ext)
            valid_keys[track_key].append(file)
            
  for key in metadata.tracks:
    metadata.tracks[key].lyrics.validate_keys(valid_keys[key])

  metadata.tracks.validate_keys(valid_track_keys)

  # We can't get rid of local artwork for a very interesting reason—the user might have
  # multiple albums with the same GUID (e.g. normal + deluxe version) and if we validate
  # these keys here, when one is refreshed it'll delete artwork for any others. This matters
  # for artwork because we're storing the original versions here (as opposed to other metadata
  # we write and then read and don't need anymore).
  #
  # There is some added complexity in that server code will take the first item in the list
  # if the field isn't locked, which allows posters to evolve over time. We make sure here
  # that we're sorting the posters we found for this "instance" first. That way if two albums
  # with the same GUID are refreshed, each one gets the right order with its own instances first.
  #
  unique_keys = set(valid_posters)
  metadata.posters.sort_these_keys_first(unique_keys)

  unique_art_keys = set(valid_art)
  metadata.art.sort_these_keys_first(unique_art_keys)

  #metadata.posters.validate_keys(valid_posters)
  #metadata.art.validate_keys(valid_art)
      
def findTrackExtra(album, track, file_path, extra_type_map, artist_extras={}):

  # Look for music videos for this track of the format: "track file name - pretty name (optional) - type (optional).ext"
  file_name = os.path.basename(file_path)
  file_root, file_ext = os.path.splitext(file_name)
  track_videos = []

  if len(file_name) > 0:
    for video in [f for f in os.listdir(os.path.dirname(file_path))
                  if os.path.splitext(f)[1][1:].lower() in config.VIDEO_EXTS
                  and helpers.unicodize(f).lower().startswith(file_root.lower())]:

      video_file, ext = os.path.splitext(video)
      name_components = video_file.split('-')
      extra_type = MusicVideoObject
      if len(name_components) > 1:
        type_component = re.sub(r'[ ._]+', '', name_components[-1].lower())
        if type_component in extra_type_map:
          extra_type = extra_type_map[type_component]
          name_components.pop(-1)

      # Use the video file name for the title unless we have a prettier one.
      pretty_title = '-'.join(name_components).strip()
      if len(pretty_title) - len(file_root) > 0:
        pretty_title = pretty_title.replace(file_root, '')
        if pretty_title.startswith(file_ext):
          pretty_title = pretty_title[len(file_ext):]
        pretty_title = re.sub(r'^[- ]+', '', pretty_title)

      track_video = extra_type(title=pretty_title, file=os.path.join(os.path.dirname(file_path), video))
      artist_extras[video] = track_video

      if extra_type in [MusicVideoObject, LyricMusicVideoObject]:
        Log('Found video %s for track: %s from file: %s' % (pretty_title, file_name, os.path.join(os.path.dirname(file_path), video)))
        track_videos.append(track_video)
      else:
        Log('Skipping track video %s (only regular music videos allowed on tracks)' % video)

  # Check for track video in global area.
  music_video_path = Prefs['music_video_path']
  if len(track_videos) == 0 and music_video_path is not None and len(music_video_path) > 0 and os.path.exists(music_video_path):
    artist_directory = os.path.join(music_video_path, album.parentTitle)
    if os.path.exists(artist_directory):
      potential_videos = [f for f in os.listdir(artist_directory) if os.path.splitext(f)[1][1:].lower() in config.VIDEO_EXTS]
      potential_video_map = {os.path.splitext(key)[0]: key for key in potential_videos}
      if track.title in potential_video_map:
        return MusicVideoObject(title=track.title, file=os.path.join(artist_directory, potential_video_map[track.title]))

  if len(track_videos) > 0:
    track_videos = sorted(track_videos, key = lambda v: (getExtraSortOrder()[type(v)], v.title))
    return track_videos[0]
  else:
    return None


def findArtistExtras(path, extra_type_map, artist_extras, artist_name):

  # Look for other videos in this directory.
  if len(path) > 0:
    for video in [f for f in os.listdir(path)
                  if os.path.splitext(f)[1][1:].lower() in config.VIDEO_EXTS
                  and f not in artist_extras]:

      if video not in artist_extras:
        Log('Found artist video: %s' % video)
        extra = parseArtistExtra(os.path.join(path, video), extra_type_map, artist_name)
        if extra is not None:
          artist_extras[video] = extra

  # Look for artist videos in the custom path if present.
  artist_name = normalizeArtist(artist_name)
  music_video_path = Prefs['music_video_path']
  if music_video_path is not None and len(music_video_path) > 0:
    if not os.path.exists(music_video_path):
      Log('The specified local music video path doesn\'t exist: %s' % music_video_path)
      return
    else:
      local_files = [f for f in os.listdir(music_video_path) 
                     if (os.path.splitext(f)[1][1:].lower() in config.VIDEO_EXTS or os.path.isdir(os.path.join(music_video_path, f)))
                     and normalizeArtist(os.path.basename(f)).startswith(artist_name)
                     and f not in artist_extras]
      for local_file in local_files:

        # Go ahead and add files directly in the specific path matching the "artist - title - type (optional).ext" convention.
        if os.path.isfile(os.path.join(music_video_path, local_file)) and local_file not in artist_extras:
          Log('Found artist video: %s' % local_file)
          extra = parseArtistExtra(os.path.join(music_video_path, local_file), extra_type_map, artist_name)
          if extra is not None:
            artist_extras[local_file] = extra

        # Also add all the videos in the "local video root/artist" directory if we found one.
        elif os.path.isdir(os.path.join(music_video_path, local_file)) and normalizeArtist(os.path.basename(local_file)) == artist_name:
          for artist_dir_file in [f for f in os.listdir(os.path.join(music_video_path, local_file))
                                  if os.path.splitext(f)[1][1:].lower() in config.VIDEO_EXTS
                                  and f not in artist_extras]:
            if artist_dir_file not in artist_extras:
              Log('Found artist video: %s' % artist_dir_file)
              extra = parseArtistExtra(os.path.join(music_video_path, local_file, artist_dir_file), extra_type_map, artist_name)
              if extra is not None:
                artist_extras[artist_dir_file] = extra      


def parseArtistExtra(path, extra_type_map, artist_name):
    
  video_file, ext = os.path.splitext(os.path.basename(path))
  name_components = video_file.split('-')

  # Set the type and whack the type component from the name if we found one. 
  if len(name_components) > 1 and name_components[-1].lower().strip() in extra_type_map:
    extra_type = extra_type_map[name_components.pop(-1).lower().strip()]
  else:
    extra_type = MusicVideoObject

  # Only return concerts if we're new enough.
  if extra_type in [ConcertVideoObject] and not Util.VersionAtLeast(Platform.ServerVersion, 0,9,12,2):
    Log('Found concert, but skipping, not new enough server.')
    return None

  # Whack the artist name if it's the first component and we have more than one.
  if len(name_components) > 1 and normalizeArtist(name_components[0]) == artist_name:
    name_components.pop(0)

  return extra_type(title='-'.join(name_components), file=helpers.unicodize(path))


def normalizeArtist(artist_name):
  try:
    u_artist_name = helpers.unicodize(artist_name)
    ret = ''
    for i in range(len(u_artist_name)):
      if not unicodedata.category(u_artist_name[i]).startswith('P'):
        ret += u_artist_name[i]
    ret = ret.replace(' ', '').lower()
    if len(ret) > 0:
      return ret
    else:
      return artist_name
  except Exception, e:
    Log('Error normalizing artist: %s' % e)
    return artist_name


def shouldFindExtras():
  # Determine whether we should look for video extras.
    try: 
      v = ConcertVideoObject()
      if Util.VersionAtLeast(Platform.ServerVersion, 0,9,12,0):
        find_extras = True
      else:
        find_extras = False
        Log('Not adding extras: Server v0.9.12.0+ required')
    except NameError, e:
      Log('Not adding extras: Framework v2.6.2+ required')
      find_extras = False
    return find_extras


def getExtraTypeMap():
  return {'video' : MusicVideoObject,
          'live' : LiveMusicVideoObject,
          'lyrics' : LyricMusicVideoObject,
          'behindthescenes' : BehindTheScenesObject,
          'interview' : InterviewObject,
          'concert' : ConcertVideoObject }

def getExtraSortOrder():
  return {MusicVideoObject : 0, LyricMusicVideoObject : 1, ConcertVideoObject : 2, LiveMusicVideoObject : 3, BehindTheScenesObject : 4, InterviewObject : 5}


def getValidFileNamesForArt(names, prefix, add_without_prefix):
  # Return the valid file names for the art 
  # given default names and the use of a prefix
  valid_file_names = []
  for ext in config.ART_EXTS:
    for name in names:
      file = (name + '.' + ext).lower()
      if add_without_prefix or name == prefix:
        valid_file_names.append(file)
      file = prefix + '-' + file
      if name != prefix:
        valid_file_names.append(file)
  return valid_file_names
