#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from systemservice import SystemService
import cgi
import os

UPDATE_FILE = 'AgentServicePendingUpdates.xml'
AGENT_INFO_KEY = '_AgentService:AgentInfo'

ATTRIBUTION_PLIST_KEY = 'PlexAgentAttributionText'

class InitiateUpdateResponse(FactoryClass(XMLObject)):
  pass
InitiateUpdateResponse = Factory(InitiateUpdateResponse)

class SearchResponse(FactoryClass(XMLObject)):
  pass
SearchResponse = Factory(SearchResponse)
  
class AgentResponse(object):
  OK                        = dict(code=0, status='OK')
  InvalidMediaType          = dict(code=1, status='Invalid media type')
  AgentNotFound             = dict(code=2, status='Agent not found')
  MediaTypeNotHandled       = dict(code=3, status='Agent does not handle the given media type')
  LanguageNotHandled        = dict(code=4, status='Agent does not handle the given language')
  UnhandledException        = dict(code=5, status='Unhandled exception')             
  
class AgentService(SystemService):
  """
    The agent service responds to metadata requests from the media server and forwards them to the
    appropriate plug-in, formatting and combining results as required.
  """
  
  def __init__(self, system):
    SystemService.__init__(self, system)
    self.processing_updates = {}
    self.pending_updates = []
    self.guid_mutexes = {}
    self.guid_mutexes_lock = Thread.Lock()
    self.update_lock = Thread.Lock()
    self.agent_lock = Thread.Lock()
    
    self.searching = False
    if AGENT_INFO_KEY in Dict:
      self.agent_info = Dict[AGENT_INFO_KEY]
    else:
      self.agent_info = {}
      
    Log.Debug("Starting the agent service")
    
    self.media_types = {
      1: 'Movie',
      2: 'TV_Show',
      8: 'Artist',
      9: 'Album',
     13: 'Photo',
	 # Game type has been removed
	 # 19: 'Game',
     20: 'Plex_Personal_Media'
    }
    
    self.dummy_agents = {
      'Movie': 'Home Movies',
      'TV_Show': 'Home Movie Series',
      'Plex_Personal_Media': 'Plex Personal Media'
    }
    
    # Create a model accessor
    self.accessor = Framework.modelling.ModelAccessor(
      Core,
      'metadata',
      Core.storage.join_path(Core.framework_path, 'Models', 'Metadata', '__init__.pym'),
      Core.storage.join_path(Core.app_support_path, 'Metadata')
    )
    
    self.access_point = self.accessor.get_access_point(Core.identifier)
    
    # WIP: Create separate combiners for different languages
    if Core.config.root_path:
      self.combiners = {}
      self.combiner = None
      
    else:
      # Create a single bundle combiner when running against PMS
      self.combiner = Framework.modelling.BundleCombiner(
        Core,
        Core.storage.join_path(Core.app_support_path, 'Metadata'),
        Core.storage.join_path(Core.plugin_support_path, 'Metadata Combination')
      )
      self.combiners = None
    
    # Create a queue for dispatching tasks in parallel, and a queue for notifying PMS of updated metadata bundles
    self.queue = Core.runtime.taskpool.create_limiting_queue(Core.config.agentservice_update_limit)
    self.notify_queue = Core.runtime.taskpool.create_limiting_queue(1)
      
    # Connect routes
    Route.Connect('/system/agents/search', self.search)
    Route.Connect('/system/agents/update', self.update)
    Route.Connect('/system/agents', self.list)
    Route.Connect('/system/agents/{identifier}/config/{mediaType}', self.get_agent_configuration)
    Route.Connect('/system/agents/{identifier}/config/{mediaType}', self.set_agent_configuration, method='PUT')
    Route.Connect('/system/agents/contributors', self.list_contributors)
    Route.Connect('/system/agents/attribution', self.get_attribution_string)
    Route.Connect('/system/agents/attribution/image', self.get_attribution_image)
    Route.Connect('/system/agents/media/get', self.media_get)
    Route.Connect('/system/agents/{identifier}/searchOne', self.search_one, method='POST')
    
    if Core.config.enable_external_debugging:
      Route.Connect('/system/debug/agentservice/status', self.debug_status_agentservice)
    
    Core.messaging.expose_function(self.search, '_AgentService:Search')
    Core.messaging.expose_function(self.update_agent_info, '_AgentService:UpdateInfo')
    
    # Set the _core attribute of the Media class - we don't use AgentKit in the System bundle, so
    # we need to do this here so retrieving media trees works.
    setattr(Framework.api.agentkit.Media, '_core', Core)

  def update_attribution_flags(self):
    @spawn
    def update_attribution_flags_inner():
      for identifier, info_list in self.agent_info.items():
        has_attribution = (self.get_attribution_string(identifier) is not None)
        for info in info_list:
          info['has_attribution'] = has_attribution

      Log("Agent info: %s", str(self.agent_info))
      Dict.Save()
    
  def get_combiner(self, lang):
    # If running on a PMS system, return the single combiner regardless of the language specified
    if Core.config.root_path == None:
      return self.combiner
      
    # If we already have a combiner for this language, return it
    if lang in self.combiners:
      return self.combiners[lang]
      
    # If not, check that the path exists
    combination_path = Core.storage.join_path(Core.config.root_path, 'Combination', lang)
    
    # If the path doesn't exist, call this function recursively to return the unknown language combiner.
    # Rules for Locale.Language.Unknown ('xx') should always exist on the node.
    if not Core.storage.dir_exists(combination_path):
      Log.Warn("No combination rules found for language '%s'", lang)
      return self.get_combiner(Locale.Language.Unknown)
    
    # Otherwise, if the rule path exists, create a combiner and return it
    Log.Info("Loading combination rules for language '%s'", lang)
    combiner = Framework.modelling.BundleCombiner(
      Core,
      Core.storage.join_path(Core.config.root_path, 'Metadata'),
      combination_path
    )
    self.combiners[lang] = combiner
    return combiner
    
    
  def ensure_agent_info_exists(self, identifier):
    @spawn
    def ensure_agent_info_exists_inner():
      @lock(self.agent_info)
      def info_lock():
        if identifier not in self.agent_info:
          Log.Info("Agents in %s not in info dictionary - pinging!", identifier)
          try: HTTP.Request("http://127.0.0.1:32400/:/plugins/%s" % identifier, immediate=True, cacheTime=0)
          except: Log.Warn("We weren't able to get information from the server about the agents.")
    
  def update_agent_info(self, identifier, agent_info):
    @lock(self.agent_info)
    def info_lock():
      try:
        Log("Receiving agent info from %s: %s", identifier, str(agent_info))
        for info in agent_info:
          has_attribution = (self.get_attribution_string(identifier) is not None)
          info['has_attribution'] = has_attribution
        self.agent_info[identifier] = agent_info
        Dict[AGENT_INFO_KEY] = self.agent_info
      except:
        Core.log_except(None, "Exception updating agent info from %s" % identifier)
        
  def remove_unavailable_agents(self):
    """
      Check for registered agents that have been removed
    """
    self.system.bundleservice.update_bundles(remove_unavailable_agents=False)
    bundles = self.system.bundleservice.bundles
    @lock(self.agent_info)
    def info_lock():
      for identifier in self.agent_info.keys():
        if identifier not in bundles:
          Log.Info("Removing unavailable agents for '%s'", identifier)
          del self.agent_info[identifier]
      Dict[AGENT_INFO_KEY] = self.agent_info
    
  def search(self, identifier, mediaType, lang, manual=False, **kwargs):
    """
      Performs a search in the given agent and generates GUIDs for the results.
    """
    try:
      self.searching = True
      manual = (manual == '1' or manual == True)
      
      # Convert the mediaType from PMS into a class name
      if int(mediaType) not in self.media_types: return SearchResponse(**AgentResponse.InvalidMediaType)
      media_type = self.media_types[int(mediaType)]

      version = self.agent_get_version(identifier, 'Artist' if media_type == 'Album' else media_type)
      if version > 0:
        # Don't allow non-manual searches for modern agents.
        if not manual:
          return None

        # For modern album searches, we need to load from artist bundles.
        if media_type == 'Album':
          media_type = 'Artist'

      @lock(self.agent_info)
      def info_lock():
        # Check that the plug-in is registered and handles the given type & language
        if identifier not in Core.messaging.plugin_list(): return SearchResponse(**AgentResponse.AgentNotFound)
        if not self.agent_handles_type(identifier, media_type): return SearchResponse(**AgentResponse.MediaTypeNotHandled)
        if lang not in self.agent_list_languages(identifier, media_type): return SearchResponse(**AgentResponse.LanguageNotHandled)

      # Get a list of results from the agent
      Log.Info('Searching for %s in %s (%s)', media_type, identifier, str(kwargs))
      result = self.agent_search(identifier, media_type, lang, manual, kwargs, version, primary=True)
      
      # Convert each item's ID into a GUID for legacy agents.
      if version == 0 and MediaContainer.is_instance(result):
        
        # If the agent returned no results (or score too low), try to hit the fallback agent
        if len(result) == 0 or result[0].score < 85:
          fallback_identifier = self.agent_get_fallback_identifier(identifier, media_type)
          
          # If a fallback identifier is available, hit the search function again using the fallback identifier & the provided args
          if fallback_identifier:
            return self.search(fallback_identifier, mediaType, lang, manual, **kwargs)
        
        for item in result:
          if not hasattr(item, 'guid'):
            id = item.id
            item.guid = identifier + '://' + item.id + '?lang=' + item.lang
            item.id = None
  
      return result
    except:
      return SearchResponse(details=Plugin.Traceback(), **AgentResponse.UnhandledException)
    finally:
      self.searching = False

  def search_one(self, identifier, mediaType, lang):
    # Override album types - should be artist for modern agents.
    if int(mediaType) == 9: mediaType = 8

    try:
      self.searching = True

      # Convert the mediaType from PMS into a class name.
      if int(mediaType) not in self.media_types: return SearchResponse(**AgentResponse.InvalidMediaType)
      media_type = self.media_types[int(mediaType)]

      @lock(self.agent_info)
      def info_lock():
        # Check that the plug-in is registered and handles the given type & language
        if identifier not in Core.messaging.plugin_list(): return SearchResponse(**AgentResponse.AgentNotFound)
        if not self.agent_handles_type(identifier, media_type): return SearchResponse(**AgentResponse.MediaTypeNotHandled)
        if lang not in self.agent_list_languages(identifier, media_type): return SearchResponse(**AgentResponse.LanguageNotHandled)

      # Grab the POST data.
      data = Request.Body

      # Get a list of results from the agent
      result = self.agent_search_one(identifier, media_type, lang, data)

      return result

    except:
      return SearchResponse(details=Plugin.Traceback(), **AgentResponse.UnhandledException)
    finally:
      self.searching = False

      
  def update(self, mediaType, guid, id=None, parentGUID=None, force=False, agent=None, async=True, parentID=None, libraryAgent=None, periodic=False, respectTags=False):
    """
      Matches a GUID with an agent and queues an update task.
    """
    try:
      dbid = id
      force = (force == '1' or force == True)
      async = (async == '1' or async == True)
      periodic = (periodic == '1' or periodic == True)
      respect_tags = (respectTags == '1' or respectTags == True)
      task_kwargs = None
      
      # Parse the GUID.
      (root_type, identifier, id, lang, base_guid) = self.parse_guid(int(mediaType), guid)
      
      # Convert the mediaType from PMS into a class name
      if int(root_type) not in self.media_types: return InitiateUpdateResponse(**AgentResponse.InvalidMediaType)
      media_type = self.media_types[int(root_type)]

      version = self.agent_get_version(libraryAgent or identifier, 'Artist' if media_type == 'Album' else media_type)
      if version > 0 and media_type == 'Album':
        # For modern album searches, we need to load from artist bundles.
        media_type = 'Artist'

      self.agent_lock.acquire()
      try:
        # Check that the plug-in is registered and handles the given type & language
        if identifier not in Core.messaging.plugin_list(): return InitiateUpdateResponse(**AgentResponse.AgentNotFound)
        if not self.agent_handles_type(identifier, media_type): return InitiateUpdateResponse(**AgentResponse.MediaTypeNotHandled)
        if lang not in self.agent_list_languages(identifier, media_type): return InitiateUpdateResponse(**AgentResponse.LanguageNotHandled)
      finally:
        self.agent_lock.release()
        
      self.update_lock.acquire()
      try:
        # Compare the database IDs of each item in the pending update list, and only add this record if it isn't found
        this_record = (mediaType, guid, dbid, parentGUID)
        record_found = False
        for record in self.pending_updates:
          if record[2] == dbid:
            record_found = True
            break
            
        if not record_found:
          Log.Debug("Adding %s to the update list", guid)
          self.pending_updates.append(this_record)
          
          # Build the arguments dictionary
          task_kwargs = dict(
            mediaType = mediaType,
            media_type = media_type,
            identifier = identifier,
            guid = guid,
            force = force,
            id = id,
            lang = lang,
            dbid = dbid,
            parentGUID = parentGUID,
            agent = agent,
            async = async,
            parentID = parentID,
            libraryAgent = libraryAgent,
            periodic = periodic,
            respect_tags = respect_tags
          )
          
          # If operating asynchronously, add a task to the queue. Synchronous operation is handled
          # below, after the lock has been released.
          if async:
            self.queue.add_task(
              self.update_task,
              kwargs = task_kwargs,
            )
          
        else:
          Log.Debug("Not adding %s to the update list because it already exists", guid)
          
      finally:
        self.update_lock.release()
        
      # If not operating asynchronously, call the update task function now
      if task_kwargs != None and not async:
        self.update_task(**task_kwargs)
      
      return InitiateUpdateResponse(path=self.get_relpath(media_type, identifier, guid), force=force, **AgentResponse.OK)
    
    except:
      Core.log_except(None, "Exception in update request handler")
      return InitiateUpdateResponse(details=Plugin.Traceback(), **AgentResponse.UnhandledException)

  def get_relpath(self, media_type, identifier, guid):
    # Create an access point to the primary agent's class
    primary_access_point = self.accessor.get_access_point(identifier, read_only=True)
    primary_metadata_cls = getattr(primary_access_point, self.agent_model_name(identifier, media_type))
    try:
      relpath_function = getattr(primary_metadata_cls, '_make_relpath')
      return relpath_function(guid)
    except:
      return None
      
      
  def update_task(self, mediaType, media_type, identifier, guid, id, lang, dbid, parentGUID, force, agent, async, parentID, libraryAgent=None, periodic=False, respect_tags=False):
    """
      Updates metadata for a given guid in the primary agent and any contributing agents.
    """
        
    # Create or create a lock for the GUID.
    self.guid_mutexes_lock.acquire()
    if guid not in self.guid_mutexes:
      self.guid_mutexes[guid] = [Thread.Lock(), 1]
    else:
      self.guid_mutexes[guid][1] = self.guid_mutexes[guid][1] + 1

    guid_mutex = self.guid_mutexes[guid][0]
    guid_count = self.guid_mutexes[guid][1]
    self.guid_mutexes_lock.release()
    
    # Acquire it.
    Log.Debug("Acquiring GUID mutex for %s (%s) (count is now %d)", guid, dbid, guid_count)
    guid_mutex.acquire()
    Log.Debug("Acquired GUID mutex for %s (%s)", guid, dbid)

    Log.Info('Preparing metadata for %s in %s (%s)', media_type, identifier, id)

    @spawn
    def notify_processing():
      try:
        url = 'http://127.0.0.1:32400/:/metadata/processing?id=%s' % (dbid)
        HTTP.Request(url, cacheTime=0, immediate=True)
      except:
        Log.Exception('Exception notifying the media server')

    self.processing_updates[dbid] = "Starting update task"
    try:
      try:
        full_guid = guid
        
        # Get the required combiner (see above).
        combiner = self.get_combiner(lang)
        
        # Parse the base GUID and set status.
        self.processing_updates[dbid] = "Updating metadata from the primary agent"

        version = self.agent_get_version(libraryAgent or identifier, media_type)

        # Download metadata in the primary agent
        if agent in (None, identifier):
          self.agent_update_metadata(identifier, media_type, guid, id, lang, dbid, parentGUID, force, version, parentID, periodic)
    
        # Create an access point to the primary agent's data
        self.processing_updates[dbid] = "Loading the primary agent's metadata"
        primary_access_point = self.accessor.get_access_point(identifier, read_only=True)
        primary_metadata_cls = getattr(primary_access_point, self.agent_model_name(identifier, media_type, libraryAgent))
        primary_metadata = primary_metadata_cls[guid]
    
        # Copy primary metadata attributes & use them as search args
        self.processing_updates[dbid] = "Creating hints from primary metadata"
        kwargs = dict()
        kwargs['primary_agent'] = identifier
        kwargs['guid'] = guid
        kwargs['force'] = force
        kwargs['id'] = dbid
        kwargs['parentID'] = parentID
    
        if hasattr(primary_metadata, 'name') and primary_metadata.name != None:
          kwargs['name'] = primary_metadata.name
        if hasattr(primary_metadata, 'first_aired') and primary_metadata.first_aired != None:
          kwargs['year'] = primary_metadata.first_aired.year
      
        # Download metadata in contributing agents
        tasks = []

        contributing_agents = combiner.contributing_agents(primary_metadata_cls, identifier)

        # Special handling for the Local Media agent. Since this agent can't be configured as a primary agent
        # in the UI, populate the contributing agent list here instead.
        #
        if len(contributing_agents) == 0 and identifier == "com.plexapp.agents.localmedia" and int(mediaType) in self.media_types:
          contributing_agents = [agent_object.identifier for agent_object in self.list_contributors(mediaType, identifier, primary_only=False)]

        for agent_identifier in contributing_agents:
          try:
            @locks(self.agent_info)
            def check_agent():
              self.processing_updates[dbid] = "Checking agent %s" % agent_identifier
              if agent in (None, agent_identifier) and agent_identifier in Core.messaging.plugin_list():
                if self.agent_handles_type(agent_identifier, media_type, primary_only=False):
                  return True
              return False
            
            if check_agent():
              update = False
              erase = False
              agent_id = None
              try:
                # If a contribution from this agent already exists, update it in-place
                if agent_identifier in primary_metadata.contributors and not force:
                  update = True
                  agent_id = None

                # Otherwise, attempt to search for a match
                else:
                  self.processing_updates[dbid] = "Searching for matches in %s" % agent_identifier
                  results = self.agent_search(agent_identifier, media_type, lang, False, kwargs, version, primary=False)
                  agent_id = None

                  # If the top-scoring item was over 75, assume it's a good match and update using the ID
                  score_threshold = 75

                  # Check whether our results are in a MediaContainer or a SearchResult tree.
                  if version == 0 and MediaContainer.is_instance(results) and len(results) > 0 and results[0] > score_threshold:
                    agent_id = results[0].id
                  else:
                    try:
                      el = XML.ElementFromString(results)
                      item_els = el.xpath('//SearchResult')
                      if len(item_els) > 0 and int(item_els[0].get('score')) > score_threshold:
                        agent_id = item_els[0].get('id')
                    except:
                      Log("Error parsing search results.")

                  # Modern search results...
                  if version > 0:
                    el = XML.ElementFromString(results)
                    item_els = el.xpath('//SearchResult')
                    if len(item_els) > 0 and int(item_els[0].get('score')) > score_threshold:
                      agent_id = item_els[0].get('id')

                  if agent_id:
                    Log("Found match with ID %s", str(agent_id))
                    update = True
                  else:
                    Log("No match found")
                    erase = True

              except:
                Log.Exception("Exception performing search in %s", agent_identifier)
                agent_id = None
                update = False

                    
              if update and (self.agent_supports_version(agent_identifier, media_type, version)):
                self.processing_updates[dbid] = "Updating in %s" % agent_identifier
   
                task = Core.runtime.taskpool.add_task(
                  f = self.agent_update_metadata,
                  args = [
                    agent_identifier,
                    media_type,
                    guid,
                    agent_id,
                    lang,
                    dbid,
                    parentGUID,
                    force,
                    version,
                    parentID,
                    periodic
                  ]
                )
                tasks.append(task)
              
              elif erase:
                task = Core.runtime.taskpool.add_task(
                  f = self.agent_erase_metadata,
                  args = [
                    agent_identifier,
                    media_type,
                    guid
                  ]
                )
                tasks.append(task)
                
              Core.runtime.taskpool.wait_for_tasks(tasks)

          except Exception, e:
            Log("Exception loading contributions from agent %s, skipping" % agent_identifier)
            continue
            
        # Combine the metadata according to the combination rules      
        self.processing_updates[dbid] = "Combining"

        model_name = self.agent_model_name(identifier, media_type, libraryAgent)
        metadata_class = getattr(self.access_point, model_name)
        if version > 1 and parentGUID:
          bundle_guid = parentGUID
        else:
          bundle_guid = full_guid

        xml = combiner.combine(metadata_class, libraryAgent or identifier, bundle_guid, self.agent_info, respect_tags)
        
        # Combine subtitles for video types.
        if int(mediaType) in (1, 2, 3, 4):
          if dbid != None:
            # FIXME, should this be bundle_guid too?
            self.combine_subtitles(combiner, metadata_class, media_type, identifier, full_guid, dbid)
          else:
            Log.Error("No database ID given for '%s' - not combining subtitles", full_guid)
      
        success = True
        Log.Info("Metadata preparation for %s complete" % guid)
      
      except:
        bundle_guid = full_guid
        success = False
        xml = None
        Core.log_except(None, "Exception in update for %s", guid)
      
      self.processing_updates[dbid] = "Notifying the media server"
      
      # Notify the media server
      notify_kwargs = dict(
        mediaType = mediaType,
        media_type = media_type,
        identifier = identifier,
        guid = bundle_guid,
        force = force,
        success = success,
        async = async,
        dbid = dbid,
        xml = xml
      )
      
      if async:
        Thread.Create(self.notify_thread, **notify_kwargs)
      else:
        self.notify_thread(**notify_kwargs)
      
      
      self.update_lock.acquire()
      try:
        # Try to find a record in the pending updates list with a matching GUID - if found, remove it
        record_to_remove = None
        for record in self.pending_updates:
          if record[2] == dbid:
            record_to_remove = record
            break
            
        if record_to_remove != None:
          Log.Debug("Removing %s from the update list", full_guid)
          self.pending_updates.remove(record_to_remove)
        else:
          Log.Debug("Not removing %s from the update list because it doesn't exist", full_guid)
            
      finally:
        self.update_lock.release()
      
      self.processing_updates[dbid] = "Done"
    finally:
      del self.processing_updates[dbid]

      # Finally, release GUID mutex.
      Log.Debug("Releasing GUID mutex for %s", guid)
      guid_mutex.release()

      self.guid_mutexes_lock.acquire()
      self.guid_mutexes[guid][1] = self.guid_mutexes[guid][1] - 1
      if self.guid_mutexes[guid][1] == 0:
        Log.Debug("We're done with the mutex for %s", guid)
        del self.guid_mutexes[guid]
      self.guid_mutexes_lock.release()
      
  def combine_subtitles(self, combiner, cls, media_type, identifier, full_guid, dbid):
    def dlog(*args):
      Log.Debug(*args)
      
    # Get the media class for this type
    media_cls = getattr(Framework.api.agentkit.Media, '_class_named')(media_type)
    
    # Get all parts from the media tree for this dbid
    tree = Framework.api.agentkit.Media.TreeForDatabaseID(dbid, getattr(media_cls, '_level_names'))
    parts = tree.all_parts()
    
    # Get the combiner config for the given class & identifier and extract the agent order
    config_el = combiner.get_config_el(cls, identifier)
    agent_order = []
    for agent_el in config_el.xpath('//agent'):
      agent_order.append(str(agent_el.text))
      
    dlog(">>> Agent order: %s", str(agent_order))
    
    # Iterate over each part
    for part in parts:
      # Get the bundle's path
      try:
        path = getattr(part, '_path')
        
        # If we have no path, don't try to combine
        if path == None:
          continue
          
        Log.Debug("Combining subtitles for media bundle at '%s'", path)
      
        # Compute the paths for this media bundle
        contrib_path = Core.storage.join_path(path, 'Contents', 'Subtitle Contributions')
        final_path = Core.storage.join_path(path, 'Contents', 'Subtitles')
        final_xml_path = final_path + '.xml'
      
        # Whack existing combined data
        Core.storage.remove_tree(final_path)
        Core.storage.ensure_dirs(final_path)
        if Core.storage.file_exists(final_xml_path):
          Core.storage.remove(final_xml_path)
      
        # Create a dict to hold the results
        combined_dict = {}
      
        # List each item in the subtitle contribution path
        for agent_identifier in agent_order:
        
          # Compute the XML file path for this agent, and load it if it exists
          agent_xml_path = Core.storage.join_path(contrib_path, agent_identifier + '.xml')
          if Core.storage.file_exists(agent_xml_path):
            dlog(">>> Checking file at path %s", agent_xml_path)
            xml_data = Core.storage.load(agent_xml_path)
            el = XML.ElementFromString(xml_data)
          
            # Iterate over each language in the XML file
            for lang_el in el.xpath('Language'):
              code = lang_el.get('code')
              dlog(">>> Processing language code '%s'", code)
              
              # Make sure the final path for this langauge exists
              Core.storage.ensure_dirs(Core.storage.join_path(final_path, code))
            
              # Get the list of subs for this language, creating one if necessary
              if code not in combined_dict:
                combined_dict[code] = []
              subtitle_list = combined_dict[code]
            
              # Append the attributes for each subtitle
              for subtitle_el in lang_el.xpath('Subtitle'):
                attrs = dict(subtitle_el.attrib)
              
                # For stored media subs, symlink them to the combined location
                if 'media' in attrs:
                  original_filename = attrs['media']
                  original_path = Core.storage.join_path(contrib_path, agent_identifier, code, original_filename)
                  combined_filename = agent_identifier + '_' + original_filename
                  combined_path = Core.storage.join_path(final_path, code, combined_filename)
                  dlog(">>> Symlinking %s to %s", original_path, combined_path)
                  Core.storage.symlink(original_path, combined_path)
                  attrs['media'] = combined_filename
                
                subtitle_list.append(attrs)
              dlog(">>> List after processing: %s", str(subtitle_list))
              
          else:
            dlog(">>> No file found at path %s", agent_xml_path)
            
        # Create a new XML document for the combined subtitles        
        dlog(">>> Creating combined XML file from %s", str(combined_dict))
        combined_el = XML.Element('Subtitles')
      
        # Add an element for each language
        for code in combined_dict:
          dlog(">>> Adding elements for '%s'", code)
          lang_el = XML.Element('Language', code=code)
        
          # Add each subtitle from the list for this language
          for attrs in combined_dict[code]:
            subtitle_el = XML.Element('Subtitle')
            [subtitle_el.set(name, attrs[name]) for name in attrs]
            lang_el.append(subtitle_el)
          
          # Append the language element to the combined element
          combined_el.append(lang_el)
      
        # Write the combined XML to a file
        combined_xml_data = XML.StringFromElement(combined_el)
        dlog("Combined XML data: %s", combined_xml_data)
        Core.storage.save(final_xml_path, combined_xml_data)
      
      except:
        Log.Error("Exception combining subtitles")
     
  def notify_thread(self, mediaType, media_type, identifier, guid, force, success, async, dbid, xml):
    """
      Notifies the media server when a bundle gets updated.
    """
    
    Log.Info("Bundle with guid %s has been updated - notifying the media server" % guid)
    
    # Get the relative path, if available
    relpath = self.get_relpath(media_type, identifier, guid)
    if relpath != None:
      path_arg = '&path=' + String.URLEncode(relpath)
    else:
      path_arg = ''
    
    queueSize = max(len(self.pending_updates) - 1, 0)
    try:
      url = 'http://127.0.0.1:32400/:/metadata/notify?guid=%s%s&force=%d&queueSize=%d&id=%s&success=' % (String.URLEncode(guid), path_arg, int(force), queueSize, dbid)
      if success:
        url += '1'
      else:
        url += '0'
      url += '&async=%d' % (1 if async else 0)
      HTTP.Request(url, cacheTime=0, immediate=True, data=xml)
    except:
      Log.Exception('Exception notifying the media server')
      
  def list(self, mediaType=None):
    """
      Returns a list of agents that handle the specified media type.
    """
    self.remove_unavailable_agents()
    
    # Convert the mediaType from PMS into a class name
    if mediaType == None:
      media_type = None
    else:
      if int(mediaType) not in self.media_types: return
      media_type = self.media_types[int(mediaType)]
    
    d = MediaContainer()
    
    @lock(self.agent_info)
    def info_lock(media_type=media_type):
      
      if mediaType is None:
        
        # List of all agents.
        for identifier in self.agent_info:
          con = XMLContainer(identifier=identifier)
          con.tagName = 'Agent'
          con.hasPrefs = self.agent_has_prefs(identifier)
          con.hasAttribution = self.agent_has_attribution(identifier)
          primary = False
          
          for media_type in self.media_types.keys():
            type = self.media_types[media_type]
            name = self.agent_get_name(identifier, type)
            
            if self.agent_handles_type(identifier, type, primary_only=True):
              primary = True
              obj = XMLContainer(mediaType=media_type, name=name)
              obj.tagName = 'MediaType'
              
              for lang in self.agent_list_languages(identifier, type):
                l = XMLObject(code=lang)
                l.tagName = 'Language'
                obj.Append(l)  
              
              con.Append(obj)

          con.primary = primary
          d.Append(con)
        
      else:
        had_null_agent = False
      
        for identifier in self.agent_info:
          if self.agent_handles_type(identifier, media_type, primary_only=True):
            name = self.agent_get_name(identifier, media_type)
            con = XMLContainer(identifier=identifier, name=name)
            con.tagName = 'Agent'
            con.hasPrefs = self.agent_has_prefs(identifier)

            for lang in self.agent_list_languages(identifier, media_type):
              obj = XMLObject(code=lang)
              obj.tagName = 'Language'
              con.Append(obj)
          
            d.Append(con)
          
            # See if the null agent is an actual real agent.
            if identifier == 'com.plexapp.agents.none':
              had_null_agent = True
          
        if media_type in self.dummy_agents and not had_null_agent:
          con = XMLContainer(identifier='com.plexapp.agents.none', name=self.dummy_agents[media_type])
          con.tagName = 'Agent'
          con.hasPrefs = False
          obj = XMLObject(code=Locale.Language.English)
          obj.tagName = 'Language'
          con.Append(obj)
          d.Append(con)

    return d
    
  def list_contributors(self, mediaType, primaryAgent, primary_only=True):
    """
      Returns a list of contributing agents for a primary agent with the specified media type and language.
    """
    # Convert the mediaType from PMS into a class name
    if int(mediaType) not in self.media_types: return
    media_type = self.media_types[int(mediaType)]
    
    d = MediaContainer()
    
    @lock(self.agent_info)
    def info_lock():
      if primaryAgent in self.agent_info and self.agent_handles_type(primaryAgent, media_type, primary_only=primary_only):
        for identifier in self.agent_info:
          if identifier != primaryAgent and (self.agent_contributes_to(identifier, media_type, primaryAgent) or self.agent_accepts_from(primaryAgent, media_type, identifier)):
            name = self.agent_get_name(identifier, media_type)
            con = XMLObject(identifier=identifier, name=name)
            con.tagName = 'Agent'
            con.hasPrefs = self.agent_has_prefs(identifier)
            d.Append(con)

    return d
    
  def get_agent_configuration(self, mediaType, identifier, lang=Locale.Language.Unknown):
    """
      Returns a list of all agents for a given primary agent and type, and the order in which they're called by the combiner
    """
    self.remove_unavailable_agents()

    # Convert the mediaType from PMS into a class name
    if int(mediaType) not in self.media_types: return
    media_type = self.media_types[int(mediaType)]
    
    all_agents = {}
    
    @lock(self.agent_info)
    def info_lock():
      if identifier in self.agent_info and self.agent_handles_type(identifier, media_type, primary_only=True):
        for contrib_identifier in self.agent_info:
          if self.agent_handles_type(contrib_identifier, media_type):
            name = self.agent_get_name(contrib_identifier, media_type)
            all_agents[contrib_identifier] = name
    
    # Create an access point to the primary agent's class
    primary_access_point = self.accessor.get_access_point(identifier, read_only=True)
    primary_metadata_cls = getattr(primary_access_point, self.agent_model_name(identifier, media_type))
    
    # Get the required combiner (see above) and retrieve the list of contributing agents from it
    combiner = self.get_combiner(lang)
    contributing_agents = combiner.contributing_agents(primary_metadata_cls, identifier, include_config_identifier=True)
    
    # Make sure the primary agent is enabled.
    if identifier not in contributing_agents:
      contributing_agents.append(identifier)
      
    c = MediaContainer()
    
    def XMLObjectForIdentifier(ident, **kwargs):
      agent = XMLObject(identifier = ident, name = all_agents[ident], **kwargs)
      agent.tagName = 'Agent'
      agent.hasPrefs = self.agent_has_prefs(ident)
      return agent
      
    for ident in contributing_agents:
      if ident in all_agents:
        c.Append(XMLObjectForIdentifier(ident, enabled = True))
        del all_agents[ident]
      
    for ident in sorted(all_agents.keys()):
      c.Append(XMLObjectForIdentifier(ident))
    
    return c
    
  def set_agent_configuration(self, mediaType, identifier, order):
    # Convert the mediaType from PMS into a class name
    if int(mediaType) not in self.media_types: return
    media_type = self.media_types[int(mediaType)]
    
    agent_list = order.split(',')
    
    el = XML.Element('combine')
    el.set('class', media_type)
    
    sources_el = XML.Element('sources')
    el.append(sources_el)
    
    for ident in agent_list:
      sources_el.append(XML.Element('agent', ident))
      
    path = Core.storage.join_path(Core.plugin_support_path, 'Metadata Combination', identifier)
    xml = XML.StringFromElement(el)
    
    Core.storage.make_dirs(path)
    Core.storage.save(Core.storage.join_path(path, String.Pluralize(media_type.replace('_', ' '))+'.xml'), xml)
      
    return el
          
  
  def get_attribution_string(self, identifier):
    # Get the list of bundles
    bundles = self.system.bundleservice.bundles

    # If the identifier couldn't be found, return immediately
    if identifier not in bundles:
      return

    # Load the plist  
    path = Core.storage.join_path(bundles[identifier].path, "Contents", "Info.plist")
    plist = Plist.ObjectFromString(Core.storage.load(path))

    # If the key is in the plist, return the string
    if ATTRIBUTION_PLIST_KEY in plist:
      return plist[ATTRIBUTION_PLIST_KEY]
      
  def get_attribution_image(self, identifier):
    # Get the list of bundles
    bundles = self.system.bundleservice.bundles

    # If the identifier couldn't be found, return immediately
    if identifier not in bundles:
      return

    # If the image exists, load and return it  
    path = Core.storage.join_path(bundles[identifier].path, "Contents", "Resources", "attribution.png")
    if Core.storage.file_exists(path):
      return DataObject(Core.storage.load(path, binary=True), "image/png")
      
  
  """ Media storage """
  
  def media_get(self, mediaType, guid, url, parentGUID=None):
    # Check the URL scheme
    if not url.startswith('metadata://'):
      Log.Error("The URL '%s' contains an unsupported scheme", url)
      return
      
    # Split the URL into parts and check for an even number
    parts = url[11:].split('/')
    if len(parts) % 2 != 0:
      Log.Error("Malformed path: '%s'", url)
      return

    # Special case for handling albums as children of modern artists.
    if parentGUID and int(mediaType) == 9:
        mediaType = 8
        parts.insert(0, 'albums')
        parts.insert(1, Hash.SHA1(guid))
        guid = parentGUID

    # Get the identifier from the GUID.
    (root_type, identifier, id, lang, base_guid) = self.parse_guid(mediaType, guid)
      
    # Get the media type
    if int(root_type) not in self.media_types: return
    media_type = self.media_types[root_type]
    model_name = self.agent_model_name(identifier, media_type)

    # Create an access point to the primary agent's class
    primary_access_point = self.accessor.get_access_point(identifier, read_only=True)
    primary_metadata_cls = getattr(primary_access_point, model_name)

    # Get the storage path for the bundle
    path_function = getattr(primary_metadata_cls, '_make_path')
    storage_path = path_function(base_guid)

    # Compute which XML file we should load
    if len(parts) == 2:
      info_path = Core.storage.join_path(storage_path, '_combined', 'Info.xml')
    else:
      info_path = Core.storage.join_path(storage_path, '_combined', *parts[:-2]) + '.xml'
      
    attr_name = parts[-2]
    attr_value = parts[-1]

    source_path = Core.storage.join_path('_combined', *parts)

    # Make sure to point to the correct source when symlinks are disabled.
    if len(os.environ.get('DISABLESYMLINKS', '')) > 0:
      # Our folder parts look like: posters/com.plexapp.agents.lastfm_d988b208b545712057b3c035cb7ecbb44d42830c
      if len(parts) == 2 and '_' in parts[1]:
        # Build the original source path for the media.
        folder_agent_and_hash = parts[1].split('_')
        sub_path = Core.storage.join_path(parts[0], folder_agent_and_hash[1])
        source_path = Core.storage.join_path(folder_agent_and_hash[0], sub_path)

    # Calculate other paths
    attr_file_path = Core.storage.join_path(storage_path, source_path)
    dl_file_path = Core.storage.join_path(storage_path, '_stored', *parts)
    Core.storage.make_dirs(Core.storage.dir_name(dl_file_path))
    
    # If the stored file already exists, return it
    if Core.storage.file_exists(dl_file_path):
      Log.Debug("Returning stored data for %s (in %s)", url, identifier)
      return Core.storage.load(dl_file_path)
    
    # Otherwise, load the info file
    info_data = Core.storage.load(info_path)
    info_xml = XML.ElementFromString(info_data)
    
    # Find the attribute in the info file
    media_xp = "//%s/item[@media='%s']" % (attr_name, attr_value)
    media_elements = info_xml.xpath(media_xp)
    
    preview_xp = "//%s/item[@preview='%s']" % (attr_name, attr_value)
    preview_elements = info_xml.xpath(preview_xp)
    
    # If a media file exists, copy it into the _stored directory and return it directly
    if len(media_elements) > 0:
      Log.Debug("Reading existing data for %s (in %s)", url, identifier)
      data = Core.storage.load(attr_file_path)
      
    # If a preview file exists, retrieve the original media file, store it and return it
    elif len(preview_elements) > 0:
      media_url = preview_elements[0].get('url')
      Log.Debug("Downloading data for %s (in %s) from %s", url, identifier, media_url)
      data = HTTP.Request(media_url, cacheTime=0).content
      
    else:
      Log.Error("No URL found in info file for '%s'", url)
      return

    if data != None:
      # Store the data
      Log.Debug("Writing data for %s (in %s)", url, identifier)
      Core.storage.save(dl_file_path, data)
    
      # Recombine the bundle
      Log.Debug("Recombining the metadata bundle for %s (%s)", guid, model_name)
      combiner = self.get_combiner(lang)
      combiner.combine(getattr(self.access_point, model_name), identifier, guid)
      
    else:
      Log.Error("Unable to store data for %s (in %s) - nothing returned", url, identifier)
    
    # Return the data
    return data
  
  def parse_guid(self, media_type, guid):
    """
      Parses a GUID and extracts the various components.
    """
    root_type = int(media_type)
    paths_to_strip = 0
    lang = None
    
    if root_type == 3: # Season
      root_type = 2
      paths_to_strip = 1
    elif root_type == 4: # Episode
      root_type = 2
      paths_to_strip = 2
    elif root_type == 10: # Track
      root_type = 9
      paths_to_strip = 1

    # Split it up.
    (identifier, parts) = guid.split('://')
    base_guid = identifier + '://'
    
    # Extract language from querystring.
    lang = Locale.Language.Unknown
    args = ''
    if parts.find('?') != -1:
      try:
        (parts, args) = parts.split('?')
        qargs = cgi.parse_qs(args)
        if 'lang' in qargs:
          lang = qargs['lang'][0]
        args = '?' + args
      
      except:
        Log.Exception("Error parsing querystring.")
        
    # Figure out the root ID.
    try:
      part_list = parts.split('/')
      id = '/'.join(part_list[0:len(part_list)-paths_to_strip])
    except:
      Log.Exception("Problem parsing GUID '%s' of type '%d'", guid, int(media_type))
      id = parts
    
    base_guid = identifier + '://' + id + args
    
    # Set the identifier and language for local:// GUIDs
    if identifier == 'local':
      identifier = 'com.plexapp.agents.localmedia'
      lang = 'xn'

    # New-fangled, need to map GUID prefix (plex://) to agent identifier.
    if identifier == 'plex' and root_type in (8, 9):
      identifier = 'tv.plex.agents.music'

    # New-fangled, need to map MBID prefixes to agent identifer.
    if identifier == 'mbid' and root_type in (8, 9):
      identifier = 'org.musicbrainz.agents.music'

    return (root_type, identifier, id, lang, base_guid)

  def agent_model_name(self, identifier, media_type, library_agent=None):
    # For artists, make sure we select the correct model type (modern or legacy).
    if media_type == 'Artist':
      return 'ModernArtist' if self.agent_get_version(library_agent or identifier, media_type) >= 2 else 'LegacyArtist'
    elif media_type == 'Album':
        return None if self.agent_get_version(library_agent or identifier, media_type) >= 2 else 'LegacyAlbum'
    else:
      return media_type

  
  """ AgentKit function calls """
  
  def agent_has_prefs(self, identifier):
    for agent in self.agent_info[identifier]:
      if 'prefs' in agent and agent['prefs'] == True:
        return True
    return False

  def agent_has_attribution(self, identifier):
    info_list = self.agent_info.get(identifier, [])
    for info in info_list:
      if info.get('has_attribution') == True:
        return True

    return False
  
  def agent_handles_type(self, identifier, media_type, primary_only=False):
    for agent in self.agent_info[identifier]:
      if media_type in agent['media_types'] and (agent['primary_provider'] or not primary_only):
        return True
    return False
    
  def agent_contributes_to(self, identifier, media_type, primary_agent):
    for agent in self.agent_info[identifier]:
      if media_type in agent['media_types'] and ((agent['contributes_to'] == None and not agent['primary_provider']) or (agent['contributes_to'] != None and primary_agent in agent['contributes_to'])):
        return True
    return False
    
  def agent_accepts_from(self, identifier, media_type, secondary_agent):
    for agent in self.agent_info[identifier]:
      if media_type in agent['media_types']:
        if 'accepts_from' in agent:
          if agent['accepts_from'] != None:
            if secondary_agent in agent['accepts_from']:
              return True
    return False
    
  def agent_get_name(self, identifier, media_type):
    for agent in self.agent_info[identifier]:
      if media_type in agent['media_types']:
        return agent['name']

  def agent_get_fallback_identifier(self, identifier, media_type):
    for agent in self.agent_info[identifier]:
      if media_type in agent['media_types']:
        if 'fallback_agent' in agent:
          return agent['fallback_agent']

  def agent_list_languages(self, identifier, media_type):
    for agent in self.agent_info[identifier]:
      if media_type in agent['media_types']:
        return agent['languages']
    return list()

  def agent_get_version(self, identifier, media_type):
    for agent in self.agent_info[identifier]:
      if media_type in agent['media_types']:
        return agent.get('version', 0)
    return False

  def agent_supports_version(self, identifier, media_type, version):
    for agent in self.agent_info[identifier]:
      if media_type in agent['media_types']:
        if agent.get('version') == version:
          return True
    return False
    
  def agent_search(self, identifier, media_type, lang, manual, kwargs, version, primary=False):
    kwargs = dict(
      media_type = media_type,
      lang = lang,
      manual = manual,
      kwargs = kwargs,
      version = version
    )
    if Util.VersionAtLeast(Core.version, 2,5):
      kwargs['version'] = version

    if Util.VersionAtLeast(Core.version, 2,6):
      kwargs['primary'] = primary

    return Core.messaging.call_external_function(
      identifier,
      '_AgentKit:Search',
      kwargs=kwargs
    )

  def agent_search_one(self, identifier, media_type, lang, data):
    return Core.messaging.call_external_function(
      identifier,
      '_AgentKit:SearchOne',
      kwargs=dict(
        media_type=media_type,
        lang=lang,
        data=data
      )
    )
    
  def agent_update_metadata(self, identifier, media_type, guid, id, lang, dbid=None, parentGUID=None, force=False, version=0, parentID=None, periodic=False):
    kwargs = dict(
      media_type = media_type,
      guid = guid,
      id = id,
      lang = lang,
      dbid = dbid,
      parentGUID = parentGUID,
      force = force,
      parentID = parentID
    )

    if Util.VersionAtLeast(Core.version, 2,5):
      kwargs['version'] = version

    if Util.VersionAtLeast(Core.version, 2,6):
      kwargs['periodic'] = periodic

    return Core.messaging.call_external_function(
      identifier,
      '_AgentKit:UpdateMetadata',
      kwargs=kwargs
    )
    
  def agent_erase_metadata(self, identifier, media_type, guid):
    return Core.messaging.call_external_function(
      identifier,
      '_AgentKit:EraseMetadata',
      kwargs=dict(
        media_type = media_type,
        guid = guid
      )
    )

    
  """ Debugging """
  
  def debug_status_agentservice(self):
    
    def lock_state(key):
      lock = Thread.Lock(key)
      state = lock.locked()
      if state: return "Locked"
      else: return "Unlocked"
      
    self.guid_mutexes_lock.acquire()
    guid_mutexes = len(self.guid_mutexes)
    self.guid_mutexes_lock.release()

    def format_dict(dct):
      r = "{\n"
      for k in dct:
        r += "\t%-32s :  %s,\n" % (str(k), str(dct[k]))
      r += "}"
      return r
      
    def format_list(lst):
      r = "[\n"
      for l in lst:
        r += "\t%s,\n" % str(l)
      r += "]"
      return r
      
    r  = "AgentService Status\n"
    r += "-------------------\n" 
    r += "Update queue size   : %d\n" % self.queue.size
    r += "Notify queue size   : %d\n" % self.notify_queue.size
    r += "Update mutexes:     : %d\n" % guid_mutexes
    r += "\n"
    r += "Info lock state     : %s\n" % lock_state(self.agent_info)
    r += "Update lock state   : %s\n" % lock_state(self.pending_updates)
    r += "\n"
    r += "Processing updates  : %s\n" % format_dict(self.processing_updates)
    r += "\n"
    r += "Pending updates     : %s\n" % format_list(self.pending_updates)
    
    return r

