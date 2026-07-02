import re, unicodedata
import config
import helpers

class SubtitleHelper(object):
  def __init__(self, filename):
    self.filename = filename

def SubtitleHelpers(filename):
  filename = helpers.unicodize(filename)
  for cls in [ VobSubSubtitleHelper, DefaultSubtitleHelper ]:
    if cls.is_helper_for(filename):
      return cls(filename)
  return None

#####################################################################################################################

class VobSubSubtitleHelper(SubtitleHelper):
  @classmethod
  def is_helper_for(cls, filename):
    (file_name, file_ext) = os.path.splitext(filename)

    # We only support idx (and maybe sub)
    if not file_ext.lower() in ['.idx', '.sub']:
      return False

    # If we've been given a sub, we only support it if there exists a matching idx file
    return os.path.exists(file_name + '.idx')

  def process_subtitles(self, part):

    lang_sub_map = {}

    # We don't directly process the sub file, only the idx. Therefore if we are passed on of these files, we simply
    # ignore it.
    (file_name, file_ext) = os.path.splitext(self.filename)
    if file_ext == '.sub':
      return lang_sub_map
    
    # If we have an idx file, we need to confirm there is an identically names sub file before we can proceed.
    sub_filename = file_name + ".sub"
    if os.path.exists(sub_filename) == False:
      return lang_sub_map

    Log('Attempting to parse VobSub file: ' + self.filename)
    idx = Core.storage.load(os.path.join(self.filename))
    if idx.count('VobSub index file') == 0:
      Log('The idx file does not appear to be a VobSub, skipping...')
      return lang_sub_map

    forced = ''
    default = ''
    split_tag = file_name.rsplit('.', 1)
    if len(split_tag) > 1 and split_tag[1].lower() in ['forced', 'default'] :
      if 'forced' == split_tag[1].lower():
        forced = '1'
      if 'default' == split_tag[1].lower():
        default = '1'

    languages = {}
    language_index = 0
    basename = os.path.basename(self.filename)
    for language in re.findall('\nid: ([A-Za-z]{2})', idx):

      if not languages.has_key(language):
        languages[language] = []

      Log('Found .idx subtitle file: ' + self.filename + ' language: ' + language + ' stream index: ' + str(language_index) + ' default: ' + default + ' forced: ' + forced)
      languages[language].append(Proxy.LocalFile(self.filename, index = str(language_index), format = "vobsub", default = default, forced = forced))
      language_index += 1

      if not lang_sub_map.has_key(language):
        lang_sub_map[language] = []
      lang_sub_map[language].append(basename)

    for language, subs in languages.items():
      part.subtitles[language][basename] = subs

    return lang_sub_map

#####################################################################################################################

class DefaultSubtitleHelper(SubtitleHelper):
  @classmethod
  def is_helper_for(cls, filename):
    (file, file_extension) = os.path.splitext(filename)
    return file_extension.lower()[1:] in config.SUBTITLE_EXTS

  def process_subtitles(self, part):

    lang_sub_map = {}

    basename = os.path.basename(self.filename)
    (file, ext) = os.path.splitext(self.filename)

    # Remove the initial '.' from the extension
    ext = ext[1:]

    forced = ''
    default = ''
    split_tag = file.rsplit('.', 1)
    if len(split_tag) > 1 and split_tag[1].lower() in ['forced', 'normal', 'default'] :
      file = split_tag[0]
      # don't do anything with 'normal', we don't need it
      if 'forced' == split_tag[1].lower():
        forced = '1'
      if 'default' == split_tag[1].lower():
        default = '1'

    # Attempt to extract the language from the filename (e.g. Avatar (2009).eng)
    language = ""
    language_match = re.match(".+\.([^-.]+)(?:-[A-Za-z]{2,4})?$", file)
    if language_match and len(language_match.groups()) == 1:
      language = language_match.groups()[0]
    language = Locale.Language.Match(language)

    codec = None
    format = None
    if ext in ['txt', 'sub']:
      try:

        file_contents = Core.storage.load(self.filename)
        lines = [ line.strip() for line in file_contents.splitlines(True) ]
        if re.match('^\{[0-9]+\}\{[0-9]*\}', lines[1]):
          format = 'microdvd'
        elif re.match('^[0-9]{1,2}:[0-9]{2}:[0-9]{2}[:=,]', lines[1]):
          format = 'txt'
        elif '[SUBTITLE]' in lines[1]:
          format = 'subviewer'
        else:
          Log("The subtitle file does not have a known format, skipping... : " + self.filename)
          return lang_sub_map
      except:
        Log("An error occurred while attempting to parse the subtitle file, skipping... : " + self.filename)
        return lang_sub_map

    if codec is None and ext in ['ass', 'ssa', 'smi', 'srt', 'psb']:
      codec = ext.replace('ass', 'ssa')

    if format is None:
      format = codec

    Log('Found subtitle file: ' + self.filename + ' language: ' + language + ' codec: ' + str(codec) + ' format: ' + str(format) + ' default: ' + default + ' forced: ' + forced)
    part.subtitles[language][basename] = Proxy.LocalFile(self.filename, codec = codec, format = format, default = default, forced = forced)

    lang_sub_map[language] = [ basename ]
    return lang_sub_map