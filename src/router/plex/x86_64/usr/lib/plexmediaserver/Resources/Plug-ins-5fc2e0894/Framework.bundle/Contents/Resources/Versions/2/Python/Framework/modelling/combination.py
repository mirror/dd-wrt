#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import os, os.path, sys
import templates
from copy import deepcopy

excluded_attr_names = ['save_externally']

class Combinable(object): pass

class BundleCombiner(object):
  
  def __init__(self, core, model_path, config_path):
    self._core = core
    self._model_path = model_path
    self._config_path = config_path
    self._linked_attributes = {
      "rating": ["rating_image", "audience_rating", "audience_rating_image"]
    }

    self._dependent_attributes = reduce(lambda x, y: x + y, self._linked_attributes.values(), [])

  def _dlog(self, msg, *args):
    if Framework.constants.flags.log_metadata_combination in self._core.flags:
      self._core.log.debug("(COMBINER) >>> %s" % msg, *args)

  def _combine_attr(self, root_path, internal_path, attr, config_identifier, config_el, candidates, p_sources, template, parts=[], cls=None, agent_info={}):
    if attr._synthetic:
      return
    
    out_path = os.path.join(root_path, '_combined', internal_path)
    
    # Apply source rules from the config
    p_sources = self._apply_sources(p_sources, config_identifier, config_el, candidates)
    name = template.__name__
    # Remove any sources that don't have an attribute candidate
    for source in p_sources:
      if source not in candidates:
        p_sources.remove(source)
    
    # Get the attribute rules
    rules = self._get_rules(config_el)
    
    # Get the rule action
    rule_action = config_el.get('action')
    
    # If no action is defined, set a default based on the template class
    if rule_action == None:
      if isinstance(attr, (templates.MapTemplate, templates.ObjectContainerTemplate)):
        rule_action = 'merge'
      else:
        rule_action = 'override'

    # If this attribute is dependent on another, don't output it directly.
    if name in self._dependent_attributes:
      return

    # Combine values
    if isinstance(attr, templates.ValueTemplate):
      #TODO: Account for external values
      if rule_action == 'override':
        for source in p_sources:
          # Treat special DEL character as blank.
          if source in candidates and candidates[source].text == '\x7f':
            candidates[source].text = ''
          if source in candidates and candidates[source].text is not None:
            els = [candidates[source]]

            # Add any linked attributes to the returned list
            if name in self._linked_attributes:
              for linked_name in self._linked_attributes[name]:
                linked_els = candidates[source].xpath('../' + linked_name)
                els.extend(linked_els)

            return els
      else:
        raise Framework.exceptions.FrameworkException("Unable to perform the action '%s' on an object of type %s" % (rule_action, type(attr)))
      
    
    # Combine records  
    elif isinstance(attr, templates.RecordTemplate):
      # TODO: Record combination
      print "TODO: Record combination"
      
      
    # Combine proxy containers
    elif isinstance(attr, templates.ProxyContainerTemplate):
      attr_el = self._core.data.xml.element(name)
      
      symlink_map = {}
      combined_names = []
      elements = {}
      
      for source in p_sources:
        if source in candidates:
          elements[source] = []
          source_dir_path = os.path.join(root_path, source, internal_path)
          if os.path.exists(source_dir_path):
            Framework.utils.makedirs(out_path)
            for item_el in candidates[source].xpath('item'):
              proxy_type = None
              
              # Find the proxy type
              for key in item_el.keys():
                if key not in ['external', 'url']:
                  proxy_type = key
                  filename = item_el.get(key)
                  break
              if not proxy_type:
                raise Framework.exceptions.FrameworkException('Proxy item %s has no type.', item_el)
              
              # Calculate the relative path for the symlink
              source_file_path = os.path.join(source_dir_path, filename)
              combined_filename = source + '_' + filename
              link_path = os.path.join(out_path, combined_filename)
              rel_path = os.path.relpath(source_file_path, out_path)
              
              combined_names.append(combined_filename)
              
              # Copy the item element
              item_copy = deepcopy(item_el)
              
              # Insert source.
              item_copy.set('provider', source)
              
              # Check whether a stored file exists
              stored_file_path = link_path.replace('_combined', '_stored')
              if os.path.exists(stored_file_path):
                # The stored file exists - modify the path & XML accordingly
                rel_path = os.path.relpath(stored_file_path, out_path)
                if proxy_type == 'preview':
                  del item_copy.attrib['preview']
                proxy_type = 'media'
              
              symlink_map[rel_path] = link_path
              
              item_copy.set(proxy_type, combined_filename)
              elements[source].append(item_copy)

        # If we are overriding, not merging, break when the symlink map becomes populated after checking a source
        if rule_action == 'override' and len(symlink_map) > 0:
          break
          
      # Check for stored media that's no longer available in agents (if required)
      stored_path = out_path.replace('_combined', '_stored')
      if self._core.storage.dir_exists(stored_path):
        for filename in self._core.storage.list_dir(stored_path):

          # Grab the identifier and get the stored file persistence attribute from the info dict.
          persist_stored_files = True
          ident = filename[:filename.rfind('_')]
          info = agent_info.get(ident, [])
          for info_dict in info:
            clsname = cls.__name__
            if clsname == 'LegacyArtist':
              clsname = 'Artist'
            if clsname == 'LegacyAlbum':
              clsname = 'Album'
            if clsname in info_dict.get('media_types', []):
              persist_stored_files = info_dict.get('persist_stored_files', True)

          if persist_stored_files and filename not in combined_names:
            source = filename.split('_')[0]
            
            # Create a synthetic element referring to the media
            synthetic_el = self._core.data.xml.element("item", media=filename, external='True')
            if source not in elements:
              elements[source] = []
            elements[source].insert(0, synthetic_el)
            
            # Compute the paths & add a symlink
            stored_file_path = self._core.storage.join_path(stored_path, filename)
            rel_path = os.path.relpath(stored_file_path, out_path)
            link_path = self._core.storage.join_path(out_path, filename)
            symlink_map[rel_path] = link_path
      
      # Add the media elements to the attribute element
      added_sources = []
      for source in p_sources:
        if source in candidates and source in elements:
          added_sources.append(source)
          for el in elements[source]:
            attr_el.append(el)
      
      # Add stored media items for agents that are currently disabled  
      for source in elements:
        if source not in added_sources:
          for el in elements[source]:
            attr_el.append(el)
          
      # Make the symlinks.
      for source in symlink_map:
        dest = symlink_map[source]
        self._core.storage.make_dirs(self._core.storage.dir_name(dest))
        self._core.storage.symlink(source, dest)

      return [attr_el]
      
      
    # Combine directories
    elif isinstance(attr, templates.DirectoryTemplate):
      symlink_map = {}
      for source in p_sources:
        source_dir_path = os.path.join(root_path, source, internal_path)
        if os.path.exists(source_dir_path):
          Framework.utils.makedirs(out_path)
          for filename in os.listdir(source_dir_path):
            # Calculate the relative path for the symlink
            source_file_path = os.path.join(source_dir_path, filename)
            link_path = os.path.join(out_path, source + '_' + filename)
            rel_path = os.path.relpath(source_file_path, out_path)
            symlink_map[rel_path] = link_path
        
        # If we are overriding, not merging, break when the symlink map becomes populated after checking a source
        if rule_action == 'override' and len(symlink_map) > 0:
          break
          
      # Make the symlinks.
      for source in symlink_map:
        self._core.storage.symlink(source, symlink_map[source])
      
      return [self._core.data.xml.element(name)]
        
    
    # Combine maps
    elif isinstance(attr, templates.MapTemplate):
      def process_key(key):
        return self._core.data.hashing.sha1(key) if cls._template.use_hashed_map_paths else key

      # If overriding, use the items from the first populated source
      if rule_action == 'override':
        item_map = {}
        for source in p_sources:
          if source in candidates:
            for item_el in candidates[source].xpath('item'):
              item_map[item_el.get('key')] = item_el
            break
        
        # Create a combined element
        attr_el = self._core.data.xml.element(name)
        for item_name in item_map:
          attr_el.append(item_map[item_name])
        
        # Return it
        return [attr_el]
      
      # TODO: Merge child attributes          
      
      elif rule_action == 'merge':

        # Track indexes for GUIDs.
        guid_indexes = {}

        # Get candidates for each item
        item_candidate_sets = {}
        for source in p_sources:
          if source in candidates:
            for item_el in candidates[source].xpath('item'):
              # Check for GUID-based items.
              key = item_el.get('key')
              guid = item_el.get('guid')

              if guid is not None:
                  guid_indexes[guid] = key
                  key = guid

              if key not in item_candidate_sets:
                item_candidate_sets[key] = {}
            
              # If the item is saved externally, discard the current element and load the contents of the XML file
              if bool(item_el.get('external')) == True:
                item_xml_path = os.path.join(root_path, source, internal_path, process_key(key) + '.xml')
                item_xml_str = self._core.storage.load(item_xml_path)
                item_el = self._core.data.xml.from_string(item_xml_str, remove_blank_text=True)
              
              item_candidate_sets[key][source] = deepcopy(item_el)
          
        # Get a config element for the current item
        # TODO: Fix?
        if name in rules:
          rule = rules[name]
        else:
          rule = self._core.data.xml.element('attribute')
        
        # Create a new element for the map attribute
        attr_el = self._core.data.xml.element(name)
        
        # Get the item template and class name
        item_template = template._item_template
        class_name = type(item_template).__name__
        setattr(item_template, '__name__', 'item')
        
        for key in item_candidate_sets:
          # If we've been passed a list for partial combination, only combine if the key matches
          if len(parts) > 0 and parts[1] != key:
            #print "Skipping attr key %s because it isn't %s" % (key, parts[1])
            continue
            
          item_candidates = item_candidate_sets[key]
          
          # Combine the item
          if isinstance(item_template, templates.RecordTemplate):
            item_els = [self._combine_object(root_path, os.path.join(internal_path, process_key(key)), config_identifier, rule, item_candidates, list(p_sources), item_template, parts=parts[2:] if len(parts) > 0 else [], cls=cls)]
          else:
            item_els = self._combine_attr(root_path, os.path.join(internal_path, process_key(key)), attr._item_template, config_identifier, rule, item_candidates, list(p_sources), item_template, parts=parts[2:] if len(parts) > 0 else [], cls=cls)
        
          # If combined successfully, process the item
          if item_els is not None:

            path = os.path.join(internal_path, process_key(key)) if cls._template.use_hashed_map_paths else None

            # Make sure we set separate key & guid attributes if this item is GUID-based.
            def set_item_attributes(item_el):
                if key not in guid_indexes:
                    if key.find('://') != -1:
                      item_el.set('guid', key)
                    else:
                      item_el.set('key', key)
                else:
                    item_el.set('key', guid_indexes[key])
                    item_el.set('guid', key)

                if path:
                  item_el.set('path', path)

            for item_el in item_els:
              # If the map is stored internally, append the element
              if not attr._item_template._external:
                set_item_attributes(item_el)
                attr_el.append(item_el)

              # If stored externally, write the element to a file and append a stub
              else:
                item_out_path = os.path.join(out_path, process_key(key))
                Framework.utils.makedirs(out_path)
                item_el.tag = class_name

                # Only serialise if we have no more parts
                if len(parts) <= 2:
                  item_xml_str = self._core.data.xml.to_string(item_el)
                  self._core.storage.save(item_out_path + '.xml', item_xml_str)

                ext_el = self._core.data.xml.element('item')
                ext_el.set('external', str(True))
                set_item_attributes(ext_el)

                # Save child available date.
                try: ext_el.set('originally_available_at', item_el.xpath('//Episode/originally_available_at')[0].text)
                except: pass

                attr_el.append(ext_el)


        # Return the combined map element   
        return [attr_el]
          
        
          
      
    elif isinstance(attr, templates.SetTemplate):
      item_list = []
      
      for source in p_sources:
        if source in candidates:
          attr_el = candidates[source]
          for item_el in attr_el.xpath('item'):
            item_list.append(item_el)
          
        # If we are overriding, not merging, break when the item list becomes populated after checking a source
        if rule_action == 'override' and len(item_list) > 0:
          break
          
      # Update the index values and create a combined element
      attr_el = self._core.data.xml.element(name)
      count = 0
      for item_el in item_list:
        item_el.set('index', str(count))
        count += 1
        attr_el.append(item_el)
      
      return [attr_el]
      #el.append(attr_el)
          
    elif isinstance(attr, templates.LinkTemplate):
      if rule_action == 'override':
        for source in p_sources:
          if source in candidates and candidates[source].text is not None:
            return [candidates[source]]
      else:
        raise Framework.exceptions.FrameworkException("Unable to perform the action '%s' on an object of type %s" % (rule_action, type(attr)))

    elif isinstance(attr, templates.ObjectContainerTemplate):
      if rule_action == 'merge':
        final_el = self._core.data.xml.element("MediaContainer")
        for source in p_sources:
          if source in candidates:
            in_path = os.path.join(root_path, source, internal_path)
            source_xml = self._core.storage.load(in_path + '.xml')
            source_el = self._core.data.xml.from_string(source_xml, remove_blank_text=True)
            final_el.extend(source_el)

        size = len(list(final_el))
        final_path = out_path + '.xml'
        if size > 0:
          final_el.set('size', str(size))
          final_xml = self._core.data.xml.to_string(final_el)
          if not os.path.exists(out_path):
            os.makedirs(out_path)
          self._core.storage.save(final_path, final_xml)
        elif os.path.exists(final_path):
          os.unlink(final_path)

      else:
        raise Framework.exceptions.FrameworkException("Unable to perform the action '%s' on an object of type %s" % (rule_action, type(attr)))


    else:
      raise Framework.exceptions.FrameworkException("Unable to combine object of type "+str(type(attr)))

  def _apply_sources(self, p_sources, config_identifier, config_el, candidates={}, respect_tags=False):
    sources_config = config_el.xpath('sources')
    if len(sources_config) > 0:
      sc_el = sources_config[0]
      action = sc_el.get('action')
      if action == None or action == 'override':
        p_sources = list()
      elif action == 'append':
        pass
      else:
        raise Framework.exceptions.FrameworkException("Unknown action '%s'" % action)
      for agent_el in sc_el.xpath('agent'):
        p_sources.append(agent_el.text)
    if config_identifier not in p_sources:
      p_sources.append(config_identifier)
      
    # Special case for local media, return all contributors.
    if config_identifier == 'com.plexapp.agents.localmedia' and len(sources_config) == 0:
      for identifier in candidates.keys():
        if identifier not in p_sources:
          p_sources.append(identifier)

    # Special case for "respect tags", bump localmedia to the top of the list.
    if respect_tags and 'com.plexapp.agents.localmedia' in p_sources:
      p_sources.remove('com.plexapp.agents.localmedia')
      p_sources.insert(0, 'com.plexapp.agents.localmedia')

    return p_sources
    
  def _get_rules(self, config_el):
    rules = {}
    for attr_el in config_el.xpath('rules/attribute'):
      attr_name = attr_el.get('name')
      rules[attr_name] = attr_el
    return rules
        
  def _combine_object(self, root_path, internal_path, config_identifier, config_el, candidates, p_sources, template, parts=[], cls=None, agent_info={}, respect_tags=False):
    out_path = os.path.join(root_path, '_combined', internal_path)
    
    # Apply any source changes specified in the config file
    p_sources = self._apply_sources(p_sources, config_identifier, config_el, candidates, respect_tags)

    # Check for rules to apply to attributes
    rules = self._get_rules(config_el)
    
    # Create a new element for the combined object
    el = self._core.data.xml.element(template.__name__)
    
    if isinstance(template, templates.RecordTemplate):
      attrs = dir(type(template))
    else:
      attrs = dir(template)
    
    for name in attrs:
      # Skip excluded attributes
      if name in excluded_attr_names or name[0] == '_':
        continue
      
      # If we were given a part list, only combine if the attribute name matches
      if len(parts) > 0 and parts[0] != name:
        #print "Skipping %s because it isn't %s" % (name, parts[0])
        continue
      
      attr = getattr(template, name)
      if isinstance(attr, templates.AttributeTemplate):
        if name in rules:
          rule = rules[name]
        else:
          rule = self._core.data.xml.element('attribute')
        
        # Get the available candidates for the attribute
        attr_candidates = {}
        for source in candidates:
          source_el_list = candidates[source].xpath(name)
          if len(source_el_list) > 0:
            attr_candidates[source] = source_el_list[0]

        attr_template = getattr(template, name)
        setattr(attr_template, '__name__', name)
        attr_els = self._combine_attr(root_path, os.path.join(internal_path, name), attr, config_identifier, rule, attr_candidates, list(p_sources), attr_template, parts, cls, agent_info)
        if attr_els is not None:
          el.extend(attr_els)
        
    return el
    
  def _combine_file(self, root_path, internal_path, config_identifier, config_el, a_sources, p_sources, template, parts=[], cls=None, agent_info={}, respect_tags=False):
    candidates = {}
    for a_source in a_sources:
      file_path = os.path.join(root_path, a_source, internal_path, 'Info.xml')
      if os.path.exists(file_path):
        source_xml_str = self._core.storage.load(file_path)
        source_el = self._core.data.xml.from_string(source_xml_str, remove_blank_text=True)
        candidates[a_source] = source_el

    return self._combine_object(root_path, internal_path, config_identifier, config_el, candidates, list(p_sources), template, parts, cls, agent_info, respect_tags)

  def _canonical_class_name(self, cls):
    if cls.__name__ == 'LegacyArtist':
      return 'Artist'
    if cls.__name__ == 'LegacyAlbum':
      return 'Album'
    return cls.__name__

  def name_for_class(self, cls):
    return Framework.utils.plural(cls._model_name.replace('_', ' '))
    
  def contributing_agents(self, cls, config_identifier, include_config_identifier=False):
    class_name = self.name_for_class(cls)
    config_file_path = os.path.join(self._config_path, config_identifier, class_name + '.xml')
    if not self._core.storage.file_exists(config_file_path):
      return []
      
    config_str = self._core.storage.load(config_file_path)
    config_el = self._core.data.xml.from_string(config_str)
    agents = []
    for agent_el in config_el.xpath('//agent'):
      if agent_el.text not in agents and (include_config_identifier or agent_el.text != config_identifier):
        agents.append(agent_el.text)
    return agents
  
  def get_config_el(self, cls, config_identifier):
    """ Load the config XML file for the given class/identifier combination and return it """
    self._dlog("Class: %s" % str(cls))
    class_name = self.name_for_class(cls)
    self._dlog("Class name: %s" % str(class_name))
    config_file_path = self._core.storage.join_path(self._config_path, config_identifier, class_name + '.xml')
    self._dlog("Config file path: %s", config_file_path)
    if self._core.storage.file_exists(config_file_path):
      config_str = self._core.storage.load(config_file_path)
      config_el = self._core.data.xml.from_string(config_str)
      self._dlog("File exists, data: %s", config_str)
    else:
      self._dlog("NO FILE EXISTS AT PATH!")
      config_el = self._core.data.xml.element('combine')
    return config_el
  

  def combine(self, cls, config_identifier, guid, agent_info={}, respect_tags=False):
    identifier, path = guid.split('://')
    qs = ''
    if path.find('?') != -1:
      (path, qs) = path.split('?')
      qs = '?' + qs
    
    # HACK: Some agents uses a '/' character in their IDs, which doesn't play nicely with
    # the new partial GUID format (e.g. com.plexapp.agents.thetvdb://12345/seasons/1/episodes/1)
    # If this is a bundle from one of those agents, disable support for partial combination and
    # accept the GUID as provided.
    
    agents_that_happen_to_do_things_a_little_differently = ['mbid', 'plex', 'com.plexapp.agents.lastfm', 'com.plexapp.agents.allmusic', 'com.plexapp.agents.plexmusic']
    
    if identifier in agents_that_happen_to_do_things_a_little_differently or cls._template.use_hashed_map_paths:
      # Define an empty list of parts
      parts = []
      
    else:    
      # Check for a partial GUID, and split any provided parts into a separate list
      parts = path.split('/')
      metadata_id = parts[0]
      parts = parts[1:]
    
      # Fix the main GUID if parts were provided
      if len(parts) > 0:
        guid = '%s://%s%s' % (identifier, metadata_id, qs)

        # The guid for these agents can not be used for combining season or episode
        # We provide com.plexapp.agents.thetvdb://12345/1/5 but we need com.plexapp.agents.thetvdb://12345/seasons/1/episodes/5
        # Add the season and episode part back
        if identifier in ['com.plexapp.agents.themoviedb', 'com.plexapp.agents.thetvdb']:
          if len(parts) == 1:
            # e.g com.plexapp.agents.thetvdb://12345/1
            parts = ['seasons', parts[0]]
          elif len(parts) == 2:
            # e.g com.plexapp.agents.thetvdb://12345/1/5
            parts = ['seasons', parts[0], 'episodes', parts[1]]

    guid_hash = self._core.data.hashing.sha1(guid)
    class_name = self.name_for_class(cls)
    bundle_path = os.path.join(self._model_path, class_name, guid_hash[0], guid_hash[1:] + '.bundle')
    template = cls._template
    config_el = self.get_config_el(cls, config_identifier)
    
    available_sources = [config_identifier]
    preferred_sources = ['_custom']
    
    # Get a list of available data sources
    root_path = os.path.join(bundle_path, 'Contents')
    for name in os.listdir(root_path):
      if name[0] != '_' or name == '_custom':
        available_sources.append(name)

    # Whack the existing combined directory unless a partial combine has been requested
    out_path = self._core.storage.join_path(root_path, '_combined')
    if len(parts) == 0:
      self._core.storage.remove_tree(out_path)
      out_file_path = self._core.storage.join_path(out_path, 'Info.xml')

    el = self._combine_file(root_path, '', config_identifier, config_el, available_sources, preferred_sources, template, parts, cls, agent_info, respect_tags)

    # Whack stored files we're not meant to persist
    def whack_stored(combined_path, stored_path):
      for name in self._core.storage.list_dir(stored_path):
        spath = self._core.storage.join_path(stored_path, name)
        cpath = spath.replace('_stored', '_combined')
        
        # Recurse into directories
        if self._core.storage.dir_exists(spath):
          whack_stored(cpath, spath)

        elif spath[0] != '.':
          # Grab the identifier and get the stored file persistence attribute from the info dict.
          ident = name[:name.rfind('_')]
          info = agent_info.get(ident, [])
          media_type_done = []
          for info_dict in info:
            clsname = self._canonical_class_name(cls)
            if clsname in info_dict.get('media_types', []) and clsname not in media_type_done:
              media_type_done.append(clsname)
              persist_stored_files = info_dict.get('persist_stored_files', True)

              # Compute the original file path
              opath = self._core.storage.join_path(stored_path.replace('_stored', ident), name[name.rfind('_')+1:])
            
              # If stored files shouldn't be persisted and a combined file doesn't exist, whack it.
              if persist_stored_files == False and self._core.storage.file_exists(opath) == False:
                self._core.log.debug("Removing stored file at %s", spath)
                self._core.storage.remove(spath)
                if self._core.storage.file_exists(cpath):
                  self._core.storage.remove(cpath)

    try:
      stored_path = self._core.storage.join_path(root_path, '_stored')
      if self._core.storage.dir_exists(stored_path):
        whack_stored(out_path, stored_path)
    except:
      self._core.log_exception("Error removing redundant stored data")

    # Write the root XML file unless we're in a partial combine
    if len(parts) == 0:
      xml_str = self._core.data.xml.to_string(el)

      Framework.utils.makedirs(out_path)
      self._core.storage.save(out_file_path, xml_str)

      # For Artists and Albums, we return the XML for POSTing directly to PMS.
      clsname = self._canonical_class_name(cls)
      if clsname == 'Artist' or clsname == 'Album':
        return xml_str

