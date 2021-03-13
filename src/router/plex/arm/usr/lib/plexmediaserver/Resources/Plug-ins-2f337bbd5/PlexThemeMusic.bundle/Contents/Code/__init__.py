#Plex Theme Music
THEME_URL = 'https://tvthemes.plexapp.com/%s.mp3'

def Start():
  HTTP.CacheTime = CACHE_1DAY
  
class PlexThemeMusicAgent(Agent.TV_Shows):
  name = 'Plex Theme Music'
  languages = [Locale.Language.NoLanguage]
  primary_provider = False
  contributes_to = [
    'com.plexapp.agents.thetvdb',
    'com.plexapp.agents.thetvdbdvdorder',
    'com.plexapp.agents.themoviedb'
  ]

  def search(self, results, media, lang):

    if media.primary_agent == 'com.plexapp.agents.thetvdb' or media.primary_agent == 'com.plexapp.agents.thetvdbdvdorder':
      results.Append(MetadataSearchResult(
        id = media.primary_metadata.id,
        score = 100
      ))

    elif media.primary_agent == 'com.plexapp.agents.themoviedb':
      # Get the TVDB id from the Movie Database Agent
      tvdb_id = Core.messaging.call_external_function(
        'com.plexapp.agents.themoviedb',
        'MessageKit:GetTvdbId',
        kwargs = dict(
          tmdb_id = media.primary_metadata.id
        )
      )

      if tvdb_id:
        results.Append(MetadataSearchResult(
          id = tvdb_id,
          score = 100
        ))

  def update(self, metadata, media, lang, force=False):
    if force or THEME_URL % metadata.id not in metadata.themes:
      try:
        metadata.themes[THEME_URL % metadata.id] = Proxy.Media(HTTP.Request(THEME_URL % metadata.id))
      except:
        pass