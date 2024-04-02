import Framework

from base import BaseHandler, InternalRequestHandler

class MessagingRequestHandler(InternalRequestHandler):

  @BaseHandler.route('/messaging/function/{name}/{packed_args}/{packed_kwargs}')
  def function_call(self, name, packed_args, packed_kwargs):
    # Decode the name
    name = Framework.utils.safe_decode(name)
    
    # Unpack the name, arguments & keyword arguments
    args = Framework.utils.unpack(packed_args)
    kwargs = Framework.utils.unpack(packed_kwargs)
    
    return self._core.messaging._process_function_call(name, args, kwargs)
    
  @BaseHandler.route('/messaging/event/{name}/{packed_args}/{packed_kwargs}')
  def event(self, name, packed_args, packed_kwargs):
    # Decode the name
    name = Framework.utils.safe_decode(name)
    
    # Unpack the arguments & keyword arguments
    args = Framework.utils.unpack(packed_args)
    kwargs = Framework.utils.unpack(packed_kwargs)
  
    self._core.messaging._process_event(name, args, kwargs)
    
  @BaseHandler.route('/messaging/wait')
  def wait(self):
    return Framework.utils.pack(True)
