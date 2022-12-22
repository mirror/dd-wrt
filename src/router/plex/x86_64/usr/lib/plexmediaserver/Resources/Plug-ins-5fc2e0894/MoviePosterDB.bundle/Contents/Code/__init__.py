# CineMaterial
CINE_ROOT = 'https://api.cinematerial.com'
CINE_JSON = '%s/1/request.json?imdb_id=%%s&key=plex&secret=%%s&width=720&thumb_width=100.' % CINE_ROOT
CINE_SECRET = '157de27cd9815301d29ab8dcb2791bdf'

####################################################################################################
def Start():

	HTTP.CacheTime = CACHE_1DAY

####################################################################################################
class CineMaterialAgent(Agent.Movies):

	name = 'CineMaterial'
	languages = [Locale.Language.NoLanguage]
	primary_provider = False
	contributes_to = ['com.plexapp.agents.imdb']

	def search(self, results, media, lang):

		if media.primary_metadata is not None:
			results.Append(MetadataSearchResult(
				id = media.primary_metadata.id,
				score = 100
			))

	def update(self, metadata, media, lang):

		imdb_code = metadata.id
		secret = Cipher.Crypt('%s%s' % (imdb_code, CINE_SECRET), 'rand0mn3ss123')
		json_obj = JSON.ObjectFromURL(CINE_JSON % (imdb_code, secret), sleep=1.0)
		valid_names = list()

		if 'errors' not in json_obj and 'posters' in json_obj:
			i = 0

			# Look through the posters for ones with language matches.
			for poster in json_obj['posters']:
				poster_language = poster['language'].lower().replace('us', 'en').replace('uk', 'en').replace('ca', 'en')
				if lang == poster_language:
					Log('Adding matching language poster for language: %s', poster['language'])
					valid_names.append(self.add_poster(metadata, secret, poster, i))
					i += 1

			# If we didn't find a language match, add the first foreign one.
			if i == 0 and len(json_obj['posters']) > 0:
				Log('Falling back to foreign language poster with language: %s', json_obj['posters'][0]['language'])
				valid_names.append(self.add_poster(metadata, secret, json_obj['posters'][0], i))

		metadata.posters.validate_keys(valid_names)

	def add_poster(self, metadata, secret, poster, index):

		image_url = poster['image_location']
		thumb_url = poster['thumbnail_location']

		Log('Adding new poster: %s' % image_url)
		metadata.posters[image_url] = Proxy.Preview(HTTP.Request(thumb_url), sort_order=index)
		return image_url
