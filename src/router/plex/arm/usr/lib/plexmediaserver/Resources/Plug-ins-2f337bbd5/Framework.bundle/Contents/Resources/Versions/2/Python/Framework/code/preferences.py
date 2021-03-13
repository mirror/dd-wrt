from __future__ import with_statement

import Framework
import os
import weakref


# Notes about pref storage:

# Prefs are encoded to strings when stored internally. This includes default values, user
# values and session values. Each pref class knows how to encode a value (convert it to
# a string representation) and decode it again (convert back from a string to a standard
# Python type). Encoded values are written to disk and returned to clients for storage
# or modification. Plug-in code will always receive decoded values.

# Encoded text values are equivalent to decoded values, except empty string are
# equivalent to None.

# Encoded bool values are 'true' or 'false'.

# Encoded enum values are the integer index of the decoded value in the list of possible
# values.


class Pref(object):

  def __init__(self, pref_type, label, default_value=None, secure=False, hidden=False):
    self.type = pref_type
    self.label = label
    self.default_value = self.encode_value(default_value)
    self.secure = secure
    self.hidden = hidden
  

  def encode_value(self, value):
    return value
  

  def decode_value(self, encoded_value):
    return encoded_value
  

  def info_dict(self, core, locale, value=None, **kwargs):
    d = dict(
      label = core.localization.localize(self.label, locale),
      type = self.type,
      secure = ('true' if self.secure else 'false'),
      value = self.default_value if value == None else self.encode_value(value),
      default = self.default_value,
    )

    d.update(**kwargs)
    return d


  def __str__(self):
    return "%s (%s)" % (type(self).__name__, self.label)
    


class TextPref(Pref):

  def __init__(self, label, default_value, options=[], secure=False, hidden=False):
    Pref.__init__(self, 'text', label, default_value, secure, hidden) 
    self.options = options

    
  def info_dict(self, core, locale, value=None, **kwargs):
    return Pref.info_dict(self, core, locale, value, option=','.join(self.options), **kwargs)


  def encode_value(self, encoded_value):
    return '' if encoded_value == None else str(encoded_value)


  def decode_value(self, value):
    return None if value == None or len(value) == 0 else str(value)


    
class BooleanPref(Pref):

  def __init__(self, label, default_value, secure=False, hidden=False):
    Pref.__init__(self, 'bool', label, default_value, secure, hidden)


  def encode_value(self, value):
    return 'true' if Framework.utils.is_true(value) else 'false'


  def decode_value(self, encoded_value):
    return Framework.utils.is_true(encoded_value)
  


class EnumPref(Pref):

  def __init__(self, label, default_value, values=[], secure=False, hidden=False):
    self.values = values
    Pref.__init__(self, 'enum', label, default_value, secure, hidden)
    

  def encode_value(self, value):
    return str(self.values.index(value)) if value in self.values else None


  def decode_value(self, encoded_value):
    try:
      int_val = int(encoded_value)
      if int_val < len(self.values):
        return self.values[int_val]
    except:
      pass


  def info_dict(self, core, locale, value=None, **kwargs):
    value_labels = []
    for v in self.values:
      value_labels.append(core.localization.localize(v, locale))
    return Pref.info_dict(self, core, locale, value,
      values = '|'.join(value_labels)
    )
      


class PreferenceSet(object):

  def __init__(self, manager, identifier):
    self._manager = weakref.proxy(manager)
    self._identifier = identifier
    self._pref_names = []
    self._prefs_dict = None
    self._prefs_lock = self._core.runtime.lock()
    self._user_values_dict = {}
    self._user_values_lock = self._core.runtime.lock()

  @property
  def _sandbox(self):
    return self._manager._sandbox

  @property
  def _core(self):
    return self._sandbox._core


  @property
  def _user_file_path(self):
    return self._core.storage.join_path(self._core.plugin_support_path, 'Preferences', self._identifier + '.xml')


  def _load_user_file(self):
    # Return immediately if daemonized
    if self._core.config.daemonized:
      return

    # Check whether the prefs file exists. If not, write defaults and get out.
    file_path = self._user_file_path
    user_values = {}

    if not self._core.storage.file_exists(file_path):
      self._core.log.info("No user preferences file exists")
      self._save_user_file()

    else:
      # Load the prefs file
      try:
        prefs_xml_str = self._core.storage.load(file_path, mtime_key=self)
        prefs_xml = self._core.data.xml.from_string(prefs_xml_str)
      
        # Iterate through each element
        for el in prefs_xml:
          pref_name = str(el.tag)

          # If a pref exists with this name, set its value
          if el.text != None and pref_name in self._prefs:
            user_values[pref_name] = str(el.text)
          
        self._core.log.debug("%s the user preferences for %s", ("Loaded" if len(self._user_values_dict) == 0 else "Reloaded"), self._identifier)

      except:
        self._core.log_exception("Exception loading user preferences from %s", file_path)

    self._user_values_dict = user_values
      
  def _save_user_file(self):
    if self._core.config.daemonized:
        self._core.log.debug("Ignored a request to save user preferences")
        return
    
    el = self._core.data.xml.element('PluginPreferences')
    
    for name, pref in self._prefs.items():
      el.append(self._core.data.xml.element(name, self._user_values_dict.get(name, pref.encode_value(pref.default_value))))
      
    prefs_xml = self._core.data.xml.to_string(el)
    self._core.storage.ensure_dirs(self._core.storage.join_path(self._core.plugin_support_path, 'Preferences'))
    self._core.storage.save(self._user_file_path, prefs_xml, mtime_key=self)
    self._core.log.debug("Saved the user preferences")
    

  @property
  def _user_values(self):
    with self._user_values_lock:
      # If the user prefs file doesn't exist, or if its mtime has changed, reload it.
      if self._core.storage.file_exists(self._user_file_path) == False or self._core.storage.has_changed(self._user_file_path, mtime_key=self):
        self._load_user_file()

      return self._user_values_dict


  def update_user_values(self, **kwargs):
    with self._user_values_lock:
      for name, value in kwargs.items():
        if isinstance(self._prefs.get(name), BooleanPref):
          value = 'true' if str(value).lower() in ['1', 'true'] else 'false'
        self._user_values_dict[name] = value

      self._save_user_file()


  @property
  def _prefs(self):
    with self._prefs_lock:
      if self._prefs_dict == None:
        self._load_prefs()
      return self._prefs_dict


  @property
  def default_prefs_path(self):
    return self._core.storage.join_path(self._core.bundle_path, 'Contents', 'DefaultPrefs.json')


  def _load_prefs(self):
    prefs_dict = {}
    file_paths = []
    prefs_json = []

    # Load the plug-in's DefaultPrefs.json file if we're running under a bundle policy.
    if self._identifier == self._core.identifier and self._sandbox.conforms_to_policy(Framework.policies.BundlePolicy):
      file_paths.append(self.default_prefs_path)
    
    # Check to see if any service preferences exist for this plug-in
    services = self._core.services
    for service_dict in (services.url_services, services.search_services, services.related_content_services):
      for service_set_identifier, service_set in service_dict.items():
        for service in service_set.values():
          if service.identifier == self._identifier:
            file_paths.append(self._core.storage.join_path(service.path, 'ServicePrefs.json'))

    # Iterate over the list of files and try to load prefs.
    for file_path in file_paths:
      if self._core.storage.file_exists(file_path):
        try:
          json = self._core.storage.load(file_path)
          json_array = self._core.data.json.from_string(json)
          prefs_json.extend(json_array)
          self._core.log.debug("Loaded preferences from %s", os.path.split(file_path)[1])

        except:
          self._core.log_exception("Exception loading preferences from %s", os.path.split(file_path)[1])
          
    # Iterate over the array loaded from the JSON files
    for pref in prefs_json:
      name = pref['id']
      
      # If a pref object with this name doesn't exist, try to create one
      if name not in self._pref_names:
        # Grab the type, default value, hidden state and label
        pref_type = pref['type']
        pref_secure = 'secure' in pref and (pref['secure'] == True or str(pref['secure']).lower() == 'true')
        pref_hidden = 'hidden' in pref and (pref['hidden'] == True or str(pref['hidden']).lower() == 'true')
        
        if 'default' in pref:
          pref_default = pref['default']
        else:
          pref_default = None
        pref_label = pref['label']
        
        # Find a suitable class...
        if pref_type == 'text':
          
          # Text prefs support options, so parse these too
          if 'option' in pref:
            pref_option = pref['option'].split(',')
          else:
            pref_option = []
          prefs_dict[name] = TextPref(pref_label, pref_default, pref_option, pref_secure, pref_hidden)
        
        elif pref_type == 'bool':
          prefs_dict[name] = BooleanPref(pref_label, pref_default, pref_secure, pref_hidden)
          
        elif pref_type == 'enum':
          # Enum prefs have a set of values - grab these
          if 'values' in pref:
            pref_values = pref['values']
          else:
            pref_values = []
          prefs_dict[name] = EnumPref(pref_label, pref_default, pref_values, pref_secure, pref_hidden)
          
        # Whoops - no class found. Ignore this pref.
        else:
          continue
          
        # Add the name to the names list
        self._pref_names.append(name)

    self._prefs_dict = prefs_dict

  def __getitem__(self, name):
    pref = self._prefs.get(name)

    if pref:
      value = None
      found = False

      # Check for a value stored in the current context, raising an exception if a secure preference value couldn't be found
      pref_values = self._sandbox.context.pref_values.get(self._identifier)
      if pref_values:
        if pref.secure == False or name in pref_values:
          value = pref_values.get(name)
          found = True
        else:
          raise Framework.exceptions.UnauthorizedException

      # If we're not on the node, check for a user value.
      if found == False and self._user_values and not self._core.config.daemonized:
        value = self._user_values.get(name, pref.default_value)
      
      return pref.decode_value(value if value else pref.default_value)

    raise KeyError("No preference named '%s' found." % name)



class PreferenceManager(object):

  def __init__(self, sandbox):
    self._sandbox = weakref.proxy(sandbox)
    self._sets = {}

  def get(self, identifier=None):
    if identifier == None:
      identifier = self._sandbox.identifier

    if identifier not in self._sets:
      self._sets[identifier] = PreferenceSet(self, identifier)

    return self._sets[identifier]
