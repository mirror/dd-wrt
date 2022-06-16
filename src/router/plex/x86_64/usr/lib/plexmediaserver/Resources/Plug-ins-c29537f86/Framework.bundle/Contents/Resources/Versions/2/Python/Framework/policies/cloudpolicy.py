#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BundlePolicy, ModernPolicy


class CloudPolicy(BundlePolicy, ModernPolicy):

  block_imports = True
  always_use_session_cookies = True
  allow_global_cookies = False
  enabled_http_caching = False
  allow_global_http_auth = False
  synthesize_defaults = True
  enable_auto_generated_routes = False
