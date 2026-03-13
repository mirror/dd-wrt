#
#  Plex Plug-in Framework
#  Copyright (C) 2012 Plex, Inc. All Rights Reserved.
#

from systemservice import SystemService


class ProxyService(SystemService):
  def __init__(self, system):
    SystemService.__init__(self, system)
    Log.Debug("Starting the proxy service")

    Route.Connect('/system/proxy', self.do_proxy, method=['GET', 'POST', 'PUT', 'DELETE'])

  def do_proxy(self):
    # Copy the request headers.
    orig_headers = dict(Request.Headers)
    headers = {}

    # Grab the real URL from the header dict.
    url = orig_headers['X-Plex-Url']

    # The proxied request may have needed some X-Plex headers, but the
    # request to the proxy may have needed some also. So delete all the
    # X-Plex- headers, and move X-Plex-Proxy- headers to X-Plex-.
    # Also remove any Cookie headers.
    prefix = "X-Plex-Proxy-"
    for header in orig_headers.keys():
      if header.startswith(prefix):
        headers["X-Plex-" + header[len(prefix):]] = orig_headers[header]
      elif not header.startswith("X-Plex-") and header != "Cookie":
        headers[header] = orig_headers[header]

    # Whack unwanted request headers.
    if 'Host' in headers:
      del headers['Host']

    # Make sure we only accept encoding we can handle. (Fix for #6713 w/ help of @sander1)
    headers['Accept-Encoding'] = "gzip, deflate"

    # Make the HTTP request.
    req = HTTP.Request(
      url = url,
      method = Request.Method,
      headers = headers,
      data = Request.Body if Request.Body and len(Request.Body) > 0 else None,
      timeout = 30,
      immediate = True
    )

    # Whack unwanted response headers. In particular, the original response
    # may have been gzipped, but this response won't be.
    headers = {}
    for name in req.headers:
      headers[name.title()] = req.headers[name]
    for name in ['Content-Encoding', 'Content-Length', 'Transfer-Encoding', 'Connection', 'X-Plex-Content-Compressed-Length', 'X-Plex-Content-Original-Length', 'X-Plex-Protocol']:
      if name in headers:
        del headers[name]

    # Case matters here. If the header isn't "Content-type" then PMS will
    # add an additional header.
    if 'Content-Type' in headers:
      headers['Content-type'] = headers['Content-Type']
      del headers['Content-Type']

    if not 'Access-Control-Allow-Origin' in headers:
      headers['Access-Control-Allow-Origin'] = '*'

    headers['Cache-Control'] = 'no-cache'

    # Update the response headers.
    for name in headers:
      Response.Headers[name] = headers[name]

    # Return the response body.
    return req.content
