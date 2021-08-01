#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Locale, String, Resource

def L(string): return Locale.LocalString(string)
def F(key, *args): return Locale.LocalStringWithFormat(key, *args)
def E(string): return String.Encode(string)
def D(string): return String.Decode(string)
def R(itemName): return Resource.ExternalPath(itemName)
def S(itemName): return Resource.SharedExternalPath(itemName)
