from systemservice import SystemService

class CodeService(SystemService):
  def __init__(self, system):
    SystemService.__init__(self, system)
    Route.Connect('/system/services/url/lookup', Core.runtime.find_handler(Framework.handlers.ServiceRequestHandler).url_lookup)
    Route.Connect('/system/services/search', self.cs_search)
    Route.Connect('/system/services/searches', self.cs_searches)
    Route.Connect('/system/services/relatedcontent/lookup', self.cs_relatedcontent_lookup)
    
    if hasattr(Core.config, 'daemonized') and Core.config.daemonized:
      Route.Connect('/services/url/lookup', Core.runtime.find_handler(Framework.handlers.ServiceRequestHandler).url_lookup)
      Route.Connect('/services/search', self.cs_search)
      Route.Connect('/services/relatedcontent/lookup', self.cs_relatedcontent_lookup)
      
  def cs_search(self, query, identifier, name=None):
    result = SearchService.Query(query, identifier, name)
    return result

  def cs_searches(self, identifier):
    oc = ObjectContainer()
    for name in SearchService.List(identifier):
      oc.add(SearchDirectoryObject(identifier=identifier, name=name, title=name))
    return oc
    
  def cs_relatedcontent_lookup(self, url):
    obj = URLService.MetadataObjectForURL(url)
    
    if obj == None:
      return None
      
    identifier = URLService.ServiceIdentifierForURL(url)
    if identifier == None:
      return None
      
    result = RelatedContentService.RelatedContentForObject(obj)
    return result
