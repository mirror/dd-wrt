#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from crypto.cipher.base import padWithPadLen
from crypto.cipher.cbc import CBC
from crypto.cipher.rijndael import Rijndael
from fcrypt import crypt as Crypt

from base import BaseKit

class CipherKit(BaseKit):
  CBC = CBC
  Rijndael = Rijndael
  PadWithPadLen = padWithPadLen
  Crypt = Crypt


class CryptoKit(BaseKit):

  _included_policies = [
    Framework.policies.CodePolicy
  ]

  _root_object = False

  def _init(self):
    self._publish(CipherKit)

