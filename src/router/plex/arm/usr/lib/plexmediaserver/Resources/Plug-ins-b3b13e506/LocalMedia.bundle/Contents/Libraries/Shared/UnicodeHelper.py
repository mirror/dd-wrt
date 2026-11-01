import string

_encodings = ['iso8859-1', 'utf-16', 'utf-16be', 'utf-8']

def fixEncoding(theString, language=None):
  encoding = ord(theString[0])
  if 0 <= encoding < len(_encodings):
    # If we're dealing with a particular language, we might want to try another code page.
    if encoding == 0 and language == 'ko':
      value = theString[1:].decode('cp949').encode('utf-8')
    else:
      value = theString[1:].decode(_encodings[encoding]).encode("utf-8")
  else:
    value = theString

  if value:
    value = value.strip('\0')

  return value
