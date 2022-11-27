#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  Functions for accessing information about the plugin environment.
"""

import sys, os, pickle, traceback, urllib
import PMS, Locale, HTTP, XML, DB, Prefs, Log, MediaXML
from MediaXML import MediaContainer, MediaItem

####################################################################################################    

BundlePath = None
""" The path to the plugin bundle """
ResourcesPath = None
""" The path to the Resources directory inside the plugin bundle """
Identifier = None
""" The reverse DNS identifier of the plugin """
DataPath = None
""" The path to the plugin's data directory """
Debug = False
""" Check whether debugging is enabled """


HTTP_TIMEOUT_VAR_NAME = "PLEX_MEDIA_SERVER_PLUGIN_TIMEOUT"
HTTP_DEFAULT_TIMEOUT = 20.0

Response = {}
MimeTypes = {}
ViewGroups = {}
Dict = {}
__savedDict = {}

####################################################################################################    

__pluginModule = None
__prefs = None
__prefsPath = None
__databasePath = None
__logFilePath = None
__requestHandlers = {}
__publicResources = {}

####################################################################################################

__XML_RESPONSE_OK = "<InternalResponse>OK</InternalResponse>"

####################################################################################################    

def AddRequestHandler(prefix, handler, name="", thumb="", art="", titleBar=""):
  """
    Add a request handler for a given path prefix. This must be a function within the plugin which
    accepts pathNouns and count as arguments. The prefix should not include a trailing '/'
    character.
    
    @param prefix: The path prefix to be handled
    @type prefix: string
    @param handler: The function that will handle the request
    @type handler: function
    @param name: The display name for the prefix
    @type prefix: string
    @param thumb: The name of an exposed resource file to use as a thumbnail
    @type thumb: string
    @param art: The name of an exposed resource file to use as artwork
    @type art: string
  """
  global __requestHandlers
  if not __requestHandlers.has_key(prefix):
    handler_info = {"handler":handler, "name":name, "thumb":thumb, "art":art, "titleBar":titleBar}
    __requestHandlers[prefix] = handler_info
    Log.Add("(Framework) Adding request handler for prefix '%s'" % prefix)

####################################################################################################

def AddViewGroup(name, viewMode="list", contentType="items"):
  global ViewGroups
  if viewMode in MediaXML.ViewModes.keys():
    Log.Add("(Framework) Adding view group '%s'" % name)
    ViewGroups[name] = {"ViewMode": str(MediaXML.ViewModes[viewMode]), "ContentType": contentType}
  else:
    Log.Add("(Framework) Couldn't create view group '%s' - invalid view mode." % name)

####################################################################################################

def AddMimeType(extension, mimetype):
  global MimeTypes
  MimeTypes[extension] = mimetype

####################################################################################################

def Prefixes():
  global __requestHandlers
  return __requestHandlers.keys()

####################################################################################################

def Redirect(url):
  global Response
  Response["Status"] = "301 Moved Permanently"
  Response["Headers"] = "Location: %s" % url
  return ""
  
####################################################################################################    

def ResourceFilePath(file_name):
  """
    Returns the full path to a file in the Resources directory inside the plugin bundle.
    
    @param file_name: The name of the file
    @type file_name: string
    @return: string
  """
  global ResourcesPath
  return ResourcesPath + "/" + file_name
  
####################################################################################################    

def Resource(file_name):
  """
    Returns the data contained in the given file in the Resources directory inside the plugin
    bundle.

    @param file_name: The name of the file
    @type file_name: string
    @return: data
  """
  f = open(ResourceFilePath(file_name), 'r')
  d = f.read()
  f.close()
  Log.Add("(Framework) Loaded resource named '%s'" % file_name)
  return d
  
####################################################################################################

def ExposeResource(resource_name, content_type):
  """
    Make the given file in the Resources directory inside the plugin bundle available
    publically via request handlers. Returns True if the file was found or False otherwise.

    @param resource_name: The name of the file
    @type resource_name: string
    @param content_type: The content type to use when returning the data in the file
    @type content_type: string
    @return: boolean
  """
  global __publicResources
  if os.path.exists(ResourceFilePath(resource_name)) and not __publicResources.has_key(resource_name):
    __publicResources[resource_name] = content_type
    Log.Add("(Framework) Exposed resource named '%s' as '%s'" % (resource_name, content_type))
    return True
  else: return False

####################################################################################################

def ExposedResourcePath(resource_name):
  """
    Returns the public request path to an exposed resource file, or None if the file was not found.

    @param resource_name: The name of the file
    @type resource_name: string
    @return: boolean
  """
  global __publicResources
  global __requestHandlers
  global MimeTypes
  if resource_name is None: return
  ext = resource_name[resource_name.rfind("."):]
  if MimeTypes.has_key(ext):
    ExposeResource(resource_name, MimeTypes[ext])
  else:
    ExposeResource(resource_name, "application/octet-stream")
  if __publicResources.has_key(resource_name) and len(__requestHandlers) > 0:
    return "%s/:/resources/%s" % (__requestHandlers.keys()[0], resource_name)
  else:
    return None

####################################################################################################    

def DataFilePath(file_name):
  """
    Returns the full path to a file in the plugin's data directory.
    
    @param file_name: The name of the file
    @type file_name: string
    @return: string
  """
  global DataPath
  return DataPath + "/" + file_name
  
####################################################################################################    

def DataFile(file_name):
  """
    Returns the data contained in the given file in the plugin's data directory.

    @param file_name: The name of the file
    @type file_name: string
    @return: data
  """
  f = open(DataFilePath(file_name), 'r')
  d = f.read()
  f.close()
  Log.Add("(Framework) Loaded data file named '%s'" % file_name)
  return d
  
####################################################################################################

def SaveDataToFile(file_name, data):
  """
    Saves data to a given file in the plugin's data directory.

    @param file_name: The name of the file
    @type file_name: string
    @param data: The data to save
    @type data: any
  """
  f = open(DataFilePath(file_name), 'w')
  f.write(data)
  f.close()
  Log.Add("(Framework) Saved data file named '%s'" % file_name)

####################################################################################################

def LoadDict():
  """
    Load the plugin's dictionary from the data directory.
  """
  global Dict
  global __savedDict
  pickle_file = DataFilePath("DictPickle")
  try:
    if os.path.exists(pickle_file):
      f = open(pickle_file, "r")   
      __savedDict = pickle.load(f)
      f.close()
      Log.Add("(Framework) Plugin dictionary unpickled")
    else:
      __savedDict = {}
  except:
    Log.Add("(Framework) Unable to load plugin dictionary - file appears to be corrupt")
    __savedDict = {}
  Dict = __savedDict.copy()

####################################################################################################

def SaveDict():
  """
    Save the plugin's dictionary to the data directory if it has been modified.
  """
  global Dict
  global __savedDict
  if Dict != __savedDict:
    Log.Add("(Framework) Dictionary has been changed")
    __savedDict = Dict.copy()
    pickle_file = DataFilePath("DictPickle")
    f = open(pickle_file, "w")
    pickle.dump(__savedDict, f, 2)
    Log.Add("(Framework) Plugin dictionary pickled")

####################################################################################################
    
def ResetDict():
  """
    Reset the plugin's dictionary. This will erase all previously stored values.
  """
  pickle_file = DataFilePath("DictPickle")
  if os.path.exists(pickle_file):
    os.remove(pickle_file)
  Dict = {}
  __savedDict = {}
  Log.Add("(Framework) Plugin dictionary reset")

####################################################################################################

def __run(_bundlePath):
  #
  # Initializes the plugin framework, verifies the plugin & extracts information, then enters a
  # run loop for handling requests. 
  #
  global BundlePath
  global ResourcesPath
  global Identifier
  global DataPath
  global Debug
  
  global __pluginModule
  global __prefs
  global __prefsPath
  global __databasePath
  global __logFilePath
  global __requestHandlers
  
  if sys.platform == "win32":
    if 'PLEXLOCALAPPDATA' in os.environ:
      key = 'PLEXLOCALAPPDATA'
    else:
      key = 'LOCALAPPDATA'
    supportFilesPath = os.path.join(os.environ[key], "Plex Media Server", "Plug-in Support")
    logFilesPath = os.path.join(os.environ[key], "Plex Media Server", "Logs", "PMS Plugin Logs")
  else:
    supportFilesPath = os.environ["HOME"] + "/Library/Application Support/Plex Media Server/Plug-in Support"
    logFilesPath = os.environ["HOME"] + "/Library/Logs/Plex Media Server/PMS Plugin Logs/"
  
  def checkpath(path):
    try:
      if not os.path.exists(path): os.makedirs(path)
    except:
      pass
  
  checkpath(supportFilesPath + "/Preferences")
  checkpath(supportFilesPath + "/Databases")
  checkpath(logFilesPath)
  
  # Set the bundle path variable
  BundlePath = _bundlePath.rstrip('/')
  ResourcesPath = BundlePath + "/Contents/Resources"
  
  # Add the bundle path to the system path
  if os.path.isdir(BundlePath + "/Contents"):
    sys.path.append(BundlePath + "/Contents")
    if os.path.isdir(BundlePath + "/Contents/Libraries"):
      sys.path.append(BundlePath + "/Contents/Libraries")
  else:
    print "Couldn't find bundle directory"
    return None
  
  # Open the Info.plist file
  infoplist = XML.ElementFromFile((BundlePath + "/Contents/Info.plist"))
  if infoplist is None:
    print "Couldn't load Info.plist file from plugin"
    return

  # Get the plugin identifier
  Identifier = infoplist.xpath('//key[text()="CFBundleIdentifier"]//following-sibling::string/text()')[0]
  if Identifier is None:
    print "Invalid Info.plist file in plugin"
    return None
    
  # Set up the log file
  __logFilePath = logFilesPath + Identifier + ".log"
  if os.path.exists(__logFilePath):
    if os.path.exists(__logFilePath + ".old"):
      os.remove(__logFilePath + ".old")
    os.rename(__logFilePath, __logFilePath + ".old")
  
  # Show a big warning message - Framework v0 is deprecated!!
  Log.Add("(Framework) Deprecated version\n\nNOTICE: This version of the Plex Media Framework is deprecated and is no longer supported.\nPlease migrate your code to a newer version. More information can be found at http://dev.plexapp.com/\n")
  
  Log.Add("(Framework) Plugin initialized", False)
  
  # Check whether debugging is enabled
  try:
    _debug = infoplist.xpath('//key[text()="PlexPluginDebug"]//following-sibling::string/text()')[0]
    if _debug == "1":
      Debug = True
      Log.Add("(Framework) Debugging is enabled")
  except: pass
  
  # Create the data path if it doesn't already exist
  DataPath = supportFilesPath + "/Data/" + Identifier
  if not os.path.isdir(DataPath):
    os.makedirs(DataPath)
  
  # Change directory to the data path
  os.chdir(DataPath)
  
  # If a preference file exists, load it
  __prefsPath = supportFilesPath + "/Preferences/" + Identifier + ".xml"
  defaultsPath = BundlePath + "/Contents/Defaults.xml"
  if os.path.exists(__prefsPath):
    __prefs = XML.ElementFromFile(__prefsPath)
    Log.Add("(Framework) Loaded user preferences")

  # Otherwise, try to apply the defaults file
  elif os.path.exists(defaultsPath):
    __prefs = XML.ElementFromFile(defaultsPath)
    Log.Add("(Framework) Loaded default preferences")
    
  # If no preferences were loaded, create an empty preferences element
  else:
    __prefs = XML.Element("PluginPreferences")
  
  # Load the plugin's dictionary file
  LoadDict()
  
  # Load the plugin's localization strings
  Locale.__loadDefaults()
  # TODO: Retrieve locale info from PMS for overriding default dict strings 
  # Locale.__loadLocale(loc)
  
  # Set the database file path
  __databasePath = supportFilesPath + "/Databases/" + Identifier + ".db"
  
  # Initialize the plugin's CookieJar
  HTTP.__loadCookieJar()
  Log.Add("(Framework) Loaded cookie jar")
  
  if HTTP_TIMEOUT_VAR_NAME in os.environ:
    HTTP.SetTimeout(float(os.environ[HTTP_TIMEOUT_VAR_NAME]))
  else:
    HTTP.SetTimeout(HTTP_DEFAULT_TIMEOUT)
  
  # Attempt to import the plugin module - if debugging is enabled, don't catch exceptions
  if Debug:
    import Code as _plugin
    Log.Add("(Framework) Imported plugin module")
  else:
    try:
      import Code as _plugin
      Log.Add("(Framework) Imported plugin module")
    except ImportError:
      Log.Add("(Framework) Couldn't import plugin from bundle")
      return
  
  __pluginModule = _plugin
  
  # Call the plugin's Start method
  Log.Add("(Framework) Attempting to start the plugin...")
  __call(_plugin.Start)
  Log.Add("(Framework) Plugin started", False)


  Log.Add("(Framework) Entering run loop")
  # Enter a run loop to handle requests
  while True:
    try:
      # Read the input
      path = raw_input()
      
      # Strip GET from the start of the path
      path = path.lstrip("GET ").strip()
      
      # Split the path into components and decode.
      pathNouns = path.replace('?query=', '/').split('/')
      pathNouns = [urllib.unquote(p) for p in pathNouns]
      
      # If no input was given, return an error
      if len(pathNouns) <= 1:
        sys.stdout.write("%s\r\n\r\n" % PMS.Error['BadRequest'])
        sys.stdout.flush()
        
      # Otherwise, attempt to handle the request
      else:
        Response['Content-Type'] = 'application/xml'
        Response['Status'] = '200 OK'
        Response["Headers"] = ""
        result = None
        pathNouns.pop(0)
        count = len(pathNouns)
        if pathNouns[count-1] == "":
          count = count - 1
          pathNouns.pop(len(pathNouns)-1)
          
        Log.Add("(Framework) Handling request :  " + path, False)
        
        # Check for a management request
        if pathNouns[0] == ":":
          result = __handlePMSRequest(pathNouns, count)

        else:  
          # Check each request handler to see if it handles the current prefix
          handler = None
          for key in __requestHandlers:
            if handler is None:
              if path.count(key, 0, len(key)) == 1:
                # Remove the prefix from the path
                keyNounCount = len(key.split('/')) - 1
                for i in range(keyNounCount):
                  pathNouns.pop(0)
                count = count - keyNounCount
                # Find the request handler
                handler = __requestHandlers[key]["handler"]
                
          # Check whether we should handle the request internally
          handled = False
          if count > 0:
            if pathNouns[0] == ":":
              handled = True
              result = __handleInternalRequest(pathNouns, count)
    
          # If the request hasn't been handled, and we have a valid request handler, call it
          if not handled and handler is not None:
            result = handler(pathNouns, count)
        
        # If the request wasn't handled, return an error
        if result == None:
          Log.Add("(Framework) Request not handled by plugin", False)
          response = "%s\r\n\r\n" % PMS.Error['NotFound']
          
        # If the plugin returned an error, return it to PMS
        elif result in PMS.Error.values():
          Log.Add("(Framework) Plug-in returned an error :  %s" % result, False)
          response = result + "\r\n"
          
        # Otherwise, return the result
        else:
          Log.Add("(Framework) Response OK")
          response = "%s\r\nContent-Type: %s\r\nContent-Length: %i\r\n%s\r\n%s\r\n" % \
            (Response["Status"], str(Response['Content-Type']), len(result), Response["Headers"], result)

        sys.stdout.write(response)
        sys.stdout.flush()
    
    # If a KeyboardInterrupt (SIGINT) is raised, stop the plugin
    except KeyboardInterrupt:
      # Commit any changes to the database and close it
      if DB.__db is not None:
        DB.Commit()
        DB.__db.close()
      # Save the dictionary
      SaveDict()
      # Exit
      sys.exit()
    
    except EOFError:
      # Commit any changes to the database and close it
      if DB.__db is not None:
        DB.Commit()
        DB.__db.close()
      Log.Add("(Framework) Plugin stopped")
      sys.exit()
          
    # If another exception is raised, deal with the problem
    except:
      # If in debug mode, print the traceback, otherwise report an internal error
      if Debug:
        Log.Add("(Framework) An exception happened:\n%s" % traceback.format_exc())
      else:
        Log.Add("(Framework) An internal error occurred", False)
      sys.stdout.write("%s\r\n\r\n" % PMS.Error['InternalError'])
      sys.stdout.flush()
    
    # Make sure the plugin's dictionary is saved
    finally:
      SaveDict()
      
####################################################################################################    

def __handlePMSRequest(pathNouns, count):
  #
  # Handle a management request from PMS
  #
  global __requestHandlers
  if count > 1:
    
    # If PMS is requesting a list of prefixes, construct a MediaContainer and return it
    if pathNouns[1] == "prefixes" and count == 2:
      dir = MediaContainer()
      for key in __requestHandlers:
        handler = __requestHandlers[key]
        item = MediaItem("Prefix")
        item.SetAttr("key", key)
        item.SetAttr("name", handler["name"])
        item.SetAttr("thumb", ExposedResourcePath(handler["thumb"]))
        item.SetAttr("art", ExposedResourcePath(handler["art"]))
        item.SetAttr("titleBar", ExposedResourcePath(handler["titleBar"]))
        dir.AppendItem(item)
      return dir.ToXML()
      
    # Jump directly to the root prefix based on identifier
    elif len(pathNouns) == 4 and pathNouns[1] == 'plugins' and pathNouns[3] == 'root':
      keys = __requestHandlers.keys()
      Response['Status'] = '301 Moved Permanently'
      for key in keys:
        if key.startswith('/video'):
          Response['Headers'] = 'Location: %s\r\n' % key
          return ''
      for key in keys:
        if key.startswith('/music'):
          Response['Headers'] = 'Location: %s\r\n' % key
          return ''
      Response['Headers'] = 'Location: %s\r\n' % keys[0]
      return ''

####################################################################################################    

def __handleInternalRequest(pathNouns, count):
  #
  # Handle a request internally
  #
  global __publicResources
  if count > 1:
    if pathNouns[1] == "resources":
      if count == 3:
        if __publicResources.has_key(pathNouns[2]):
          Log.Add("Getting resource")
          resource = Resource(pathNouns[2])
          Response["Content-Type"] = __publicResources[pathNouns[2]]
          return resource

    elif pathNouns[1] == "prefs":
      return Prefs.__handleRequest(pathNouns,count)

####################################################################################################

def __call(function):
  #
  # Call a function, ensuring that the plugin's dictionary is saved afterwards
  #
  try:
    function()
  finally:
    SaveDict()
    
####################################################################################################
