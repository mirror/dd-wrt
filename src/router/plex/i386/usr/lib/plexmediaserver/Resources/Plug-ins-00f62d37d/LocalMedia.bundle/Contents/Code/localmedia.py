import os, unicodedata
import config
import helpers
import subtitlehelpers

#####################################################################################################################

def findAssets(metadata, media_title, paths, type, parts=[]):

  ignore_samples = ['[-\._ ]sample', 'sample[-\._ ]']
  ignore_trailers = ['-trailer\.']

  # Do a quick check to make sure we've got the extra types available in this framework version,
  # and that the server is new enough to support them.
  #
  try: 
    t = InterviewObject()
    if Util.VersionAtLeast(Platform.ServerVersion, 0,9,9,13):
      find_extras = True
    else:
      find_extras = False
      Log('Not adding extras: Server v0.9.9.13+ required')
  except NameError, e:
    Log('Not adding extras: Framework v2.5.0+ required')
    find_extras = False

  if find_extras:
    extra_type_map = {'trailer' : TrailerObject,
                'deleted' : DeletedSceneObject,
                'behindthescenes' : BehindTheScenesObject,
                'interview' : InterviewObject,
                'scene' : SceneOrSampleObject,
                'featurette' : FeaturetteObject,
                'short' : ShortObject,
                'other' : OtherObject}

  # We start by building a dictionary of files to their absolute paths. We also need to know
  # the number of media files that are actually present, in case the found local media asset 
  # is limited to a single instance per media file.
  #
  path_files = {}
  multi_parts = []
  total_media_files = 0
  root_file = getRootFile(helpers.unicodize(parts[0].file)) if parts else None
  for path in paths:
    path = helpers.unicodize(path)
    for file_path in sorted(os.listdir(path)):

      # When using os.listdir with a unicode path, it will always return a string using the
      # NFD form. However, we internally are using the form NFC and therefore need to convert
      # it to allow correct regex / comparisons to be performed.
      #
      file_path = helpers.unicodize(file_path)
      full_path = os.path.join(path,file_path)

      if os.path.isfile(full_path):
        path_files[file_path.lower()] = full_path

      # Only count real and distinct (not stacked) video files.
      (root, ext) = os.path.splitext(file_path)
      should_count = True
  
      # Check for valid video file extension.
      if ext.lower()[1:] not in config.VIDEO_EXTS:
        should_count = False

      # Don't count sample files if they're smaller than 300MB.
      if should_count:
        for rx in ignore_samples:
          if re.search(rx, full_path, re.IGNORECASE) and os.path.getsize(full_path) < 300 * 1024 * 1024:
            Log('%s looks like a sample, won\'t contribute to total media file count.' % file_path)
            should_count = False

      # Don't count trailer files.
      if should_count:
        for rx in ignore_trailers:
          if re.search(rx, full_path, re.IGNORECASE):
            Log('%s looks like a trailer, won\'t contribute to total media file count.' % file_path)
            should_count = False

      # Don't count dot files.
      if should_count:
        if root.lower().startswith('.'):
          Log('%s won\'t contribute to total media file count.' % file_path)
          should_count = False

      # Don't count things that follow the "-extra" naming convention.
      if should_count and find_extras:
        for key in extra_type_map.keys():
          if root.endswith('-' + key):
            Log('%s looks like a %s extra, won\'t contribute to total media file count.' % (file_path, key))
            should_count = False

      # Don't count multi-part files (stack everything up to and including the year).
      if should_count:
        year = re.search(r'([\(\[\.\-])([1-2][0-9]{3})([\.\-\)\]_,+])', file_path)
        if year:
          multi_part = file_path[0:year.end()]
          if multi_part in multi_parts:
            should_count = False
            Log('%s looks like part of a multi-version set, won\'t contribute to total media file count.' % file_path)
          else:
            multi_parts.append(multi_part)

      # Don't count stacked parts.
      if should_count:
        if full_path in [p.file for p in parts[1:]]:
          should_count = False
          Log('%s looks like a stacked part, won\'t contribute to total media file count.' % file_path)

      # Don't count things that follow specific trailer naming conventions.
      if should_count:
        if root == 'trailer' or root.startswith('movie-trailer'):
          Log('%s looks like a trailer, won\'t contribute to total media file count.' % (file_path))
          should_count = False

      if should_count:
        total_media_files += 1

    if find_extras and type == 'movie':
      extras = []
      re_strip = Regex('[\W ]+')
      
      if total_media_files != 1:
        Log('Found %d media files in this directory, skipping local extras search: %s' % (total_media_files, path))
      else:

        # Look for extras in named directories.
        Log('Looking for local extras in path: ' + path)
        for root, dirs, files in os.walk(path):
          for d in dirs:
            for key in extra_type_map.keys():
              if re_strip.sub('', d.lower()).startswith(key):
                for f in os.listdir(os.path.join(root, d)):
                  (fn, ext) = os.path.splitext(f)
                  if not fn.startswith('.') and ext[1:] in config.VIDEO_EXTS:

                    # On Windows, os.walk() likes to prepend the "extended-length path prefix" to root.
                    # This causes issues later on when this path is converted to the file:// URL for
                    # serialization and later consumption by PMS, so clean it up here.
                    #
                    root = re.sub(r'^\\\\\?\\', '', root)
                    
                    Log('Found %s extra: %s' % (key, f))
                    extras.append({'type' : key, 'title' : helpers.unicodize(fn), 'file' : os.path.join(root, d, f)})
                continue

        # Look for filenames following the "-extra" convention and a couple of other special cases.
        for f in os.listdir(path):

          (fn, ext) = os.path.splitext(f)

          # Files named exactly 'trailer' or starting with 'movie-trailer'.
          if (fn == 'trailer' or fn.startswith('movie-trailer')) and not fn.startswith('.') and ext[1:] in config.VIDEO_EXTS:
            Log('Found trailer extra, renaming with title: ' + media_title)
            extras.append({'type' : key, 'title' : media_title, 'file' : os.path.join(path, f)})

          # Files following the "-extra" convention.
          else:
            for key in extra_type_map.keys():
              if not fn.startswith('.') and fn.endswith('-' + key) and ext[1:] in config.VIDEO_EXTS:
                Log('Found %s extra: %s' % (key, f))
                title = ' '.join(fn.split('-')[:-1])
                extras.append({'type' : key, 'title' : helpers.unicodize(title), 'file' : os.path.join(path, f)})
    
        # Make sure extras are sorted alphabetically and by type.
        type_order = ['trailer', 'behindthescenes', 'interview', 'deleted', 'scene', 'sample', 'featurette', 'short', 'other']
        extras.sort(key=lambda e: e['title'])
        extras.sort(key=lambda e: type_order.index(e['type']))

        for extra in extras:
          metadata.extras.add(extra_type_map[extra['type']](title=extra['title'], file=extra['file']))

        Log('Added %d extras' % len(metadata.extras))

  Log('Looking for %s media (%s) in %d paths (root file: %s) with %d media files.', type, media_title, len(paths), root_file, total_media_files)
  Log('Paths: %s', ", ".join([ helpers.unicodize(p) for p in paths ]))

  # Figure out what regexs to use.
  search_tuples = []
  if type == 'season':
    search_tuples += [['season-?0?%s[-a-z]?(-poster)?' % metadata.index, metadata.posters, config.IMAGE_EXTS, False]]
    search_tuples += [['season-?0?%s-banner[-a-z]?' % metadata.index, metadata.banners, config.IMAGE_EXTS, False]]
    search_tuples += [['season(-|0|\s)?%s-(fanart|art|background|backdrop)[-a-z]?' % metadata.index, metadata.art, config.IMAGE_EXTS, False]]
    if int(metadata.index) == 0: # Season zero, also look for Frodo-compliant 'specials' artwork.
      search_tuples += [['season-specials-poster', metadata.posters, config.IMAGE_EXTS, False]]
      search_tuples += [['season-specials-banner', metadata.banners, config.IMAGE_EXTS, False]]
      search_tuples += [['season-specials-(fanart|art|background|backdrop)[-a-z]?', metadata.art, config.IMAGE_EXTS, False]]
  elif type == 'show':
    search_tuples += [['(show|poster|folder)-?[0-9]?', metadata.posters, config.IMAGE_EXTS, False]]
    search_tuples += [['banner-?[0-9]?', metadata.banners, config.IMAGE_EXTS, False]]
    search_tuples += [['(fanart|art|background|backdrop)-?[0-9]?', metadata.art, config.IMAGE_EXTS, False]]
    search_tuples += [['theme-?[0-9]?', metadata.themes, config.AUDIO_EXTS, False]]
  elif type == 'episode':
    search_tuples += [[re.escape(root_file) + '(-|-thumb)?[0-9]?', metadata.thumbs, config.IMAGE_EXTS, False]]
  elif type == 'movie':
    search_tuples += [['(poster|default|cover|movie|folder|' + re.escape(root_file) + ')-?[0-9]?', metadata.posters, config.IMAGE_EXTS, True]]
    search_tuples += [['(fanart|art|background|backdrop|' + re.escape(root_file) + '-fanart' + ')-?[0-9]?', metadata.art, config.IMAGE_EXTS, True]]

  for (pattern, media_list, extensions, limited) in search_tuples:
    valid_keys = []
    
    sort_index = 1
    file_path_keys = sorted(path_files.keys(), key = lambda x: os.path.splitext(x)[0])
    for file_path in file_path_keys:
      for ext in extensions:
        if re.match('%s.%s' % (pattern, ext), file_path, re.IGNORECASE):

          # Use a pattern if it's unlimited, or if there's only one media file.
          if (limited and total_media_files == 1) or (not limited) or (file_path.find(root_file.lower()) == 0):

            # Read data and hash it.
            data = Core.storage.load(path_files[file_path])
            media_hash = hashlib.md5(data).hexdigest()
      
            # See if we need to add it.
            valid_keys.append(media_hash)
            if media_hash not in media_list:
              media_list[media_hash] = Proxy.Media(data, sort_order = sort_index)
              sort_index = sort_index + 1
              Log('  Local asset added: %s (%s)', path_files[file_path], media_hash)
          else:
            Log('Skipping file %s because there are %d media files.', file_path, total_media_files)
              
    Log('Found %d valid things for pattern %s (ext: %s)', len(valid_keys), pattern, str(extensions))
    media_list.validate_keys(valid_keys)

def getRootFile(filename):
  path = os.path.dirname(filename)
  if 'video_ts' == helpers.splitPath(path.lower())[-1]:
    path = '/'.join(helpers.splitPath(path)[:-1])
  basename = os.path.basename(filename)
  (root_file, ext) = os.path.splitext(basename)
  return root_file

#####################################################################################################################

def findSubtitles(part):

  RE_METAFILES = re.compile('^[\.~]')

  lang_sub_map = {}
  part_filename = helpers.unicodize(part.file)
  part_basename = os.path.splitext(os.path.basename(part_filename))[0]
  paths = [ os.path.dirname(part_filename) ]

  # Check for a global subtitle location
  global_subtitle_folder = os.path.join(Core.app_support_path, 'Subtitles')
  if os.path.exists(global_subtitle_folder):
    paths.append(global_subtitle_folder)

  # We start by building a dictionary of files to their absolute paths. We also need to know
  # the number of media files that are actually present, in case the found local media asset 
  # is limited to a single instance per media file.
  #
  file_paths = {}
  total_media_files = 0
  for path in paths:
    path = helpers.unicodize(path)
    for file_path_listing in os.listdir(path):

      # When using os.listdir with a unicode path, it will always return a string using the
      # NFD form. However, we internally are using the form NFC and therefore need to convert
      # it to allow correct regex / comparisons to be performed.
      #
      file_path_listing = helpers.unicodize(file_path_listing)
      if os.path.isfile(os.path.join(path, file_path_listing)) and not RE_METAFILES.search(file_path_listing):
        file_paths[file_path_listing.lower()] = os.path.join(path, file_path_listing)

      # If we've found an actual media file, we should record it.
      (root, ext) = os.path.splitext(file_path_listing)
      if ext.lower()[1:] in config.VIDEO_EXTS:
        total_media_files += 1

  Log('Looking for subtitle media in %d paths with %d media files.', len(paths), total_media_files)
  Log('Paths: %s', ", ".join([ helpers.unicodize(p) for p in paths ]))

  for file_path in file_paths.values():

    local_basename = helpers.unicodize(os.path.splitext(os.path.basename(file_path))[0]) # no language, no flag
    local_basename2 = local_basename.rsplit('.', 1)[0] # includes language, no flag
    local_basename3 = local_basename2.rsplit('.', 1)[0] # includes language and flag
    filename_matches_part = local_basename == part_basename or local_basename2 == part_basename or local_basename3 == part_basename

    # If the file is located within the global subtitle folder and it's name doesn't match exactly
    # then we should simply ignore it.
    #
    if file_path.count(global_subtitle_folder) and not filename_matches_part:
      continue

    # If we have more than one media file within the folder and located filename doesn't match 
    # exactly then we should simply ignore it.
    #
    if total_media_files > 1 and not filename_matches_part:
      continue

    subtitle_helper = subtitlehelpers.SubtitleHelpers(file_path)
    if subtitle_helper != None:
      local_lang_map = subtitle_helper.process_subtitles(part)
      for new_language, subtitles in local_lang_map.items():

        # Add the possible new language along with the located subtitles so that we can validate them
        # at the end...
        #
        if not lang_sub_map.has_key(new_language):
          lang_sub_map[new_language] = []
        lang_sub_map[new_language] = lang_sub_map[new_language] + subtitles

  # Now whack subtitles that don't exist anymore.
  for language in lang_sub_map.keys():
    part.subtitles[language].validate_keys(lang_sub_map[language])
    
  # Now whack the languages that don't exist anymore.
  for language in list(set(part.subtitles.keys()) - set(lang_sub_map.keys())):
    part.subtitles[language].validate_keys({})
