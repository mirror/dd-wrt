#
# Copyright (c) 2015 Plex Development Team. All rights reserved.
#
import re
import os.path
from urllib import urlopen, quote
from xml.dom import minidom
from collections import Counter, defaultdict
import Media
import AudioFiles
import mutagen
from Utils import Log, LevenshteinDistance, LevenshteinRatio, CleanUpString, Unicodize
from UnicodeHelper import toBytes

DEBUG = True
RE_MULTIDISC = re.compile(r'[ \-:]+(?:\[disc|cd) ?([\d]+).*', flags=re.IGNORECASE)
RE_ADDENDUM = re.compile(' ([(\[].+[)\]])$')

def Scan(path, files, media_list, subdirs, language=None, root=None, respect_tags=False):

  # Scan for audio files.
  AudioFiles.Scan(path, files, media_list, subdirs, root)
  
  root_str = root or ''
  loc_str = os.path.join(root_str, path)
  Log('Scanning: ' + loc_str)
  Log('Files: ' + str(files))
  Log('Subdirs: ' + str(subdirs))

  # Look at the files and determine whether we can do a quick match (minimal tag parsing).
  do_quick_match = True
  mixed = False

  # Make sure we're looking at a leaf directory (no audio files below here).
  if len(subdirs) > 0:
    Log('Found directories below this one; won\'t attempt quick matching.')
    do_quick_match = False

  if files:

    # Make sure we're not sitting in the section root.
    parent_path = os.path.split(files[0])[0]
    if parent_path == root:
      Log('File(s) are in section root; doing expensive matching with mixed content.')
      do_quick_match = False
      mixed = True

    # Make sure we have reliable track indices for all files and there are no dupes.
    tracks = {}
    for f in files:
      try: 
        index = re.search(r'^([0-9]{1,2})[^0-9].*', os.path.split(f)[-1]).groups(0)[0]
      except:
        do_quick_match = False
        Log('Couldn\'t find track indices in all filenames; doing expensive matching.')
        break
      if tracks.get(index):
        do_quick_match = False
        mixed = True
        Log('Found duplicate track index: %s; doing expensive matching with mixed content.' % index)
        break
      else:
        tracks[index] = True

    # Read the first track's tags to check for milti-disc and VA.
    if do_quick_match:
      disc = album_artist = None
      try:
        (artist, album, title, track, disc, album_artist, compil) = AudioFiles.getInfoFromTag(files[0], language)
      except:
        Log('Exception reading tags from first file; doing expensive matching.')
        do_quick_match = False

      # Make sure we are on the first disc.
      if disc is not None and disc > 1:
        Log('Skipping quick match because of non-first disc.')
        do_quick_match = False

      # We want to read all the tags for VA albums to pick up track artists.
      if album_artist is not None and album_artist == 'Various Artists':
        Log('Skipping quick match for Various Artists album.')      
        do_quick_match = False

    artist = None
    album = None

    if do_quick_match:
      Log('Doing quick match')
      
      # See if we have some consensus on artist/album by reading a few tags.
      for i in range(3):
        if i < len(files):
          this_artist = this_album = tags = None
          try: tags = mutagen.File(files[i], easy=True)
          except: Log('There was an exception thrown reading tags.')
          
          if tags:
            # See if there's an album artist tag.
            album_artist_tags = [t for t in ['albumartist', 'TPE2', 'performer'] if t in tags]
            album_artist_tag = album_artist_tags[0] if len(album_artist_tags) else None
            
            this_artist = tags[album_artist_tag][0] if album_artist_tag else tags['artist'][0] if 'artist' in tags else None
            this_album = tags['album'][0] if 'album' in tags else None

          if artist and artist != this_artist:
            Log('Found different artists in tags (%s vs. %s); doing expensive matching.' % (artist, this_artist))
            do_quick_match = False
            break

          if album and album != this_album:
            Log('Found different albums in tags (%s vs. %s); doing expensive matching.' % (album, this_album))
            do_quick_match = False
            break

          artist = this_artist
          album = this_album
      
      if not artist or not album:
        Log('Couldn\'t determine unique artist or album from tags; doing expensive matching.')
        do_quick_match = False

    query_list = []
    result_list = []
    fingerprint = False

    # Directory looks clean, let's build a query list directly from info gleaned from file names.
    if do_quick_match:
      Log('Building query list for quickmatch with artist: %s, album: %s' % (artist, album))

      # Determine if the artist and/or album appears in all filenames, since we'll want to strip these out for clean titles.
      strip_artist = True if len([f for f in files if artist.lower() in Unicodize(os.path.basename(f), language).lower()]) == len(files) else False
      strip_album = True if len([f for f in files if album.lower() in Unicodize(os.path.basename(f), language).lower()]) == len(files) else False

      for f in files:
        try:
          filename = os.path.splitext(os.path.split(f)[1])[0]
          (head, index, title) = re.split(r'^([0-9]{1,2})', filename)

          # Replace underscores and dots with spaces.
          title = re.sub(r'[_\. ]+', ' ', title)

          # Things in parens seem to confuse Gracenote, so let's strip them out.
          title = re.sub(r' ?\(.*\)', '', title)

          # Remove artist name from title if it appears in all of them.
          if strip_artist and len(files) > 2:
            title = re.sub(r'(?i)' + artist, '', title)

          # Remove album title from title if it appears in all of them.
          if strip_album and len(files) > 2:
            title = re.sub(r'(?i)' + album, '', title)

          # Remove any remaining index-, artist-, and album-related cruft from the head of the track title.
          title = re.sub(r'^[\W\-]+', '', title).strip()

          # Last chance for artist or album prefix.
          if not strip_artist and Unicodize(title, language).lower().find(artist.lower()) == 0:
            title = title[len(artist):]
            
          if not strip_album and Unicodize(title, language).lower().find(album.lower()) == 0:
            title = title[len(album):]
      
          t = Media.Track(artist=toBytes(artist), album=toBytes(album), title=toBytes(title), index=int(index))
          t.parts.append(f)

          Log(' - Adding: %s - %s' % (index, title))
          query_list.append(t)

        except Exception as e:
          Log('Error preparing tracks for quick matching: ' + str(e))

    # Otherwise, let's do old school directory crawling and tag reading.
    else:
      AudioFiles.Process(path, files, media_list, subdirs, root)
      query_list = list(media_list)
    
    # Try as-is first (ask for everything at once).
    discs = [query_list]
    final_match = run_queries(discs, result_list, language, fingerprint, mixed, do_quick_match)
    
    # If the match was still shitty, and it looks like we have multiple discs, try splitting.
    if final_match < 75:
      discs = group_tracks_by_disc(query_list)
      if len(discs) > 1:
        Log('Result still looked bad, we will try splitting into separate per-disc queries.')
        other_result_list = []
        other_match = run_queries(discs, other_result_list, language, fingerprint, mixed, do_quick_match)
        
        if other_match > final_match:
          Log('The split result was best, we will use it.')
          result_list = other_result_list
          final_match = other_match
        
    # If we have a crappy match, don't use it.
    if final_match < 50.0:
      Log('That was terrible, let us not use it.')
      result_list = []

    # Finalize the results.
    used_tags = False
    del media_list[:]
    if len(result_list) > 0:
      # Gracenote results.
      for result in result_list:
        media_list.append(result)
    else:
      # We bailed during the GN lookup, fall back to tags.
      used_tags = True
      AudioFiles.Process(path, files, media_list, subdirs, root)

    # If we wanted to respect tags, then make sure we used tags.
    if not used_tags and respect_tags:

      # Let's grab tag results, and then set GUIDs we found.
      tag_media_list = []
      AudioFiles.Process(path, files, tag_media_list, subdirs, root)
      
      # Now suck GN data out.
      path_map = {}
      for track in media_list:
        path_map[track.parts[0]] = track
        
      for track in tag_media_list:
        if track.parts[0] in path_map:
          gn_track = path_map[track.parts[0]]
          track.guid = gn_track.guid
          track.album_guid = gn_track.album_guid
          track.artist_guid = gn_track.artist_guid
          track.album_thumb_url = gn_track.album_thumb_url
          track.artist_thumb_url = gn_track.artist_thumb_url
          
          # If the tags failed, fill in key data from Gracenote.
          if track.album == '[Unknown Album]':
            track.album = gn_track.album
          
          if track.artist == '[Unknown Artist]':
            track.artist = gn_track.artist
      
      media_list[:] = tag_media_list

def run_queries(discs, result_list, language, fingerprint, mixed, do_quick_match):

  # Try a text-based match first.
  (match1, albums1, arts1) = run_query_on_discs(discs, result_list, language, fingerprint, mixed, do_quick_match)
  final_match = match1
  
  # If the result looks shoddy, try with fingerprinting.
  if albums1 > len(discs) or match1 < 75 or arts1 == 0:
    Log('Not impressed, trying the other way (fingerprinting: %s)' % (not fingerprint))
    other_result_list = []
    (match2, albums2, arts2) = run_query_on_discs(discs, other_result_list, language, not fingerprint, mixed, do_quick_match)
    
    if match2 > match1 or (match2 == match1 and (albums2 < albums1 or arts2 > arts1)):
      Log('This way gave a better match, keeping.')
      result_list[:] = other_result_list
      final_match = match2
      
  return final_match

def run_query_on_discs(discs, result_list, language, fingerprint, mixed, do_quick_match):
  match1 = albums1 = total_tracks = 0
  for tracks in discs:
    (match, albums1, arts1) = lookup(tracks, result_list, language=language, fingerprint=fingerprint, mixed=mixed, do_quick_match=do_quick_match)
    total_tracks += len(tracks)
    match1 += match * len(tracks)

  if total_tracks > 0:
    match1 = match1 / float(total_tracks)
    Log('Querying all discs generated %d albums and a total match of %d' % (albums1, match1))

  return (match1, albums1, arts1)

def group_tracks_by_disc(query_list):
  tracks_by_disc = defaultdict(list)
  
  # See if we have multiple disks, first checking tags.
  discs = set([t.disc for t in query_list if t.disc is not None])
  if len(discs) > 1:
    for t in query_list:
      tracks_by_disc[t.disc].append(t)
    return tracks_by_disc.values()
  
  # Otherwise, let's sort by filename, and see if we have clusters of tracks.
  sorted_tracks = sorted(query_list, key=lambda track: track.parts[0])
  
  disc = 1
  last_index = 0
  for t in sorted_tracks:
    if t.index < last_index:
      disc += 1
      if t.index != 1:
        Log('Disc %d didn\'t start with first track, we won\'t use this method.' % disc)
        tracks_by_disc = defaultdict(list)
        break
    tracks_by_disc[disc].append(t)
    last_index = t.index
  
  if len(tracks_by_disc) > 1:
    return tracks_by_disc.values()
  
  # Otherwise, let's consider it a single disc.
  return [query_list]

def compute_input_sanity(query_list):
  indexes = defaultdict(list)
  for track in query_list:
    disc = track.disc if track.disc else 1
    indexes[disc].append(track.index)
  
  # See if we have contiguous/unique tracks.
  contiguous = True
  unique = True

  for disc in indexes.keys():
    indexes[disc].sort()
    
    # See if they're contiguous.
    for i, index in enumerate(indexes[disc]):
      if i + 1 != index:
        contiguous = False
        break
  
    # See if they're unique.
    unique = unique and (len(indexes[disc]) == len(set(indexes[disc])))

  # See how many distinct album names.
  unique_albums = len(set([t.album for t in query_list]))

  return (contiguous and unique, unique_albums, len(indexes))

def lookup(query_list, result_list, language=None, fingerprint=False, mixed=False, multiple=False, do_quick_match=False):

  # This shouldn't happen, but be safe.
  if len(query_list) == 0:
    return (0, 0, 0)

  # See if input looks like a sane album
  (sane_input_tracks, unique_input_albums, input_discs) = compute_input_sanity(query_list)

  # Build up the query with the contents of the query list.
  args = ''
  parts = {}

  Log('Running Gracenote match on %d tracks with fingerprinting: %d and mixedContent: %d and multiple: %d' % (len(query_list), fingerprint, mixed, multiple))
  for i, track in enumerate(query_list):
    
    # We need to pass at least a path and an identifier for each track that we know about.
    args += '&tracks[%d].path=%s' % (i, quote(track.parts[0], ''))
    args += '&tracks[%d].userData=%d' % (i, i)
    
    # Keep track of the identifier -> part mapping so we can reassemble later.
    parts[i] = track.parts[0]

    if track.name:
      args += '&tracks[%d].title=%s' % (i, quote(toBytes(track.title or track.name), ''))
    if track.artist and track.artist != 'Various Artists':
      args += '&tracks[%d].artist=%s' % (i, quote(toBytes(track.artist), ''))
    if track.album_artist:
      args += '&tracks[%d].albumArtist=%s' % (i, quote(toBytes(track.album_artist), ''))      
    elif track.artist and track.artist != 'Various Artists':
      args += '&tracks[%d].albumArtist=%s' % (i, quote(toBytes(track.artist), ''))
    if track.album and track.album != '[Unknown Album]':
      args += '&tracks[%d].album=%s' % (i, quote(toBytes(track.album), ''))
    if track.index:
      args += '&tracks[%d].index=%s' % (i, track.index)
    if track.disc:
      args += '&tracks[%d].parentIndex=%s' % (i, track.disc)
    Log(' - %s/%s - %s/%s - %s' % (toBytes(track.artist), toBytes(track.album), toBytes(track.disc), toBytes(track.index), toBytes(track.name)))

  url = 'http://127.0.0.1:32400/services/gracenote/search?fingerprint=%d&mixedContent=%d&multiple=%d%s&lang=%s' % (fingerprint, mixed, multiple, args, language)
  try:
    res = minidom.parse(urlopen(url))
  except Exception, e:
    Log('Error parsing Gracenote response: ' + str(e))
    return (0, 0, 0)

  # See which tracks we got matches for.
  matched_tracks = {track.getAttribute('userData'): track for track in res.getElementsByTagName('Track')}

  # Figure out the unique artists/albums/indexes.
  unique_artists = len(set([t[1].getAttribute('grandparentTitle') for t in matched_tracks.items()]))
  unique_albums = len(set([t[1].getAttribute('parentTitle') for t in matched_tracks.items()]))
  unique_indices = len(set([t[1].getAttribute('index') for t in matched_tracks.items()]))

  if DEBUG:
    Log('Raw track matches:')
    for track in [match[1] for match in matched_tracks.items()]:
      Log('  - %s / %s - %s/%s - %s' %(track.getAttribute('grandparentTitle'), track.getAttribute('parentTitle'), track.getAttribute('parentIndex'), track.getAttribute('index'), track.getAttribute('title')))

  # Look through the results and determine some consensus metadata so we can do a better job of keeping rogue and 
  # unmatched tracks together. We're going to weight matches in the first third of the tracks twice as high, for 
  # cases in which matches come through for the last half of tracks.
  #
  sorted_items = sorted(matched_tracks.items(), key= lambda t: int(t[1].getAttribute('parentIndex') or 1)*100 + int(t[1].getAttribute('index') or -1))
  sorted_items = sorted_items[0:len(sorted_items)/3] + sorted_items
  
  artist_list = [(t[1].getAttribute('grandparentGUID'), t[1].getAttribute('grandparentTitle'), t[1].getAttribute('grandparentThumb')) for t in sorted_items]
  artist_consensus = Counter(artist_list).most_common()[0][0] if len(artist_list) > 0 else ('', '', '')

  album_list = [(t[1].getAttribute('parentGUID'), t[1].getAttribute('parentTitle'), t[1].getAttribute('parentThumb')) for t in sorted_items]
  album_consensus = Counter(album_list).most_common()[0][0] if len(album_list) > 0 else ('', '', '')
  
  year_list = [t[1].getAttribute('year') for t in sorted_items]
  year_consensus = Counter(year_list).most_common()[0][0] if len(year_list) > 0 else -1

  # If the matches are all from a single disc, use it, that way when we merge in missed tracks they won't be left out.
  disc_list = list(set([t[1].getAttribute('parentIndex') for t in sorted_items]))
  disc_consensus = disc_list[0] if len(disc_list) == 1 else 1

  consensus_track = Media.Track(album_guid=album_consensus[0], album=album_consensus[1], album_thumb_url=album_consensus[2], disc=disc_consensus, artist=artist_consensus[1], artist_guid=artist_consensus[0], artist_thumb_url=artist_consensus[2], year=year_consensus)

  # Sanity check the result if we have sane input.
  artist_override = None
  number_of_artists = len(set([q.artist for q in query_list]))
  number_of_matched_tracks = len([i for i,q in enumerate(query_list) if str(i) in matched_tracks])
  
  if (do_quick_match == True or multiple == False and number_of_artists == 1) and query_list[0].artist != '[Unknown Artist]':

    # Start with a medium (and arbitrary) ratio.
    min_ratio = 0.60
    
    clean_query_artist = CleanUpString(query_list[0].artist)
    clean_result_artist = CleanUpString(consensus_track.artist)
    ratio = LevenshteinRatio(clean_query_artist, clean_result_artist)
    Log('Sanity checking lev artist ratio for %s vs. %s; got %f with required minimum of %f' % (clean_query_artist, clean_result_artist, ratio, min_ratio))
    if ratio < min_ratio:
      
      # We're suspicous. Let's check the track titles and album names and see how they matched.
      total_track_ratio = 0
      total_album_ratio = 0
    
      for i, query_track in enumerate(query_list):
        if str(i) in matched_tracks:
          total_track_ratio += compute_track_lev_ratio(query_track, matched_tracks[str(i)])
          total_album_ratio += LevenshteinRatio(matched_tracks[str(i)].getAttribute('parentTitle'), query_track.album)
      
      average_track_ratio = total_track_ratio / len(query_list)
      average_album_ratio = total_album_ratio / len(query_list)
      
      # If we've got really excellent track matches on a good number of tracks, then it's likely
      # that the GN match is just calling the artist different (VA vs artist, etc.) Prefer the name
      # in the tag if we have one and it's consistent.
      #
      track_min_ratio = 0.88
      if average_album_ratio > 0.90:
        track_min_ratio = 0.75
      if average_album_ratio > 0.98 and number_of_matched_tracks == len(query_list):
        track_min_ratio = 0.50
      
      Log('Track average lev ratio %f, album lev ratio %f, required track ratio: %f' % (average_track_ratio, average_album_ratio, track_min_ratio))
      if len(query_list) >= 4 and average_track_ratio > track_min_ratio:
        if number_of_artists == 1:
          Log('Using override artist of %s' % toBytes(query_list[0].artist))
          artist_override = query_list[0].artist
      elif len(query_list) < 4 or average_track_ratio < 0.75 or ratio < 0.20:
        return (0, 0, 0)

  # Check for Various Artists albums which come back matching to an artist, or movie name.
  number_of_album_artists = len(set([q.album_artist for q in query_list if q.album_artist]))
  if number_of_artists > 1 and number_of_album_artists == 1 and query_list[0].album_artist and LevenshteinRatio(query_list[0].album_artist, 'Various Artists') > 0.9:
    Log('Using override artist of Various Artists')
    artist_override = 'Various Artists'

    # Restore track artists from tags if necessary.
    for i, query_track in enumerate(query_list):
      if str(i) in matched_tracks:
        track = matched_tracks[str(i)]
        if query_track.artist and not track.getAttribute('originalTitle'):
          Log('Restoring track artist %s from tags for track %s - %s' % (query_track.artist, query_track.index, query_track.name))
          track.setAttribute('originalTitle', query_track.artist)

  # Add Gracenote results to the result_list where we have them.
  tracks_without_matches = []
  perfect_matches = 0
  track_mismatches = 0
  
  for i, query_track in enumerate(query_list):
    if str(i) in matched_tracks:
      try:
        track = matched_tracks[str(i)]

        final_artist = artist_override or track.getAttribute('grandparentTitle')
        if artist_override:
          consensus_track.artist = artist_override

        # Clean out any spurious track artists that exactly match the album artist.
        if track.getAttribute('originalTitle') == track.getAttribute('grandparentTitle'):
          track.setAttribute('originalTitle', '')

        # Index doesn't match and disc doesn't match and there is more than one album involved.
        if unique_albums > 1 and (query_track.index and int(track.getAttribute('index') or -1) != query_track.index) and (query_track.disc and track.getAttribute('parentIndex') and query_track.disc != int(track.getAttribute('parentIndex') or 1)):
          Log('Both disc (%s -> %s) and track (%s -> %s) mismatched, we\'re going to treat this as a bad match.' % (query_track.disc, track.getAttribute('parentIndex'), int(track.getAttribute('index') or -1), query_track.index))
          tracks_without_matches.append((query_track, parts[i]))
          track_mismatches += 1
          continue

        # If the track index changed, and we didn't perfectly match everything, consider this a bad sign that something
        # went wrong during fingerprint matching and abort.
        #
        if (not query_track.index or query_track.index and int(track.getAttribute('index') or -1) != query_track.index) and (len(matched_tracks) < len(query_list) or unique_albums > 1 or len(matched_tracks) != unique_indices):
          Log('Track index changed (%s -> %s) and match was not perfect, using merged hints.' % (query_track.index, track.getAttribute('index')))
          result_list.append(merge_hints(query_track, consensus_track, parts[i], do_quick_match))
          
          # See how bad the track title mismatch was.
          if compute_track_lev_ratio(query_track, track) > 0.90:
            Log('Even though track index changed, giving partial credit because lev ratio was high')
            perfect_matches += 0.50

          track_mismatches += 1
          continue

        # If we had sane input, but some tracks got put into a different album, don't allow that.
        if sane_input_tracks and track.getAttribute('parentGUID') != consensus_track.album_guid:
          Log('Had sane input but track %s got split, using merged hints.' % track.getAttribute('index'))
          result_list.append(merge_hints(query_track, consensus_track, parts[i], do_quick_match))
          perfect_matches += 0.75
          continue

        t = Media.Track(
          index=int(track.getAttribute('index')),
          album=toBytes(track.getAttribute('parentTitle')),
          artist=toBytes(track.getAttribute('originalTitle') or final_artist),
          title=toBytes(track.getAttribute('title')),
          disc=toBytes(track.getAttribute('parentIndex')),
          album_thumb_url=toBytes(track.getAttribute('parentThumb')),
          artist_thumb_url=toBytes(track.getAttribute('grandparentThumb')),
          year=toBytes(track.getAttribute('year')),
          guid=toBytes(track.getAttribute('guid')),
          album_guid=toBytes(track.getAttribute('parentGUID')),
          artist_guid=toBytes(track.getAttribute('grandparentGUID')))

        # Set the album_artist if we got a track artist and it differs from the album's primary contributor.
        if track.getAttribute('originalTitle') and toBytes(final_artist) != t.artist:
          t.album_artist = toBytes(final_artist)

        t.parts.append(parts[int(track.getAttribute('userData'))])

        if DEBUG:
          #t.name += ' [GN MATCH]'
          if t.album_thumb_url == 'http://':
            t.album_thumb_url = 'https://dl.dropboxusercontent.com/u/8555161/no_album.png'
          if t.artist_thumb_url == 'http://':
            t.artist_thumb_url = 'https://dl.dropboxusercontent.com/u/8555161/no_artist.png'
        
        # Subtract from score if the index didn't match, and use the parsed index, it's likely to be more accurate.
        if query_track.index and int(track.getAttribute('index') or -1) != query_track.index:
          lev = compute_track_lev_ratio(query_track, track)
          
          # Penalize more if disc mismatches as well.
          if query_track.disc and track.getAttribute('parentIndex') and query_track.disc != int(track.getAttribute('parentIndex')):
            # This looks pretty bad, but not as bad if track was a good match.
            perfect_matches += 0.50 if (lev > 0.95) else 0.25
          else:
            # This is less bad, and we steal the index from the query, since otherwise we might end up with dupes.
            perfect_matches += 0.75
            t.index = query_track.index
            
          track_mismatches += 1
        else:
          perfect_matches += 1

        # Add the result.
        Log('Adding matched track: %s / %s / disc %0s track %02d - %s' % (t.artist, t.album, t.disc, t.index, t.name))
        result_list.append(t)

      except Exception, e:
        Log('Error adding track: ' + str(e))

    else:
      Log('Didn\'t get a track match for %s at path: %s' % ((query_track.title or query_track.name), query_track.parts[0]))

      if unique_albums == 1 and unique_artists == 1:
        Log('Other positive Gracenote matches were all from the same artist and album (%s, %s); merging with Gracenote hints.' % (toBytes(consensus_track.artist), toBytes(consensus_track.album)))
        result_list.append(merge_hints(query_track, consensus_track, parts[i], do_quick_match))
      else:
        Log('No matches, just appending query track')
        tracks_without_matches.append((query_track, parts[i]))
        
  # Now consider the unmatched tracks. If they were the minority, then just merge them in.
  if len(tracks_without_matches) / float(len(query_list)) < 0.3:
    if len(tracks_without_matches) > 0:
      Log('Minority of tracks (%d) were unmatched, hooking them back up.' % len(tracks_without_matches))
      result_list.extend([merge_hints(tup[0], consensus_track, tup[1], do_quick_match) for tup in tracks_without_matches])
  else:
    Log('The majority of tracks were unmatched, letting them be.')
    result_list.extend([tup[0] for tup in tracks_without_matches])
    
  # Gracenote often returns albums with [disc 2] etc. in the title, but tracks with disc=1. Use the album title disc number in these cases.
  for t in result_list:
    m = RE_MULTIDISC.search(t.album)
    if m and int(m.group(1)) > 1 and str(t.disc) == '1':
      t.disc = m.group(1)
    t.album = RE_MULTIDISC.sub('', t.album)

  # Multi-disc album titles have cruft in them. Compute a penalty for multiple album titles after trimming it out.
  album_title_penalty = len(set([RE_MULTIDISC.sub('', t.album).strip() for t in result_list]))

  # Compute a score.
  match_percentage = (perfect_matches / float(len(query_list))) * 100.0 - album_title_penalty
  number_of_albums = len(set([track.album_guid for track in result_list]))
  number_of_album_art = reduce(lambda count, (track): count + 1 if track.album_thumb_url is not None and len(track.album_thumb_url) > 0 else 0, result_list, 0)
  track_mismatch_percentage = (track_mismatches/float(len(query_list))) * 100.0

  # If we had a high percentage of track mismatches and a bad album Lev ratio, bail.
  album_lev_ratio = LevenshteinRatio(consensus_track.album, query_list[0].album)
  if track_mismatch_percentage > 50 and album_lev_ratio < .9:
    Log('%d%% of tracks were mismatched and album also looked like a bad match (%s vs. %s, lev ratio %f), won\'t use this result.' % (track_mismatch_percentage, toBytes(consensus_track.album), toBytes(query_list[0].album), album_lev_ratio))
    return (0, 0, 0)

  # Some EPs get matches as the "parent" album. Symptoms include reordered tracks, and generally less tracks.
  if number_of_albums == 1 and (track_mismatch_percentage > 50 or match_percentage < 75) and len(query_list) < 9:
    better_album = improve_from_tag('', query_list[0].parts[0], 'album')
    if len(better_album) > 0:
      for track in result_list:
        track.album = better_album
  
  Log('STAT MATCH PERCENTAGE: %f' % match_percentage)
  Log('STAT ALBUMS MATCHED: %d' % number_of_albums)
  Log('STAT ALBUM ART: %d' % number_of_album_art)
  
  return (match_percentage, number_of_albums, number_of_album_art)

def merge_hints(query_track, consensus_track, part, do_quick_match):

  # If we did a quick match, read tags, as it may have much better tags.
  track_title = query_track.name
  if do_quick_match:
    track_title = improve_from_tag(track_title, part, 'title')

  # We don't want to use consensus disc numbers, since tags are more reliable. It's common for bonus discs, etc. to get "split".
  try: disc = improve_from_tag('1', part, 'discnumber').split('/')[0].split('of')[0].strip()
  except: disc = '1'

  merged_track = Media.Track(
    index=int(query_track.index) if (query_track.index is not None and str(query_track.index).isdigit()) else -1,
    album=toBytes(consensus_track.album),
    artist=toBytes(consensus_track.artist),
    title=toBytes(track_title),
    disc=disc,
    album_thumb_url=toBytes(consensus_track.album_thumb_url),
    artist_thumb_url=toBytes(consensus_track.artist_thumb_url),
    year=toBytes(consensus_track.year),
    album_guid=toBytes(consensus_track.album_guid),
    artist_guid=toBytes(consensus_track.artist_guid))

  merged_track.parts.append(part)

  return merged_track
  
def compute_track_lev_ratio(query_track, matched_track):
  track_title = improve_from_tag(query_track.name, query_track.parts[0], 'title')
  
  # Sometimes tracks end up with artist name.
  if track_title.find(query_track.artist) == 0:
    track_title = track_title[len(query_track.artist):]
    
  other_track_track = matched_track.getAttribute('title')
    
  # Sometimes one track will have a parenthetical and the other one doesn't.
  if RE_ADDENDUM.search(track_title) and not RE_ADDENDUM.search(other_track_track):
    track_title = RE_ADDENDUM.sub('', track_title)
  elif RE_ADDENDUM.search(other_track_track) and not RE_ADDENDUM.search(track_title):
    other_track_track = RE_ADDENDUM.sub('', other_track_track)

  return LevenshteinRatio(other_track_track, track_title)
  
def improve_from_tag(existing, file, tag):
  tags = None
  try: tags = mutagen.File(file, easy=True)
  except: Log('There was an exception thrown reading tags.')

  if tags and tag in tags:
    existing = tags[tag][0]
    
  return toBytes(existing)
