import unicodedata, sys

# Unicode control characters can appear in ID3v2 tags but are not legal in XML.
RE_UNICODE_CONTROL =  u'([\u0000-\u0008\u000b-\u000c\u000e-\u001f\ufffe-\uffff])' + \
                      u'|' + \
                      u'([%s-%s][^%s-%s])|([^%s-%s][%s-%s])|([%s-%s]$)|(^[%s-%s])' % \
                      (
                        unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                        unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                        unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff)
                      )

# A platform independent way to split paths which might come in with different separators.
def splitPath(str):
  if str.find('\\') != -1:
    return str.split('\\')
  else: 
    return str.split('/')


def unicodize(s):

  encoding_options = ['utf-8', sys.getdefaultencoding(), sys.getfilesystemencoding(), 'ISO-8859-1']
  normalized = False
  for encoding in encoding_options:
    try:
      s = unicodedata.normalize('NFC', unicode(s.decode(encoding)))
      normalized = True
      break
    except: pass

  if not normalized:
    try: s = unicodedata.normalize('NFC', s)
    except Exception, e: Log(type(e).__name__ + ' exception precomposing: ' + str(e))

  try:
    s = re.sub(RE_UNICODE_CONTROL, '', s)
  except:
    Log('Couldn\'t strip control characters: ' + s)

  return s


def cleanFilename(filename):
  #this will remove any whitespace and punctuation chars and replace them with spaces, strip and return as lowercase
  return string.translate(filename.encode('utf-8'), string.maketrans(string.punctuation + string.whitespace, ' ' * len (string.punctuation + string.whitespace))).strip().lower()