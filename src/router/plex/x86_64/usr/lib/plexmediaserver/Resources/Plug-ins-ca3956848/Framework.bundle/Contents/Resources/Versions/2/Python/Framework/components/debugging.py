#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
import sys
import gc
import threading
import traceback

from base import BaseComponent

    
class Debugging(BaseComponent):

  
  def allocations(self):
    gc.collect()

    counts = {}
    for obj in gc.get_objects():
      objtype = type(obj)
      if objtype in counts:
        counts[objtype] += 1
      else:
        counts[objtype] = 1

    modules = {}
    for objtype in counts:
      module = objtype.__module__
      if module not in modules:
        modules[module] = []

      modules[module].append(dict(
        name = objtype.__name__,
        count = counts[objtype],
      ))

    for module in modules:
      modules[module].sort(key = lambda x: x['count'])

    return modules

  def thread_stacks(self):
    id2name = dict([(th.getName(), th.getName()) for th in threading.enumerate()])
    
    code = []
    for threadId, stack in sys._current_frames().items():
      code.append("\n# Thread:") # % (id2name[threadId], threadId))
      for filename, lineno, name, line in traceback.extract_stack(stack):
        code.append('File: "%s", line %d, in %s' % (filename, lineno, name))
        if line:
          code.append("  %s" % (line.strip()))

    return "\n".join(code)
    