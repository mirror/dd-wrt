#!/usr/bin/python
#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

## CONFIGURE THE PYTHON ENVIRONMENT ##

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

import subsystem

import os
import config

if sys.platform == "win32":
  # This is needed to ensure binary data transfer over stdio between PMS and plugins
  import msvcrt
  msvcrt.setmode(sys.stdin.fileno(), os.O_BINARY)
  msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)
 
  # This is needed to prevent explosions when there's a system Python installed whose libraries conflict with our own
  if 'PLEXLOCALAPPDATA' in os.environ:
    key = 'PLEXLOCALAPPDATA'
  else:
    key = 'LOCALAPPDATA'
  ourlocalappdata = os.path.join(os.environ[key], 'Plex Media Server')
  for x in [x for x in sys.path if sys.prefix.lower() not in x.lower() and ourlocalappdata.lower() not in x.lower()]: sys.path.remove(x)
else:
  import traceback
  import sys
  def dumpstacks(signal, frame):
    code = []
    for threadId, stack in sys._current_frames().items():
      code.append("\n# Thread: %d" % (threadId))
      for filename, lineno, name, line in traceback.extract_stack(stack):
        code.append('File: "%s", line %d, in %s' % (filename, lineno, name))
        if line:
          code.append("  %s" % (line.strip()))
    print "\n".join(code)

  import signal
  signal.signal(signal.SIGUSR1, dumpstacks)

PYTHON_DIR = os.path.dirname(os.path.abspath(sys.argv[0]))
FRAMEWORK_DIR = os.path.abspath(os.path.join(PYTHON_DIR, '..'))

# Redirect stdout to stderr
sys.stdout = sys.stderr

if sys.platform == "win32":
  os_name = "Windows"
  cpu_name = "i386"
  #TODO - support Windows x64 (Win64)
else:
  uname = os.uname()
  os_name = uname[0]
  cpu_name = uname[4]

mapped_cpu = config.cpu_map.get(cpu_name, cpu_name)
  
# Special case for Linux/x64 (really should be special case for OS X...)
if os_name == 'Linux' and cpu_name == 'x86_64':
  mapped_cpu = 'x86_64'

PLATFORM_DIR = os.path.abspath(os.path.join(FRAMEWORK_DIR, '..', '..', "Platforms", config.os_map[os_name], mapped_cpu))
SHARED_DIR = os.path.abspath(os.path.join(FRAMEWORK_DIR, '..', '..', "Platforms", "Shared"))
#TODO: Check for errors

# If the environment variable PLEXBUNDLEDEXTS is set, this indicates a newer
# server which comes bundled with its own binaries so we'll skip this step
# completely.
#
if 'PLEXBUNDLEDEXTS' not in os.environ:
  lib_path = os.path.join(PLATFORM_DIR, "Libraries")
  # The binary lib path goes at the end on non-Mac platforms, because binary
  # extensions should be picked up from the PMS Exts directory first.
  #
  if sys.platform != "darwin":
    sys.path.append(lib_path)
  else:
    sys.path.insert(0, lib_path)

# Insert the shared (Python-only) libraries.
sys.path.insert(0, os.path.join(SHARED_DIR, "Libraries"))

## LOAD AND CONFIGURE THE FRAMEWORK ##

import Framework
import Framework.constants as const
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-i", "--interface", dest="interface", default=config.default_interface)
parser.add_option("-q", "--quiet", action="store_false", dest="verbose", default=True)
parser.add_option("-p", "--socket-interface-port", dest="socket_interface_port", default=config.socket_interface_port)
parser.add_option("-s", "--server-version", dest="server_version")
parser.add_option("-d", "--daemon", dest="daemon_command")
parser.add_option("-P", "--pid-file", dest="pid_file")
parser.add_option("-l", "--log-file", dest="log_file")
parser.add_option("-c", "--config-file", dest="config_file")
(options, args) = parser.parse_args()

bundle_path = args[0]

del parser
del OptionParser
      
# Select the interface class to use
if options.interface == const.interface.pipe:
  interface_class = Framework.interfaces.PipeInterface
elif options.interface == const.interface.socket:
  interface_class = Framework.interfaces.SocketInterface
  if int(options.socket_interface_port) != config.socket_interface_port:
    config.socket_interface_port = int(options.socket_interface_port)
else:
  #TODO: Error info - no matching interface found
  sys.stderr.write('No matching interface found.\n')
  sys.exit(1)
  
if options.server_version != None:
  config.server_version = options.server_version

# Configure the log_dir, if one was given
if options.log_file:
  config.log_file = os.path.abspath(options.log_file)
  
# Configure the pid file, if one was given
if options.pid_file:
  config.pid_file = os.path.abspath(options.pid_file)
  
# Load the config file if one was provided
if options.config_file:
  import simplejson
  f = open(options.config_file, 'r')
  json_config = simplejson.load(f)
  f.close()
  for key in json_config:
    setattr(config, key, json_config[key])

def run(daemonized=False):
  # Copy the damonized attribute into config
  setattr(config, 'daemonized', daemonized)
  
  # Create a core object for the plug-in bundle
  core = Framework.core.FrameworkCore(bundle_path, FRAMEWORK_DIR, config)

  # Try to load the plug-in code
  if not core.load_code():
    sys.stderr.write('Error loading bundle code.\n')
    sys.exit(2)

  # Create an instance of the selected interface
  interface = interface_class(core)

  # Try to start the core
  if not core.start():
    sys.stderr.write('Error starting framework core for %s.\n' % bundle_path)
    sys.exit(3)
  
  # Start listening on the interface
  interface.listen(daemonized)
  
# If running normally, start the core in the current process
if options.daemon_command == None:
  run()
  
# If issued a daemon command, check what we're supposed to do
else:
  import plistlib, daemon, tempfile
  
  # Read the plist to get the identifier
  plist_path = os.path.join(bundle_path, 'Contents', 'Info.plist')
  ident = plistlib.readPlist(plist_path)['CFBundleIdentifier']
  
  class PluginDaemon(daemon.Daemon):
    def run(self):
      run(True)

  # Make sure we have a pid file for this daemon
  pid_config = dict(identifier = ident, port = config.socket_interface_port)
  pid_dir = os.path.join(config.root_path, config.pid_files_dir)

  # Create the pid files dir if it doesn't exist
  if not os.path.exists(pid_dir):
    try:
      os.makedirs(pid_dir)
    except:
      pass
    
  if not config.pid_file:
    pid_file = os.path.join(pid_dir, '%(identifier)s.%(port)d.pid' % pid_config)
  else:
    pid_file = config.pid_file % pid_config

  # Create the daemon object
  d = PluginDaemon(pid_file)
  
  # Decide which action to perform
  if options.daemon_command == 'start':
    d.start()
  elif options.daemon_command == 'restart':
    d.restart()
  elif options.daemon_command == 'stop':
    # Wait up to 60 seconds for the interface to stop
    port = 0
    try:
      port = pid_config['port']
      urllib2.urlopen('http://127.0.0.1:%d/:/shutdownInterface' % port, timeout=60)
    except:
      print "Error shutting down interface on port %d" % port
    # Kill the daemon process
    d.stop()
  else:
    print "Unknown command '%s'" % options.daemon_command
    
    