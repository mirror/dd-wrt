import Framework

from base import BaseHandler, InternalRequestHandler

class DebugRequestHandler(InternalRequestHandler):

  @BaseHandler.route('/debugging/allocations')
  def allocations(self):
    return self._core.debugging.allocations()

  @BaseHandler.route('/debugging/thread_stacks')
  def thread_stacks(self):
  	return self._core.debugging.thread_stacks()
