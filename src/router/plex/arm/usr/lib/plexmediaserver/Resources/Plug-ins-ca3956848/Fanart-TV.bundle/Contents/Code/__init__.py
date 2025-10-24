API_KEY = '72519ab36caf49c09f69a028fb7f741d'
MOVIE_ART_URL = 'https://webservice.fanart.tv/v3/movies/%s' # IMDb or TheMovieDB id
TV_ART_URL = 'https://webservice.fanart.tv/v3/tv/%s' # TheTVDB id
ARTIST_ART_URL = 'https://webservice.fanart.tv/v3/music/%s' # MusicBrainz artist id

PREVIEW_PATH = '/preview/'
FANART_PATH = '/fanart/'

MB_ARTIST = 'https://musicbrainz.plex.tv/ws/2/artist/%s'
MB_RELEASE = 'https://musicbrainz.plex.tv/ws/2/release/%s?inc=release-groups'
MB_NS = {'a': 'http://musicbrainz.org/ns/mmd-2.0#'}

RE_KEY_CHECK = Regex('[a-f0-9]{32}')

####################################################################################################
def Start():

	HTTP.CacheTime = CACHE_1WEEK

####################################################################################################
@expose
def AlbumPosters(artist_mbid, album_mbid, lang):
	try:
		release_group = XML.ElementFromURL(MB_RELEASE % (album_mbid)).xpath('//a:release-group/@id', namespaces=MB_NS)[0]
	except:
		release_group = None

	posters = []

	try:
		json_obj = GetJSON(ARTIST_ART_URL % (artist_mbid))
	except:
		json_obj = None

	if json_obj and 'albums' in json_obj:
		for mbid in json_obj['albums'].keys():
			if mbid == release_group:
				for img in SortMedia(json_obj['albums'][mbid]['albumcover'], lang=lang):
					poster_url = img['url']
					posters.append(poster_url)
				break

	return posters

####################################################################################################
def GetJSON(url):

	http_headers = {
		'api-key': API_KEY
	}

	if Prefs['personal_api_key'] and RE_KEY_CHECK.search(Prefs['personal_api_key']):
		http_headers['client-key'] = Prefs['personal_api_key']

	return JSON.ObjectFromURL(url, headers=http_headers, sleep=1.0)

####################################################################################################
def SortMedia(json, lang):

	images = list()

	for img in json:

		if 'lang' not in img:
			score = 0
		elif img['lang'] == lang:
			score = 2
		elif img['lang'] == '00':
			score = 1
		else:
			score = 0

		images.append({
			'score': score,
			'url': img['url'],
			'likes': int(img['likes']),
			'season': img['season'] if 'season' in img else None
		})

	images = sorted(images, key=lambda k: (k['score'], k['likes']), reverse=True)

	return images

####################################################################################################
class FanartTVAgent(Agent.Movies):

	name = 'Fanart.tv'
	languages = [Locale.Language.NoLanguage]
	primary_provider = False
	contributes_to = [
		'com.plexapp.agents.imdb',
		'com.plexapp.agents.themoviedb'
	]

	def search(self, results, media, lang):

		if media.primary_metadata:
			results.Append(MetadataSearchResult(
				id = media.primary_metadata.id,
				score = 100
			))

	def update(self, metadata, media, lang):

		# Backdrops
		valid_names = list()

		try:
			json_obj = GetJSON(MOVIE_ART_URL % (metadata.id))
		except:
			json_obj = None

		if json_obj and 'moviebackground' in json_obj:

			i = 0

			for img in SortMedia(json_obj['moviebackground'], lang=lang):
				art_url = img['url']
				valid_names.append(art_url)

				if art_url not in metadata.art:
					try:
						i += 1
						metadata.art[art_url] = Proxy.Preview(HTTP.Request(art_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
					except:
						pass

		metadata.art.validate_keys(valid_names)

		# Posters
		valid_names = list()

		try:
			json_obj = GetJSON(MOVIE_ART_URL % (metadata.id))
		except:
			json_obj = None

		if json_obj and 'movieposter' in json_obj:

			i = 0

			for img in SortMedia(json_obj['movieposter'], lang=lang):
				poster_url = img['url']
				valid_names.append(poster_url)

				if poster_url not in metadata.posters:
					try:
						i += 1
						metadata.posters[poster_url] = Proxy.Preview(HTTP.Request(poster_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
					except:
						pass

		metadata.posters.validate_keys(valid_names)

####################################################################################################
class FanartTVAgent(Agent.TV_Shows):

	name = 'Fanart.tv'
	languages = [Locale.Language.NoLanguage]
	primary_provider = False
	contributes_to = [
		'com.plexapp.agents.thetvdb',
		'com.plexapp.agents.themoviedb'
	]

	def search(self, results, media, lang):

		if media.primary_agent == 'com.plexapp.agents.thetvdb':
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

	def update(self, metadata, media, lang):

		# Backdrops
		valid_names = list()

		try:
			json_obj = GetJSON(TV_ART_URL % (metadata.id))
		except:
			json_obj = None

		if json_obj and 'showbackground' in json_obj:

			i = 0

			for img in SortMedia(json_obj['showbackground'], lang=lang):
				art_url = img['url']
				valid_names.append(art_url)

				if art_url not in metadata.art:
					try:
						i += 1
						metadata.art[art_url] = Proxy.Preview(HTTP.Request(art_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
					except:
						pass

		metadata.art.validate_keys(valid_names)

		# Posters
		valid_names = list()

		try:
			json_obj = GetJSON(TV_ART_URL % (metadata.id))
		except:
			json_obj = None

		if json_obj and 'tvposter' in json_obj:

			i = 0

			for img in SortMedia(json_obj['tvposter'], lang=lang):
				poster_url = img['url']
				valid_names.append(poster_url)

				if poster_url not in metadata.posters:
					try:
						i += 1
						metadata.posters[poster_url] = Proxy.Preview(HTTP.Request(poster_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
					except:
						pass

		metadata.posters.validate_keys(valid_names)

		# Banners
		valid_names = list()

		try:
			json_obj = GetJSON(TV_ART_URL % (metadata.id))
		except:
			json_obj = None

		if json_obj and 'tvbanner' in json_obj:

			i = 0

			for img in SortMedia(json_obj['tvbanner'], lang=lang):
				banner_url = img['url']
				valid_names.append(banner_url)

				if banner_url not in metadata.banners:
					try:
						i += 1
						metadata.banners[banner_url] = Proxy.Preview(HTTP.Request(banner_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
					except:
						pass

		metadata.banners.validate_keys(valid_names)

		# Season posters
		@parallelize
		def UpdateSeasons():

			# Loop over seasons
			for s in media.seasons:
				season = metadata.seasons[s]

				# Set season metadata
				@task
				def UpdateSeason(season=season, s=s):

					try:
						json_obj = GetJSON(TV_ART_URL % (metadata.id))
					except:
						json_obj = None

					valid_names = list()

					if json_obj and 'seasonposter' in json_obj:

						i = 0

						for img in SortMedia(json_obj['seasonposter'], lang=lang):

							if 'season' in img and img['season'] == s:
								poster_url = img['url']
								valid_names.append(poster_url)

								if poster_url not in season.posters:
									try:
										i += 1
										season.posters[poster_url] = Proxy.Preview(HTTP.Request(poster_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
									except:
										pass

					season.posters.validate_keys(valid_names)

####################################################################################################
class FanartTVAgent(Agent.Artist):

	name = 'Fanart.tv'
	languages = [Locale.Language.NoLanguage]
	primary_provider = False
	contributes_to = [
		'com.plexapp.agents.lastfm'
	]

	def search(self, results, media, lang):

		# Get the artist's MusicBrainz id from the Last.fm Agent
		artist_mbid = Core.messaging.call_external_function(
			'com.plexapp.agents.lastfm',
			'MessageKit:GetMusicBrainzId',
			kwargs = dict(
				artist = media.primary_metadata.title
			)
		)

		if artist_mbid:

			# MusicBrainz ids can change over time while Last.fm is still listing an older id.
			# If we do not get any data back: check if we can use a new/different MusicBrainz id.
			try:
				json_obj = GetJSON(ARTIST_ART_URL % (artist_mbid))
			except:
				json_obj = None

			if not json_obj:
				try:
					artist_mbid = XML.ElementFromURL(MB_ARTIST % (artist_mbid)).xpath('//a:artist/@id', namespaces=MB_NS)[0]
				except:
					artist_mbid = None

		if artist_mbid:

			results.Append(MetadataSearchResult(
				id = artist_mbid,
				score = 100
			))

	def update(self, metadata, media, lang):

		# Artist art
		valid_names = list()

		try:
			json_obj = GetJSON(ARTIST_ART_URL % (metadata.id))
		except:
			json_obj = None

		if json_obj and 'artistbackground' in json_obj:

			i = 0

			for img in SortMedia(json_obj['artistbackground'], lang=lang):
				art_url = img['url']
				valid_names.append(art_url)

				if art_url not in metadata.art:
					try:
						i += 1
						metadata.art[art_url] = Proxy.Preview(HTTP.Request(art_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
					except:
						pass

		metadata.art.validate_keys(valid_names)

		# Artist posters
		valid_names = list()

		try:
			json_obj = GetJSON(ARTIST_ART_URL % (metadata.id))
		except:
			json_obj = None

		if json_obj and 'artistthumb' in json_obj:

			i = 0

			for img in SortMedia(json_obj['artistthumb'], lang=lang):
				poster_url = img['url']
				valid_names.append(poster_url)

				if poster_url not in metadata.posters:
					try:
						i += 1
						metadata.posters[poster_url] = Proxy.Preview(HTTP.Request(poster_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
					except:
						pass

		metadata.posters.validate_keys(valid_names)

####################################################################################################
class FanartTVAgent(Agent.Album):

	name = 'Fanart.tv'
	languages = [Locale.Language.NoLanguage]
	primary_provider = False
	contributes_to = [
		'com.plexapp.agents.lastfm'
	]

	def search(self, results, media, lang):

		artist = String.Unquote(media.primary_metadata.id.split('/')[0])
		album = media.primary_metadata.title

		# Get the artist's MusicBrainz id from the Last.fm Agent
		artist_mbid = Core.messaging.call_external_function(
			'com.plexapp.agents.lastfm',
			'MessageKit:GetMusicBrainzId',
			kwargs = dict(
				artist = artist
			)
		)

		if artist_mbid:

			# MusicBrainz ids can change over time while Last.fm is still listing an older id.
			# If we do not get any data back: check if we can use a new/different MusicBrainz id.
			try:
				json_obj = GetJSON(ARTIST_ART_URL % (artist_mbid))
			except:
				json_obj = None

			if not json_obj:
				try:
					artist_mbid = XML.ElementFromURL(MB_ARTIST % (artist_mbid)).xpath('//a:artist/@id', namespaces=MB_NS)[0]
				except:
					artist_mbid = None

		if artist_mbid:

			# Get the album's MusicBrainz id from the Last.fm Agent
			album_mbid = Core.messaging.call_external_function(
				'com.plexapp.agents.lastfm',
				'MessageKit:GetMusicBrainzId',
				kwargs = dict(
					artist = artist,
					album = album
				)
			)

			if album_mbid:

				results.Append(MetadataSearchResult(
					id = '%s/%s' % (artist_mbid, album_mbid),
					score = 100
				))

	def update(self, metadata, media, lang):

		(artist_mbid, album_mbid) = metadata.id.split('/')

		valid_names = AlbumPosters(artist_mbid, album_mbid, lang)

		i = 0
		for poster_url in valid_names:
			if poster_url not in metadata.posters:
				try:
					i += 1
					metadata.posters[poster_url] = Proxy.Preview(HTTP.Request(poster_url.replace(FANART_PATH, PREVIEW_PATH), sleep=0.5).content, sort_order=i)
				except:
					pass

		metadata.posters.validate_keys(valid_names)
