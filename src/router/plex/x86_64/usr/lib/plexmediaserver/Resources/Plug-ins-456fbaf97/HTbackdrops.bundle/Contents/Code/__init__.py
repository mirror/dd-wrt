API_URL = 'https://htbackdrops.plex.tv/api'
API_KEY = '15f8fe4ad7760d77c85e686eefafd26f'
SEARCH_ARTIST  = '%s/%s/searchXML?keywords=%%s&default_operator=and&limit=50&aid=1,5' % (API_URL, API_KEY)
MUSIC_ART      = '%s/%s/searchXML?keywords=%%s&default_operator=and&limit=50&aid=1' % (API_URL, API_KEY)
MUSIC_THUMBS   = '%s/%s/searchXML?keywords=%%s&default_operator=and&limit=50&aid=5' % (API_URL, API_KEY)
THUMB_URL      = '%s/%s/download/%%s/thumbnail' % (API_URL, API_KEY)
FULL_IMAGE_URL = '%s/%s/download/%%s/fullsize' % (API_URL, API_KEY)

def Start():
  HTTP.CacheTime = CACHE_1WEEK
  HTTP.Headers['User-Agent'] = 'Plex Media Server/%s' % Platform.ServerVersion

@expose
def ArtistSearch(artist_name):
  artist_results = []
  artistName = artist_name.lower().replace('.', ' ')
  previousName = ''

  for artist in XML.ElementFromURL(SEARCH_ARTIST % String.URLEncode(artistName), sleep=1.0).xpath('//image/title/text()'):
    curName = artist.lower().replace('.', ' ').replace('’','\'')
    if curName != previousName:
      score = 100 - Util.LevenshteinDistance(artistName, curName)
      artist_results.append({'id' : curName, 'score' : score})
      previousName = curName

  return sorted(artist_results, key=lambda a: a['score'], reverse=True)

class HTBDAgent(Agent.Artist):
  name = 'Home Theater Backdrops'
  languages = Locale.Language.All()
  primary_provider = False
  contributes_to = ['com.plexapp.agents.lastfm']

  def search(self, results, media, lang):
    if media.primary_metadata is not None:
      for artist_result in ArtistSearch(media.primary_metadata.title):
        results.Append(MetadataSearchResult(artist_result['id'], artist_result['score']))

  def update(self, metadata, media, lang):
    # Remove old art
    for key in metadata.art.keys():
      if key.startswith(API_URL) == False or API_KEY not in key:
        del metadata.art[key]

    # Remove old posters
    for key in metadata.posters.keys():
      if key.startswith(API_URL) == False or API_KEY not in key:
        del metadata.posters[key]

    for s in [MUSIC_ART, MUSIC_THUMBS]:
      i = 0

      for id in XML.ElementFromURL(s % String.URLEncode(metadata.id), sleep=1.0).xpath('//image/id/text()'):
        i += 1
        thumb = HTTP.Request(THUMB_URL % (id), cacheTime=CACHE_1MONTH)
        largeImgUrl = FULL_IMAGE_URL % id

        if s == MUSIC_ART:
          if largeImgUrl not in metadata.art:
            metadata.art[largeImgUrl] = Proxy.Preview(thumb, sort_order = i)
        else:
          if largeImgUrl not in metadata.posters:
            metadata.posters[largeImgUrl] = Proxy.Preview(thumb, sort_order = i)
