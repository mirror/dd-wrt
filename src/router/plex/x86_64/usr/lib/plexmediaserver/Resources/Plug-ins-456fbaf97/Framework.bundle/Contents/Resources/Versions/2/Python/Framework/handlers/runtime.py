import Framework
import urllib
import cookielib
import sys

from base import BaseHandler, InternalRequestHandler, PrefixRequestHandler

NO_QUERY = object()

class RuntimeRequestHandler(InternalRequestHandler):

  @BaseHandler.before_all
  def parse_prefs(self, context):
    # Parse the preferences request header.
    if Framework.constants.header.preferences in context.request.headers:
      pref_values = {}
      request_prefs = context.request.headers[Framework.constants.header.preferences].split('&')
      for request_pref in request_prefs:
        if '=' in request_pref:
          name, value = request_pref.split('=')
          pref_values[name] = urllib.unquote(value)

      context.pref_values[context._sandbox.identifier] = pref_values
      self._core.log.debug("Parsed preferences: %s" % pref_values.keys())


  @BaseHandler.before_all
  def parse_state_data(self, context):
    # Build a context-specific opener if we should be proxying user data.
    if context.proxy_user_data:
      #self._core.log.debug("Enabling user data proxying")
      if context.cookie_jar == None:
        context.cookie_jar = cookielib.MozillaCookieJar()

      context.session_data = None

      # If the cookie header is present, extract the key & value and try to match to the current identifier
      if 'Cookie' in context.request.headers and len(context.request.headers['Cookie']) > 0:
        for parts in [pair.strip().split('=') for pair in context.request.headers['Cookie'].split(',')]:
          if len(parts) < 2:
            continue

          key = parts[0].strip()
          value = parts[1].split(';')[0].strip()
          state_data = None

          # If found, unpack the encoded cookies and state data and populate the cookie jar and other dicts
          if key == self._core.identifier:
            self._core.log.debug("Received packed state data (%d bytes)", len(value))
            
            unpacked_value = Framework.utils.unpack(value)
            state_data = unpacked_value if isinstance(unpacked_value, dict) else dict(cookies = unpacked_value)
            
            for cookie in state_data.get('cookies',  []):
              if not isinstance(cookie, dict):
                context.cookie_jar.set_cookie(cookie)
            
            context.session_data = state_data.get('session')
            break
      
      context.opener = self._core.networking.build_opener(cookie_jar=context.cookie_jar)

      # If we have no session data, create it
      if context.session_data == None:
        context.create_session_data()

    else:
      context.opener = None


  @BaseHandler.after_all
  def return_cache_time(self, context, response_headers):
    if context.cache_time != None:
      response_headers.setdefault('Cache-Control', 'no-cache' if context.cache_time == 0 else 'max-age=%d' % context.cache_time)


  @BaseHandler.after_all
  def return_state_data(self, context, response_headers):
    # Create a packed cookie containing all state data.
    if context.proxy_user_data:
      state_data = dict(session = context.session_data)

      if context.cookie_jar != None:
        state_data['cookies'] = [cookie for cookie in context.cookie_jar]
      
      packed_data = self._core.identifier + '=' + Framework.utils.pack(state_data)
      response_headers['Set-Cookie'] = packed_data
      self._core.log.debug("Sending packed state data (%d bytes)", len(packed_data))
      

  @BaseHandler.route('/root')
  def root(self):
    # Get a list of all prefixes.
    handlers = [handler for handler in self._core.runtime._handlers if isinstance(handler, Framework.handlers.PrefixRequestHandler)]
    prefixes = [handler.prefix for handler in handlers]
    
    # Catch-all handlers take precedence over all others.
    for prefix in prefixes:
      if prefix.startswith('/plugins'):
        return Framework.objects.Redirect(self._core, prefix)
    
    # Try to find a video handler first
    for prefix in prefixes:
      if prefix.startswith('/video'):
        return Framework.objects.Redirect(self._core, prefix)
        
    # No video handler found? Look for a music one
    for prefix in prefixes:
      if prefix.startswith('/music'):
        return Framework.objects.Redirect(self._core, prefix)

    # Otherwise, return the first one in the list
    return Framework.objects.Redirect(self._core, handlers[0].prefix)


  #TODO: Dedupe
  @BaseHandler.route('/plist')
  def plist(self):
    return self._core.storage.load(self._core.plist_path)


  @BaseHandler.route('/function/{function_name}')
  @BaseHandler.route('/function/{function_name}/{query}', query = NO_QUERY)
  def call_function(self, function_name, **kwargs):
      self._core.log.debug("Calling function '%s'", function_name)
      
      # Strip the extension (if included)
      pos = function_name.rfind('.')
      if pos > -1:
        function_name = function_name[:pos]
      
      if kwargs.get('query') == NO_QUERY:
        del kwargs['query']
        
      result = self._core.sandbox.call_named_function(function_name, kwargs=kwargs)
      return result   


  @BaseHandler.route('/prefs')
  @BaseHandler.route('/prefs/list', show_values=False)
  def get_prefs(self, show_values=True):
    mc = Framework.objects.MediaContainer(self._core)
    locale = self._core.sandbox.context.locale
    
    manager = self._core.sandbox.preferences.get()
    manager._prefs # Access the prefs to ensure they're loaded

    for name in manager._pref_names:
      pref = manager._prefs[name]
      value = manager[name]
      
      if not pref.hidden:
        info_dict = pref.info_dict(self._core, locale, value)
      
        if not show_values and 'value' in info_dict:
          del info_dict['value']
      
        obj = Framework.objects.XMLObject(self._core, tagName='Setting', id=name, **info_dict)
        mc.Append(obj)

    return mc


  @BaseHandler.route('/prefs/set')
  def set_prefs(self, **kwargs):
    self._core.sandbox.preferences.get().update_user_values(**kwargs)
    result = self._core.sandbox.call_named_function('ValidatePrefs')
    return result if result != None else ''

  
  @BaseHandler.route('/resourceHashes')
  def get_resource_hashes(self):
    json = self._core.runtime.get_resource_hashes()
    if json is not None:
      return self._core.data.json.to_string(json)
    