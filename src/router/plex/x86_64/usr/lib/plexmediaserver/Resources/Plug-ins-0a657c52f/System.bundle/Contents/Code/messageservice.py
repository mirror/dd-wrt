#
#  Plex Plug-in Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from systemservice import SystemService

EVENTS_KEY = '_MessageService:Events'


class MessageService(SystemService):
  
  def __init__(self, system):
    SystemService.__init__(self, system)
    
    # Create or load the events dict
    if EVENTS_KEY in Dict:
      self.events = Dict[EVENTS_KEY]
    else:
      self.events = dict()
      
    # Connect the routes
    Route.Connect('/system/messaging/register_for_event/{target}/{encoded_key}', self.register_for_event)
    Route.Connect('/system/messaging/broadcast_event/{source}/{encoded_key}/{packed_args}/{packed_kwargs}', self.broadcast_event)
    Route.Connect('/system/messaging/clear_events/{target}', self.clear_events)
    
    
  def register_for_event(self, target, encoded_key):
    Thread.AcquireLock(self)
    try:
      # Decode the key
      key = Framework.utils.safe_decode(encoded_key)
      
      # Create an event set for this target if one doesn't already exist
      if target not in self.events:
        self.events[target] = set()
      
      # Add the key to the event set and save the events dict
      self.events[target].add(key)
      Dict[EVENTS_KEY] = self.events
      
    finally:
      Thread.ReleaseLock(self)
    return ''
    
    
  def fire_event(self, target, encoded_key, packed_args, packed_kwargs):
    # Forwards an event to a specific plug-in
    url = Core.messaging.generate_messaging_url(target, 'event', encoded_key, packed_args, packed_kwargs)
    try:
      HTTP.Request(url, cacheTime = 0, immediate = True)
    except:
      pass
    
  def broadcast_event(self, source, encoded_key, packed_args, packed_kwargs):
    Thread.AcquireLock(self)
    
    try:
      # Decode the key
      key = Framework.utils.safe_decode(encoded_key)
      Log.Info("Broadcasting event %s", key)
      
      # Check each event set to see if it contains the key
      for target in self.events:
        
        # If so, forward the event in a new thread
        if key in self.events[target]:
          Core.runtime.create_thread(self.fire_event, target, encoded_key, packed_args, packed_kwargs)
    
    finally:
      Thread.ReleaseLock(self)
    return ''
    
    
  def clear_events(self, target):
    Thread.AcquireLock(self)
    
    try:
      # If an event set exists for this target, remove it and save the events dict
      if target in self.events:
        del self.events[target]
        Dict[EVENTS_KEY] = self.events 
        
    finally:
      Thread.ReleaseLock(self)  
    return ''


