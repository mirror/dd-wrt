#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework
from pipeinterface import PipeInterface
import sys
import socket
import threading
import time

import tornado.httpserver
import tornado.web
import tornado.ioloop

SHUTDOWN_URI = '/:/shutdownInterface'


class PluginRequestHandler(tornado.web.RequestHandler):
  _core = None
  _write_body = True

  SUPPORTED_METHODS = ("GET", "HEAD", "POST", "DELETE", "PUT", "OPTIONS")
  
  @property
  def _core(self):
    return type(self)._core
    
  def _handle_request_threaded(self):
    self._core.runtime.create_thread(self._handle_request)
    
  @tornado.web.asynchronous
  def get(self):
    self._handle_request_threaded()
    
  @tornado.web.asynchronous
  def put(self):
    self._handle_request_threaded()
    
  @tornado.web.asynchronous
  def post(self):
    self._handle_request_threaded()

  @tornado.web.asynchronous
  def delete(self):
    self._handle_request_threaded()
    
  @tornado.web.asynchronous
  def head(self):
    self.request.method = 'GET'
    self._write_body = False
    self._handle_request_threaded()

  def options(self):
    self._headers = {
      #'Access-Control-Allow-Origin': '*',
      'Access-Control-Max-Age': '86400',
      'Access-Control-Allow-Methods': 'GET, HEAD, POST, DELETE, PUT, OPTIONS',
    }

    request_headers = self.request.headers.get('Access-Control-Request-Headers')
    if request_headers != None:
      self._headers['Access-Control-Allow-Headers'] = request_headers

    self.set_status(200)
    self.finish()

  def _handle_request(self):
    try:
      # Lock here so we can modify the application's state information safely
      self.application.lock.acquire()
      
      # Get the current active state
      active = self.application.active
      
      # If we received a shutdown request, set the active state to False
      if self.request.uri == SHUTDOWN_URI:
        self.application.active = False
        if self.application.server and self.application.server._socket:
          self.application.server._socket.close()
          self.application.server._socket = None
        
      # If the app's active, increment the request count
      elif active:
        self.application.requests_in_progress += 1
        
      self.application.lock.release()
    
      # If this is the shutdown request, wait for all requests to complete before returning
      if self.request.uri == SHUTDOWN_URI:
        self.application.request_condition.acquire()

        while self.application.requests_in_progress > 0:
          self.application.request_condition.wait()

        self.application.request_condition.release()

        self.set_status(200)

        
      # If this is a normal request and the app is active, pass it to the framework core and return the result
      elif active:
        status, headers, body = type(self)._core.runtime.handle_request(self.request)
      
        self.set_status(status)
        self._headers = headers

        if self._write_body == False:
          self._headers['Content-Length'] = str(len(body))
    
        if self._write_body:
          self.write(body)
      
      
      # If the application is inactive, return a 503 error
      else:
        self.set_status(503)
      
      
    except:
      self._core.log_exception("Exception when writing response for request '%s'", self.request.uri)
      
      
    finally:
      self.finish()
      
      # If this was an active request...
      if active and self.request.uri != SHUTDOWN_URI:
        # Lock again, since we're modifying the state
        self.application.lock.acquire()
        
        # Decrement the request count
        self.application.requests_in_progress -= 1
        
        # Notify threads waiting for a request to end
        self.application.request_condition.acquire()
        self.application.request_condition.notifyAll()
        self.application.request_condition.release()
        
        self.application.lock.release()
      

class PluginApplication(tornado.web.Application):
  def __init__(self, handlers=None, default_host="", transforms=None, wsgi=False, **settings):
    tornado.web.Application.__init__(self, handlers, default_host, transforms, wsgi, **settings)
    
    # A few extra properties to store application state information.
    # See PluginRequestHandler (above) for their usage.
    self.active = True
    self.requests_in_progress = 0
    self.request_condition = threading.Condition()
    self.lock = threading.Lock()
    self.server = None


class SocketInterface(PipeInterface):
    
  def listen(self, daemonized):
    self._core.log.debug('Starting socket server')
    
    # Get the port from the config dictionary
    port = self._core.config.socket_interface_port
    
    # Create the Tornado application and server objects
    PluginRequestHandler._core = self._core
    application = PluginApplication([
      (r".*", PluginRequestHandler)
    ], transforms=[])
    server = tornado.httpserver.HTTPServer(application)
    application.server = server
    
    # Attempt to listen on the given port
    server.listen(port, address='127.0.0.1')
    
    # If no port was defined, check which one we're actually listening on
    if port == 0:
      port = server._socket.getsockname()[1]
      
    self._core.runtime._interface_args['port'] = port
    
    # Start the IOLoop
    self._core.runtime.create_thread(tornado.ioloop.IOLoop.instance().start)
    self._core.log.info("Socket server started on port %s", port)

    # Start pipe communication if we're not daemonized
    if not daemonized:
      PipeInterface.listen(self, False)
