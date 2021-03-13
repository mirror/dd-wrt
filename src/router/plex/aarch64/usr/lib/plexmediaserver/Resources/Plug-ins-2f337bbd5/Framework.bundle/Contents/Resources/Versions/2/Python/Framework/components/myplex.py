#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import urllib
from networking import GLOBAL_DEFAULT_TIMEOUT

from base import BaseComponent

MYPLEX_SERVER = "https://my.plexapp.com:443"
MYPLEX_DEV_SERVER = "https://mydev.plexapp.com:443"


class MyPlex(BaseComponent):

  @property
  def _server_url(self):
    return MYPLEX_DEV_SERVER if Framework.constants.flags.use_myplex_dev_server in self._core.flags else MYPLEX_SERVER

  def request(self, path, values={}, headers={}, timeout=GLOBAL_DEFAULT_TIMEOUT, context=None, method='GET', **kwargs):
    if context == None:
      context = self._core.sandbox.context

    kwargs['auth_token'] = context.token
    query_string = '&'.join(['%s=%s' % (key, urllib.quote(value)) for key, value in kwargs.items()])
    
    return self._core.networking.http_request(
      url = self._server_url + path + '?' + query_string,
      values = values,
      headers = headers,
      timeout = timeout,
      immediate = True,
      opener = context.opener,
    )
