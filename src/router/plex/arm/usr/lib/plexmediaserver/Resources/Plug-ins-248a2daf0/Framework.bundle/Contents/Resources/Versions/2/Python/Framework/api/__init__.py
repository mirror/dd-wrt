#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

from base         import BaseKit
from runtimekit   import RuntimeKit
from parsekit     import ParseKit
from templatekit  import TemplateKit
from modelkit     import ModelKit
from agentkit     import AgentKit
from resourcekit  import ResourceKit
from localekit    import LocaleKit
from networkkit   import NetworkKit
from messagekit   import MessageKit
from servicekit   import ServiceKit
from datakit      import DataKit
from corekit      import CoreKit
from playerkit    import PlayerKit
from threadkit    import ThreadKit
from helperkit    import HelperKit
from objectkit    import ObjectKit
from constkit     import ConstKit
from logkit       import LogKit
from utilkit      import UtilKit
from streamkit    import StreamKit
from cloudkit     import CloudKit
from cryptokit    import CryptoKit


class DevKit(BaseKit):

  _root_object = False

  _children = [
    CoreKit,
    RuntimeKit,
    LocaleKit,
    ParseKit,
    TemplateKit,
    ModelKit,
    AgentKit,
    ResourceKit,
    NetworkKit,
    MessageKit,
    ServiceKit,
    DataKit,
    PlayerKit,
    ThreadKit,
    HelperKit,
    ObjectKit,
    ConstKit,
    LogKit,
    UtilKit,
    StreamKit,
    CloudKit,
    CryptoKit,
  ]
