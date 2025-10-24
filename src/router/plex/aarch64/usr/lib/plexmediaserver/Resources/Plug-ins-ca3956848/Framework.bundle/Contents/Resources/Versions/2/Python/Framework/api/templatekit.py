#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BaseKit

class ProxyTemplateKit(object):
  Preview         = Framework.modelling.templates.ProxyTemplate('Preview')
  Media           = Framework.modelling.templates.ProxyTemplate('Media')
  LocalFile       = Framework.modelling.templates.ProxyTemplate('LocalFile')
  Remote          = Framework.modelling.templates.ProxyTemplate('Remote')

class TemplateKit(BaseKit):
  """
    TemplateKit is a very simple API that provides access to the template classes
    in Framework.modelling for use when defining model templates.
  """

  _included_policies = [
    Framework.policies.ModelPolicy,
  ]

  Abstract        = Framework.modelling.templates.AbstractTemplate
  Model           = Framework.modelling.templates.ModelTemplate
  Record          = Framework.modelling.templates.RecordTemplate
  Link            = Framework.modelling.templates.LinkTemplate
  
  Set             = Framework.modelling.templates.SetTemplate
  Map             = Framework.modelling.templates.MapTemplate
  Directory       = Framework.modelling.templates.DirectoryTemplate
  ProxyContainer  = Framework.modelling.templates.ProxyContainerTemplate
  
  String          = Framework.modelling.templates.StringTemplate
  Integer         = Framework.modelling.templates.IntegerTemplate
  Float           = Framework.modelling.templates.FloatTemplate
  Boolean         = Framework.modelling.templates.BooleanTemplate
  Date            = Framework.modelling.templates.DateTemplate
  Time            = Framework.modelling.templates.TimeTemplate
  Datetime        = Framework.modelling.templates.DatetimeTemplate
  
  Data            = Framework.modelling.templates.DataTemplate
  Proxy           = ProxyTemplateKit

  ObjectContainer = Framework.modelling.templates.ObjectContainerTemplate
