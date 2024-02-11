#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import sys, os, pickle, traceback, string, urllib, time, random, shutil, urlparse, cgi, types, cerealizer
import PMS, Locale, HTTP, XML, Database, Prefs, Data, Dict, Resource, Objects, Thread, Helper, Client, JSON
from PMS.Shortcuts import *
from Routes import MatchRoute
import __objectManager

####################################################################################################    

__bundlePath = None

Identifier = None
Debug = False
UpdatingCache = False

####################################################################################################    

HTTP_TIMEOUT_VAR_NAME = "PLEX_MEDIA_SERVER_PLUGIN_TIMEOUT"
HTTP_DEFAULT_TIMEOUT = 20.0

__pluginModule = None
__logFilePath = None
__requestHandlers = {}
__prefixHandlers = {}
PublicFunctionArgs = {}
LastPrefix = None

__modBlacklist = []
__modWhitelist = []
__modChecked = []
__modBundled = []
__modLibs = []

__reservedFunctionNames = ["CreateDatabase", "CreatePrefs", "ValidatePrefs", "CreateDict", "UpdateCache", "Start"]
__cacheUpdateInterval = 600

__viewGroups = {}
ViewModes = {"List": 65586, "InfoList": 65592, "MediaPreview": 458803, "Showcase": 458810, "Coverflow": 65591, 
               "PanelStream": 131124, "WallStream": 131125, "Songs": 65593, "Seasons": 65593, "Albums": 131123, 
               "Episodes": 65590,"ImageStream":458809,"Pictures":131123}

####################################################################################################

def AddPrefixHandler(prefix, handler, name, thumb="icon-default.png", art="art-default.png", titleBar="titlebar-default.png"):
  global __prefixHandlers
  if prefix[-1] == "/":
    prefix = prefix[:-1]
  if not __prefixHandlers.has_key(prefix):
    handler_info = {"handler":handler, "name":name, "thumb":thumb, "art":art, "titleBar":titleBar}
    __prefixHandlers[prefix] = handler_info
    PMS.Log("(Framework) Added a handler for prefix '%s'" % prefix)
  else:
    PMS.Log("(Framework) Couldn't add a handler for prefix '%s' - prefix already exists" % prefix)
  if not __objectManager.FunctionIsParent("Start", "pms_fwk_handler_wrapper"):
    PMS.Log("(Framework) WARNING: Plugin.AddPrefixHandler() should only be called from within Start() method.")
    
####################################################################################################
  
def AddPathRequestHandler(prefix, handler, name, thumb="icon-default.png", art="art-default.png"):
  global __requestHandlers
  PMS.Log("(Framework) NOTICE: Path request handlers are deprecated and will be removed in the next major framework revision.")
  if prefix[-1] == "/":
    prefix = prefix[:-1]
  if not __requestHandlers.has_key(prefix):
    handler_info = {"handler":handler, "name":name, "thumb":thumb, "art":art}
    __requestHandlers[prefix] = handler_info
    PMS.Log("(Framework) Added a path request handler for prefix '%s'" % prefix)
  else:
    PMS.Log("(Framework) Couldn't add a path request handler for prefix '%s' - prefix already exists" % prefix)
  if not __objectManager.FunctionIsParent("Start"):
    PMS.Log("(Framework) WARNING: Plugin.AddPathRequestHandler() should only be called within the Start() method.")
    
####################################################################################################

def AddViewGroup(name, viewMode="List", mediaType="items"):
  global __viewGroups
  if viewMode in ViewModes.keys():
    __viewGroups[name] = {"ViewMode": str(ViewModes[viewMode]), "MediaType": mediaType}
    PMS.Log("(Framework) Added a view group named '%s'" % name)
  else:
    PMS.Log("(Framework) Couldn't create the view group '%s' - invalid view mode." % name)
  if not __objectManager.FunctionIsParent("Start"):
    PMS.Log("(Framework) WARNING: Plugin.AddViewGroup() should only be called within the Start() method.")
    
####################################################################################################

def Prefixes():
  global __requestHandlers
  global __prefixHandlers
  prefixes = list(__requestHandlers.keys())
  for prefix in __prefixHandlers.keys():
    if prefix not in prefixes:
      prefixes.append(prefix)
  return prefixes

####################################################################################################

def CurrentPrefix():
  global LastPrefix
  if LastPrefix: return LastPrefix
  else: return Prefixes()[0]

####################################################################################################

def ViewGroups():
  return dict(__viewGroups)

####################################################################################################

def Restart():
  path = os.path.join(__bundlePath, "Contents/Info.plist")
  if os.path.exists(path):
    os.utime(path, None)
    return True
  else:
    return False
    
def Traceback():
  return traceback.format_exc()

####################################################################################################

def __setupPermissionLists():
  global __modBlacklist
  global __modWhitelist
  global __modChecked
  global __modBundled
  global __modLibs
  __modBlacklist = ["os", "sys", "pickle", "socket", "urllib", "urllib2", "cookielib", "sqlite3", "email", "hashlib", "demjson", "traceback", "feedparser", "thread", "threading", "yaml", "lxml"]
  __modWhitelist = ["re", "string", "time", "datetime", "calendar", "operator"]
  __modChecked = []
  __modBundled = []
  __modLibs = []

####################################################################################################

def __scanModules():
  global __modBundled
  global __modLibs
  
  for path, dirs, files in os.walk(os.path.join(__bundlePath, "Contents/Code")):
    for d in dirs:
      __modBundled.append(d)
    for f in files:
      if f.endswith(".py"):
        if f != "__init__.py":
          __modBundled.append(f[:-3])
      elif not f.endswith(".pyc"):
        PMS.Log("(Framework) WARNING: Non-code file named '%s' found inside the Code directory. Future framework versions will abort at this point." % f)

  for path, dirs, files in os.walk(os.path.join(sys.path[0], "../Libraries")):
    for d in dirs:
      __modLibs.append(d)
    for f in files:
      if f != "__init__.py" and f.endswith(".py"):
        __modLibs.append(f[:-3])

####################################################################################################

def __checkModule(mod, allowed):
  # Gather all accessed modules
  global __modChecked
  global __modBundled
  accessed = []
  for n in mod.__dict__:
    if len(n) < 2:
      accessed.append(n)
    elif n[:2] != "__" and n[-2:] != "__": accessed.append(n)
  
  for n in accessed:
    if n not in allowed:
      _obj = mod.__dict__[n]
      if type(_obj).__name__ == "module" and n not in __modChecked and n not in __modLibs:
        __modChecked.append(n)
        if n in __modBlacklist:
          PMS.Log("(Framework) WARNING: The blacklisted module '%s' has been imported. Future framework versions will abort at this point." %n)
        else:
          if n not in __modBundled:
            PMS.Log("(Framework) NOTICE: The unknown module '%s' has been imported. This may be blacklisted in future and should be used with caution." %n)
          __checkModule(_obj, allowed)
      
####################################################################################################

def __checkFrameworkCompatibility():
  try:
    lastFrameworkVersion = Dict.Get("Framework.LastCompatibilityVersion")
    if lastFrameworkVersion != PMS.FrameworkCompatibilityVersion: raise
    return True
  except:
    PMS.Log("(Framework) Data stored by an earlier framework version has been removed due to incompatibilities.")
    if os.path.exists(Prefs.__prefsPath): os.unlink(Prefs.__prefsPath)
    if os.path.exists(Database.__databasePath): os.unlink(Database.__databasePath)
    shutil.rmtree(Data.__dataPath)
    os.makedirs(Data.__dataItemPath)
    Dict.__loadDefaults()
    Dict.__save(addToLog=False)
    return False
  

####################################################################################################

def __run(_bundlePath):
  #
  # Initializes the framework, verifies the plug-in & extracts information, then enters a
  # run loop for handling requests. 
  #
  global Identifier
  global Debug
  global __bundlePath  
  global __pluginModule
  global __logFilePath
  global __requestHandlers
  global LastPrefix
  
  FirstRun = False
  random.seed()
  
  # Set up the support file paths
  if sys.platform == "win32":
    if 'PLEXLOCALAPPDATA' in os.environ:
      key = 'PLEXLOCALAPPDATA'
    else:
      key = 'LOCALAPPDATA'
    pmsPath = os.path.join(os.environ[key], 'Plex Media Server')
    logFilesPath = os.path.join(os.environ[key], "Plex Media Server", "Logs", "PMS Plugin Logs")
  else:
    pmsPath = "%s/Library/Application Support/Plex Media Server" % os.environ["HOME"]
    logFilesPath = "%s/Library/Logs/Plex Media Server/PMS Plugin Logs" % os.environ["HOME"]
  
  supportFilesPath = "%s/Plug-in Support" % pmsPath
  frameworkSupportFilesPath = "%s/Framework Support" % pmsPath
  
  # Make sure framework directories exist
  def checkpath(path):
    try:
      if not os.path.exists(path): os.makedirs(path)
    except:
      pass
  
  checkpath("%s/Preferences" % supportFilesPath)
  checkpath("%s/Databases" % supportFilesPath)
  checkpath(logFilesPath)
  checkpath(frameworkSupportFilesPath)
    
  # Set the bundle path
  __bundlePath = _bundlePath.rstrip('/')
  
  # Add the bundle path to the system path, including any libraries
  if os.path.isdir("%s/Contents" % __bundlePath):
    sys.path.append("%s/Contents" % __bundlePath)
    if os.path.isdir("%s/Contents/Libraries" % __bundlePath):
      sys.path.append("%s/Contents/Libraries" % __bundlePath)
  else:
    print "Couldn't find bundle directory"
    return None
  
  # Open the Info.plist file
  f = open("%s/Contents/Info.plist" % __bundlePath, "r")
  infoplist = XML.ElementFromString(f.read())
  f.close()
  if infoplist is None:
    print "Couldn't load Info.plist file from plug-in"
    return

  # Get the plug-in identifier
  Identifier = infoplist.xpath('//key[text()="CFBundleIdentifier"]//following-sibling::string/text()')[0]
  if Identifier is None:
    print "Invalid Info.plist file in plug-in"
    return None
    
  # Set up the log file
  __logFilePath = "%s/%s.log" % (logFilesPath, Identifier)
  if os.path.exists(__logFilePath):
    if os.path.exists("%s.old" % __logFilePath):
      os.remove("%s.old" % __logFilePath)
    os.rename(__logFilePath, "%s.old" % __logFilePath)
  
  # Now we can start logging
  PMS.Log("(Framework) Bundle verification complete", False)
  
  # Check whether debugging is enabled
  try:
    _debug = infoplist.xpath('//key[text()="PlexPluginDebug"]//following-sibling::string/text()')[0]
    if _debug == "1":
      Debug = True
      PMS.Log("(Framework) Debugging is enabled")
  except: pass

  # Log the system encoding (set during bootstrap)
  PMS.Log("(Framework) Default encoding is " + sys.getdefaultencoding())

  # Set up framework paths
  Prefs.__prefsPath = "%s/Preferences/%s.xml" % (supportFilesPath, Identifier)
  Data.__dataPath = "%s/Data/%s" % (supportFilesPath, Identifier)
  Data.__dataItemPath = "%s/DataItems" % Data.__dataPath
  if not os.path.isdir(Data.__dataItemPath):
    FirstRun = True
    os.makedirs(Data.__dataItemPath)
  Resource.__resourcePath = "%s/Contents/Resources" % __bundlePath
  Helper.__helperPath = "%s/Contents/Helpers" % __bundlePath
  Resource.__sharedResourcePath = "%s/Plug-ins/Framework.bundle/Contents/Resources/Versions/1/Resources" % pmsPath
  Database.__databasePath = "%s/Databases/%s.db" % (supportFilesPath, Identifier)
  os.chdir(Data.__dataItemPath)
  Locale.SetDefaultLocale()
  PMS.Log("(Framework) Configured framework modules")

  # Attempt to import the plug-in module - if debugging is enabled, don't catch exceptions
  if Debug:
    import Code as _plugin
    PMS.Log("(Framework) Imported plug-in module")
  else:
    try:
      import Code as _plugin
      PMS.Log("(Framework) Imported plug-in module")
    except ImportError:
      PMS.Log("(Framework) Couldn't import plug-in from bundle")
      __exit()
      return
      
  # Load the list of trusted plug-ins
  _trusted = []
  try:
    _trustedJSON = Resource.LoadShared("trust.json")
    if _trustedJSON:
      _trusted = JSON.ObjectFromString(_trustedJSON)
  except:
    pass

  # Populate the permission lists
  __setupPermissionLists()

  # Register the plug-in with the framework
  __pluginModule = _plugin

  # Check the imported module to make sure nothing untoward is happening!
  if Identifier in _trusted:
    PMS.Log("(Framework) Plug-in is trusted, skipping module check")
  else:
    __scanModules()
    _allowed = []
    for n in PMS.__dict__:
      if n[0] != "_":
        if type(PMS.__dict__[n]).__name__ == "module":
          _allowed.append(n)
    for n in __modWhitelist:
      _allowed.append(n)
    __checkModule(_plugin, _allowed)
    PMS.Log("(Framework) Checked module imports")

  # Initialize the framework modules
  Dict.__load()
  if not FirstRun:
    __checkFrameworkCompatibility()
  Prefs.__load()
  HTTP.__loadCookieJar()
  HTTP.__loadCache()
  if HTTP_TIMEOUT_VAR_NAME in os.environ:
    HTTP.SetTimeout(float(os.environ[HTTP_TIMEOUT_VAR_NAME]))
  else:
    HTTP.SetTimeout(HTTP_DEFAULT_TIMEOUT)
  PMS.Log("(Framework) Initialized framework modules")

  # Call the plug-in's Start method
  PMS.Log("(Framework) Attempting to start the plug-in...")
  __call(__pluginModule.Start)
  PMS.Log("(Framework) Plug-in started", False)

  # Start timers
  __startCacheManager(firstRun=FirstRun)
  
  PMS.Log("(Framework) Entering run loop")
  # Enter a run loop to handle requests
  while True:
    try:
      # Read the input
      path = raw_input()
      path = path.lstrip("GET ").strip()
      LastPrefix = None
      
      # Read headers
      headers = {}
      stop = False
      while stop == False:
        line = raw_input()
        if len(line) == 1:
          stop = True
        else:
          split = string.split(line.strip(), ":", maxsplit=1)
          if len(split) == 2:
            headers[split[0].strip()] = split[1].strip()

      # Set the locale
      if headers.has_key("X-Plex-Language"):
        loc = headers["X-Plex-Language"].lower()
        Locale.__loadLocale(loc)
        
      # Set the version
      if headers.has_key("X-Plex-Version"):
        Client.__setVersion(headers["X-Plex-Version"])

      PMS.Request.Headers.clear()
      PMS.Request.Headers.update(headers)
      
      req = urlparse.urlparse(path)
      path = req.path
      qs_args = cgi.parse_qs(req.query)
      kwargs = {}
      if 'function_args' in qs_args:
        try:
          unquoted_args = urllib.unquote(qs_args['function_args'][0])
          decoded_args = PMS.String.Decode(unquoted_args)
          kwargs = cerealizer.loads(decoded_args)
        except:
          PMS.Log(PMS.Plugin.Traceback())
          raise Exception("Unable to deserialize arguments")
      
      for arg_name in qs_args:
        if arg_name != 'function_args':
          kwargs[arg_name] = qs_args[arg_name][0]
      
      # First, try to match a connected route
      rpath = path
      for key in __prefixHandlers:
        if rpath.count(key, 0, len(key)) == 1:
          rpath = rpath[len(key):]
          break
      f, route_kwargs = MatchRoute(rpath)
      
      if f is not None:
        PMS.Log("(Framework) Handling route request :  %s" % path, False)
        route_kwargs.update(kwargs)
        result = f(**route_kwargs)

      # No route, fall back to other path handling methods
      else:
        mpath = path
        
        if mpath[-1] == "/":
          mpath = mpath[:-1]
        
        # Split the path into components and decode.
        pathNouns = path.split('/')
        pathNouns = [urllib.unquote(p) for p in pathNouns]
      
        # If no input was given, return an error
        if len(pathNouns) <= 1:
          __return("%s\r\n\r\n" % PMS.Error['BadRequest'])
        
        # Otherwise, attempt to handle the request
        else:
          result = None
          pathNouns.pop(0)
          count = len(pathNouns)
          if pathNouns[-1] == "":
            pathNouns.pop(len(pathNouns)-1)
          PMS.Log("(Framework) Handling request :  %s" % path, False)
        
          # Check for a management request
          if pathNouns[0] == ":":
            result = __handlePMSRequest(pathNouns, path, **kwargs)

          else:  
            handler = None
            isPrefixHandler = False
          
            # See if there's a prefix handler available
            for key in __prefixHandlers:
              if mpath.count(key, 0, len(key)) == 1:
                LastPrefix = key
            if mpath in __prefixHandlers:
              handler = __prefixHandlers[mpath]["handler"]
              isPrefixHandler = True
            
            else:
              # Check each request handler to see if it handles the current prefix
              popped = False
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
                    LastPrefix = key
                    popped = True
            
              # If no path request handler was found, make sure we still pop the prefix so internal requests work
              for key in __prefixHandlers:
                if popped == False:
                  if mpath.count(key, 0, len(key)) == 1:
                    keyNounCount = len(key.split('/')) - 1
                    for i in range(keyNounCount):
                      pathNouns.pop(0)
                    popped = True

            # Check whether we should handle the request internally
            handled = False
            if count > 0:
              if pathNouns[0] == ":":
                handled = True
                result = __handleInternalRequest(pathNouns, path, **kwargs)
    
          
            # Check if the App Store has flagged the plug-in as broken
            if os.path.exists(os.path.join(frameworkSupportFilesPath, "%s.broken" % Identifier)):
              #TODO: Localise this bit, use message from the App Store if available
              handled = True
              result = PMS.Objects.MessageContainer("Please try again later", "This plug-in is currently unavailable")
              PMS.Log("(Framework) Plug-in is flagged as broken")
          
            # If the request hasn't been handled, and we have a valid request handler, call it
            else:
              if not handled and handler is not None:
                if isPrefixHandler:
                  result = handler(**kwargs)
                else:
                  result = handler(pathNouns, path, **kwargs)
        
      response = None
      
      # If the request wasn't handled, return an error
      if result == None:
        PMS.Log("(Framework) Request not handled by plug-in", False)
        response = "%s\r\n\r\n" % PMS.Error['NotFound']
        
      # If the plugin returned an error, return it to PMS
      elif result in PMS.Error.values():
        PMS.Log("(Framework) Plug-in returned an error :  %s" % result, False)
        response = "%s\r\n\r\n" % result
        
      # Otherwise, check if a valid object was returned, and return the result
      elif __objectManager.ObjectHasBase(result, Objects.Object):
        resultStr = result.Content()
        resultStatus = result.Status()
        resultHeaders = result.Headers()
      
      elif isinstance(result, basestring):
        resultStr = result
        resultStatus = '200 OK'
        resultHeaders = "Content-type: text/plain\r\n"
      
      if resultStr is not None:
        PMS.Log("(Framework) Response OK")
        resultLen = len(resultStr)
        if resultLen > 0:
          resultHeaders += "Content-Length: %i\r\n" % resultLen
          resultStr = "\r\n"+resultStr
        
      else:
        resultStr = ""
        resultStatus = '500 Internal Server Error'
        resultHeaders = ''
        PMS.Log("(Framework) Unknown response type")
        
      if response == None:
        response = str("%s\r\n%s" % (resultStatus, resultHeaders)) + str(resultStr) + str("\r\n")
      
      #PMS.Log("\n---\n"+response+"---\n")
        
      __return(response)
    
    # If a KeyboardInterrupt (SIGINT) is raised, stop the plugin
    except KeyboardInterrupt:
      # Save data & exit
      __saveData()
      __exit()     
    
    except EOFError:
      # Save data & exit
      __saveData()
      __exit()
          
    # If another exception is raised, deal with the problem
    except:
      __except()
      __return("%s\r\n\r\n" % PMS.Error['InternalError'])
    
    # Make sure the plugin's data is saved
    finally:
      __saveData()
      
####################################################################################################    

def __handlePMSRequest(pathNouns, path, **kwargs):
  #
  # Handle a management request from PMS
  #
  global __requestHandlers
  if len(pathNouns) > 1:
    
    # If PMS is requesting a list of prefixes, construct a MediaContainer and return it
    if pathNouns[1] == "prefixes" and len(pathNouns) == 2:
      dir = Objects.MediaContainer()
      hasPrefs = len(Prefs.__prefs) > 0
      for key in __requestHandlers:
        handler = __requestHandlers[key]
        dir.Append(Objects.XMLObject(tagName="Prefix", key=key, name=handler["name"], thumb=R(handler["thumb"]), art=R(handler["art"]), titleBar=R(handler["titleBar"]), hasPrefs=hasPrefs, identifier=Identifier))
      for key in __prefixHandlers:
        handler = __prefixHandlers[key]
        dir.Append(Objects.XMLObject(tagName="Prefix", key=key, name=handler["name"], thumb=R(handler["thumb"]), art=R(handler["art"]), titleBar=R(handler["titleBar"]), hasPrefs=hasPrefs, identifier=Identifier))
      return dir
      
    # Set rating
    elif pathNouns[1][:4] == "rate":
      try:
        rating = int(kwargs["rating"])
        if rating >= 0 and rating <= 10:
          __callNamed("SetRating", key=String.Decode(kwargs["key"]), rating=rating)
      except:
        __except()
      return
      
    # Jump directly to the root prefix based on identifier
    elif len(pathNouns) == 4 and pathNouns[1] == 'plugins' and pathNouns[3] == 'root':
      keys = __prefixHandlers.keys()
      keys.extend(__requestHandlers.keys())
      for key in keys:
        if key.startswith('/video'):
          return Objects.Redirect(key)
      for key in keys:
        if key.startswith('/music'):
          return Objects.Redirect(key)
      return Objects.Redirect(keys[0])
      
####################################################################################################    

def __handleInternalRequest(pathNouns, _path, **kwargs):
  #
  # Handle a request internally
  #
  if len(pathNouns) > 1:
    if pathNouns[1] == "resources":
      if len(pathNouns) == 3:
        ext = pathNouns[2][pathNouns[2].rfind("."):]
        PMS.Log("(Framework) Getting resource named '%s'" % pathNouns[2])
        resource = Resource.Load(pathNouns[2])
        return Objects.DataObject(resource, Resource.MimeTypeForExtension(ext))

    if pathNouns[1] == "sharedresources":
      if len(pathNouns) == 3:
        ext = pathNouns[2][pathNouns[2].rfind("."):]
        PMS.Log("(Framework) Getting shared resource named '%s'" % pathNouns[2])
        resource = Resource.LoadShared(pathNouns[2])
        return Objects.DataObject(resource, Resource.MimeTypeForExtension(ext))
        
    elif pathNouns[1] == "function" and len(pathNouns) >= 3:
      name = pathNouns[2]
      pos = name.rfind(".")
      if pos > -1:
        name = name[:pos]
      PMS.Log(name)
      if name not in __reservedFunctionNames:
        if len(pathNouns) == 4:
          kwargs['query'] = pathNouns[3]
          
        result = __callNamed(name, **kwargs)
        #PMS.Log(result)
        return result
        
    elif pathNouns[1] == "prefs":
      if len(pathNouns) == 2:
        return Prefs.__container()
      else:
        Prefs.__setAll(kwargs)
        result = __callNamed("ValidatePrefs", addToLog=False)
        if result:
          return result
        else:
          return ''

####################################################################################################

def __startCacheManager(firstRun=False):
  Thread.CreateTimer(HTTP.__autoUpdateCacheTime, __triggerAutoHTTPCacheUpdate)
  if "UpdateCache" in __pluginModule.__dict__:
    if firstRun:
      Thread.Create(__triggerCacheUpdate)
    else:
      Thread.CreateTimer((__cacheUpdateInterval/2)+random.randrange(__cacheUpdateInterval), __triggerCacheUpdate)
    PMS.Log("(Framework) Cache manager started")

####################################################################################################

def __triggerCacheUpdate():
  global UpdatingCache
  if UpdatingCache: return
  UpdatingCache = True
  __callNamed("UpdateCache", addToLog=False)
  Thread.CreateTimer(__cacheUpdateInterval, __triggerCacheUpdate)
  UpdatingCache = False

####################################################################################################
  
def __triggerAutoHTTPCacheUpdate():
  HTTP.__autoUpdateCachedPages()
  Thread.CreateTimer(HTTP.__autoUpdateCacheTime, __triggerAutoHTTPCacheUpdate)

####################################################################################################

def __call(function, *args, **kwargs):
  #
  # Call a function, ensuring that the plug-in's data is saved afterwards
  #
  try:
    return function(*args, **kwargs)
  except:
    __except()
  finally:
    __saveData()
    
####################################################################################################

def __callNamed(functionName, addToLog=True, *args, **kwargs):
  global __pluginModule
  if functionName in __pluginModule.__dict__:
    if addToLog:
      PMS.Log("(Framework) Calling named function '%s'" % functionName)
    return __call(__pluginModule.__dict__[functionName], *args, **kwargs)
  elif addToLog:
    PMS.Log("(Framework) Named function '%s' couldn't be found" % functionName)

####################################################################################################

def __return(string):
  sys.stdout.write(string)
  sys.stdout.flush()

####################################################################################################

def __except():
  # If in debug mode, print the traceback, otherwise report an internal error
  if Debug:
    PMS.Log("(Framework) An exception happened:\n%s" % traceback.format_exc())
  else:
    PMS.Log("(Framework) An internal error occurred", False)
    
####################################################################################################

def __saveData():
  if Database.__changed and Database.__db is not None:
    Database.Commit()
    
####################################################################################################

def __exit():
  if Database.__db is not None:
    Database.__db.close()
  PMS.Log("(Framework) Plug-in stopped")
  sys.exit()
  
####################################################################################################

