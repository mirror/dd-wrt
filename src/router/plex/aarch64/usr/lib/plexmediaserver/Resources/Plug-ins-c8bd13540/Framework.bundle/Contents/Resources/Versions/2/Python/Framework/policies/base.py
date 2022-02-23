#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework


class BasePolicy(object):
  
  environment = {}
  ext = 'py'

  allow_whitelist_extension = False
  elevated_execution = False
  allow_bundled_libraries = False
  block_imports = False
  always_use_session_cookies = False
  allow_global_cookies = True
  enable_http_caching = True
  allow_global_http_auth = True
  synthesize_defaults = False
  enable_auto_generated_routes = True
  

class CodePolicy(BasePolicy): pass
class BundlePolicy(CodePolicy): pass
class ModernPolicy(BasePolicy): pass
