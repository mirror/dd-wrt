#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BaseKit

class LocaleKit(BaseKit):
  
  _included_policies = [
    Framework.policies.CodePolicy,
  ]


  def _init(self):
    self._publish(self.LocalString, name='L')
    self._publish(self.LocalStringWithFormat, name='F')
    
    self.Language = self._core.localization.language
    self.CountryCodes = self._core.localization.countrycodes
    
  @property
  def DefaultLocale(self):
    """
      Returns the default locale currently in use by the plug-in, e.g. ``en-us``.
    """
    return self._core.localization.default_locale
    
  @DefaultLocale.setter
  def DefaultLocale(self, value):
    self._core.localization.default_locale = value
    
  @property
  def Geolocation(self):
    """
      Returns the user's country, obtained via IP-based geolocation, e.g. ``US``.
    """
    return self._core.networking.http_request("http://geo.plexapp.com/geolocate.php", cacheTime=86400).content
  
  @property
  def CurrentLocale(self):
    """
      Returns the locale of the user currently making a request to the plug-in, or *None* if no locale was provided.
    """
    return self._context.locale
  
  def LocalString(self, key):
    """
      Retrieves the localized version of a string with the given key. Strings from the user's
      current locale are given highest priority, followed by strings from the default locale.
      
      See `String files` for more information on proividing localized versions of strings.
    """
    return self._core.localization.local_string(key, self.CurrentLocale)
    
  def LocalStringWithFormat(self, key, *args):
    """
      Retrieves the localized version of a string with the given key, and formats it using the
      given arguments.
    """
    return self._core.localization.local_string_with_format(key, self.CurrentLocale, *args)
