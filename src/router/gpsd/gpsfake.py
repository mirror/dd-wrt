"""
$Id: gpsfake.py 4121 2006-12-12 17:30:01Z esr $

gpsfake.py -- classes for creating a controlled test environment around gpsd.

The gpsfake(1) regression tester shipped with gpsd is a trivial wrapper
around this code.  For a more interesting usage example, see the
valgrind-audit script shipped with the gpsd code.

To use this code, start by instantiating a TestSession class.  Use the
prefix argument if you want to run the daemon under some kind of run-time
monitor like valgrind or gdb.  Here are some particularly useful possibilities:

valgrind --tool=memcheck --gen-suppressions=yes --leak-check=yes
    Run under Valgrind, checking for malloc errors and memory leaks.

xterm -e gdb -tui --args
    Run under gdb, controlled from a new xterm.

You can use the options argument to pass in daemon options; normally you will
use this to set the debug-logging level.

On initialization, the test object spawns an instance of gpsd with no
devices or clients attached, connected to a control socket.

TestSession has methods to attach and detch fake GPSes. The
TestSession class simulates GPS devices for you with objects composed
from a pty and a class instance that cycles sentences into the master side
from some specified logfile; gpsd reads the slave side.  A fake GPS is
identified by the string naming its slave device.

TestSession also has methods to start and end client sessions.  Daemon
responses to a client are fed to a hook function which, by default,
discards them.  You can change the hook to sys.stdout.write() to dump
responses to standard output (this is what the gpsfake executable
does) or do something more exotic. A client session is identified by a
small integer that counts the number of client session starts.

There are a couple of convenience methods.  TestSession.wait() does nothing,
allowing a specified number of seconds to elapse.  TestSession.client_query()
ships commands to an open client session.

TestSession does not currently capture the daemon's log output.  It is
run with -N, so the output will go to stderr (along with, for example,
Valgrind notifications).

Each FakeGPS instance tries to packetize the data from the logfile it
is initialized with.  It looks for packet headers associated with common
packet types such as NMEA, SiRF, TSIP, and Zodiac.  Additionally, the Type
header in a logfile can be used to force the packet type, notably to RTCM
which is fed to the daemon character by character,

The TestSession code maintains a run queue of FakeGPS and gps.gs (client-
session) objects. It repeatedly cycles through the run queue.  For each
client session object in the queue, it tries to read data from gpsd.  For
each fake GPS, it sends one line of stored data.  When a fake-GPS's
go predicate becomes false, the fake GPS is removed from the run queue.

There are two ways to use this code.  The more deterministic is
non-threaded mode: set up your client sessions and fake GPS devices,
then call the run() method.  The run() method will terminate when
there are no more objects in the run queue.  Note, you must have
created at least one fake client or fake GPS before calling run(),
otherwise it will terminate immediately.

To allow for adding and removing clients while the test is running,
run in threaded mode by calling the start() method.  This simply calls
the run method in a subthread, with locking of critical regions.
"""
import sys, os, time, signal, pty, termios
import string, exceptions, threading, socket, commands
import gps

class TestLoadError(exceptions.Exception):
    def __init__(self, msg):
        self.msg = msg

class TestLoad:
    "Digest a logfile into a list of sentences we can cycle through."
    def __init__(self, logfp):
        self.sentences = []	# This and .packtype are the interesting bits
        self.logfp = logfp
        self.logfile = logfp.name
        self.type = None
        self.serial = None
        # Skip the comment header
        while True:
            first = logfp.read(1)
            self.first = first;
            if first == "#":
                line = logfp.readline()
                if line.strip().startswith("Type:"):
                    if line.find("RTCM") > -1:
                        self.type = "RTCM"
                if "Serial:" in line:
                    line = line[1:].strip()
                    try:
                        (xx, baud, params) = line.split()
                        baud = int(baud)
                        if params[0] in ('7', '8'):
                            databits = int(params[0])
                        else:
                            raise ValueError
                        if params[1] in ('N', 'O', 'E'):
                            parity = params[1]
                        else:
                            raise ValueError
                        if params[2] in ('1', '2'):
                            stopbits = int(params[2])
                        else:
                            raise ValueError
                    except ValueError, IndexError:
                        raise TestLoadError("bad serial-parameter spec in %s"%\
                                            logfp.name)
                    
                    self.serial = (baud, databits, parity, stopbits)
            else:
                break
        # Grab the packets
        while True:
            packet = self.packet_get()
            #print "I see: %s, length %d" % (`packet`, len(packet))
            if not packet:
                break
            else:
                self.sentences.append(packet)
        # Look at the first packet to grok the GPS type
        if self.sentences[0][0] == '$':
            self.packtype = "NMEA"
            self.legend = "gpsfake: line %d: "
            self.idoffset = None
            self.textual = True
        elif self.sentences[0][0] == '\xff':
            self.packtype = "Zodiac binary"
            self.legend = "gpsfake: packet %d: "
            self.idoffset = None
            self.textual = True
        elif self.sentences[0][0] == '\xa0':
            self.packtype = "SiRF binary"
            self.legend = "gpsfake: packet %d: "
            self.idoffset = 3
            self.textual = False
        elif self.sentences[0][0] == '\x10':
            self.packtype = "TSIP binary"
            self.legend = "gpsfake: packet %d: "
            self.idoffset = 1
            self.textual = False
        elif self.type == "RTCM":
            self.packtype = "RTCM"
            self.legend = None
            self.idoffset = None
            self.textual = False
        else:
            sys.stderr.write("gpsfake: unknown log type (not NMEA or SiRF) can't handle it!\n")
            self.sentences = None
    def packet_get(self):
        "Grab a packet.  Unlike the daemon's state machine, this assumes no noise."
        if self.first == '':
            first = self.logfp.read(1)
        else:
            first=self.first
            self.first=''
        if not first:
            return None
        elif self.type == "RTCM":
            return first
        elif first == '$':					# NMEA packet
            return "$" + self.logfp.readline()
        second = self.logfp.read(1)
        if first == '\xa0' and second == '\xa2':		# SiRF packet
            third = self.logfp.read(1)
            fourth = self.logfp.read(1)
            length = (ord(third) << 8) | ord(fourth)
            return "\xa0\xa2" + third + fourth + self.logfp.read(length+4)
        elif first == '\xff' and second == '\x81':		# Zodiac
            third = self.logfp.read(1)
            fourth = self.logfp.read(1)
            fifth = self.logfp.read(1)
            sixth = self.logfp.read(1)
            id = ord(third) | (ord(fourth) << 8)
            ndata = ord(fifth) | (ord(sixth) << 8)
            return "\xff\x81" + third + fourth + fifth + sixth + self.logfp.read(2*ndata+6)
        elif first == '\x10':					# TSIP
            packet = first + second
            delcnt = 0
            while True:
                next = self.logfp.read(1)
                if not next:
                    return ''
                packet += next
                if next == '\x10':
                    delcnt += 1
                elif next == '\x03':
                    if delcnt % 2:
                        break
                else:
                    delcnt = 0
            return packet
        else:
            raise PacketError("unknown packet type, leader %s, (0x%x)" % (first, ord(first)))

class PacketError(exceptions.Exception):
    def __init__(self, msg):
        self.msg = msg

class FakeGPS:
    "A fake GPS is a pty with a test log ready to be cycled to it."
    def __init__(self, logfp,
                 speed=4800, databits=8, parity='N', stopbits=1,
                 verbose=False):
        self.verbose = verbose
        self.go_predicate = lambda: True
        self.readers = 0
        self.index = 0
        self.speed = speed
        baudrates = {
            0: termios.B0,
            50: termios.B50,
            75: termios.B75,
            110: termios.B110,
            134: termios.B134,
            150: termios.B150,
            200: termios.B200,
            300: termios.B300,
            600: termios.B600,
            1200: termios.B1200,
            1800: termios.B1800,
            2400: termios.B2400,
            4800: termios.B4800,
            9600: termios.B9600,
            19200: termios.B19200,
            38400: termios.B38400,
            57600: termios.B57600,
            115200: termios.B115200,
            230400: termios.B230400,
        }
        speed = baudrates[speed]	# Throw an error if the speed isn't legal
        if type(logfp) == type(""):
            logfp = open(logfp, "r");            
        self.testload = TestLoad(logfp)
        # FIXME: explicit arguments should probably override this
        #if self.testload.serial:
        #    (speed, databits, parity, stopbits) = self.testload.serial
        (self.master_fd, self.slave_fd) = pty.openpty()
        self.slave = os.ttyname(self.slave_fd)
        ttyfp = open(self.slave, "rw")
        (iflag, oflag, cflag, lflag, ispeed, ospeed, cc) = termios.tcgetattr(ttyfp.fileno())
        cc[termios.VMIN] = 1
	cflag &= ~(termios.PARENB | termios.PARODD | termios.CRTSCTS)
	cflag |= termios.CREAD | termios.CLOCAL
        iflag = oflag = lflag = 0
 	iflag &=~ (termios.PARMRK | termios.INPCK)
 	cflag &=~ (termios.CSIZE | termios.CSTOPB | termios.PARENB | termios.PARODD)
        if databits == 7:
            cflag |= termios.CS7
        else:
            cflag |= termios.CS8
        if stopbits == 2:
            cflag |= termios.CSTOPB
 	if parity == 'E':
 	    iflag |= termios.INPCK
 	    cflag |= termios.PARENB
 	elif parity == 'O':
 	    iflag |= termios.INPCK
 	    cflag |= termios.PARENB | termios.PARODD
        ispeed = ospeed = speed
        termios.tcsetattr(ttyfp.fileno(), termios.TCSANOW,
                          [iflag, oflag, cflag, lflag, ispeed, ospeed, cc])
    def read(self):
        "Discard control strings written by gpsd."
        termios.tcflush(self.master_fd, termios.TCIFLUSH)
    def feed(self):
        "Feed a line from the contents of the GPS log to the daemon."
        line = self.testload.sentences[self.index % len(self.testload.sentences)]
        os.write(self.master_fd, line)
        # Delay so we won't spam the buffers in the pty layer or gpsd itself.
        # Assumptions: 1 character is 10 bits (generous; at 8N1 it will be 9).
        time.sleep((10.0 * len(line)) / self.speed)
        self.index += 1

class DaemonError(exceptions.Exception):
    def __init__(self, msg):
        self.msg = msg

class DaemonInstance:
    "Control a gpsd instance."
    def __init__(self, control_socket=None):
        self.sockfile = None
        self.pid = None
        if control_socket:
            self.control_socket = control_socket
        else:
            self.control_socket = "/tmp/gpsfake-%d.sock" % os.getpid()
        self.pidfile  = "/tmp/gpsfake_pid-%s" % os.getpid()
    def spawn(self, options, port, background=False, prefix=""):
        "Spawn a daemon instance."
        self.spawncmd = "gpsd -N -S %s -F %s -P %s %s" % (port, self.control_socket, self.pidfile, options)
        if prefix:
            self.spawncmd = prefix + " " + self.spawncmd.strip()
        if background:
            self.spawncmd += " &"
        status = os.system(self.spawncmd)
        if os.WIFSIGNALED(status) or os.WEXITSTATUS(status):
            raise DaemonError("daemon exited with status %d" % status)
    def wait_pid(self):
        "Wait for the daemon, get its PID and a control-socket connection."
        while True:
            try:
                fp = open(self.pidfile)
            except IOError:
                time.sleep(0.5)
                continue
            try:
                fp.seek(0)
                pidstr = fp.read()
                self.pid = int(pidstr)
            except ValueError:
                time.sleep(0.5)
                continue	# Avoid race condition -- PID not yet written
            fp.close()
            break
    def __get_control_socket(self):
        # Now we know it's running, get a connection to the control socket.
        if not os.path.exists(self.control_socket):
            return None
        try:
            self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM, 0)
            self.sock.connect(self.control_socket)
        except socket.error:
            if self.sock:
                self.sock.close()
            self.sock = None
        return self.sock
    def is_alive(self):
        "Is the daemon still alive?"
        try:
            st = os.kill(self.pid, 0)
            return True
        except OSError:
            return False
    def add_device(self, path):
        "Add a device to the daemon's internal search list."
        if self.__get_control_socket():
            self.sock.sendall("+%s\r\n" % path)
            self.sock.recv(12)
            self.sock.close()
    def remove_device(self, path):
        "Remove a device from the daemon's internal search list."
        if self.__get_control_socket():
            self.sock.sendall("-%s\r\n" % path)
            self.sock.recv(12)
            self.sock.close()
    def kill(self):
        "Kill the daemon instance."
        if self.pid:
            try:
                os.kill(self.pid, signal.SIGTERM)
            except OSError:
                pass
            self.pid = None
            time.sleep(1)	# Give signal time to land

class TestSessionError(exceptions.Exception):
    def __init__(self, msg):
        self.msg = msg

class TestSession:
    "Manage a session including a daemon with fake GPSes and clients."
    CLOSE_DELAY = 1
    def __init__(self, prefix=None, port=None, options=None, verbose=False):
        "Initialize the test session by launching the daemon."
        self.verbose = verbose
        self.daemon = DaemonInstance()
        self.fakegpslist = {}
        self.client_id = 0
        self.readers = 0
        self.writers = 0
        self.runqueue = []
        self.index = 0
        if port:
            self.port = port
        else:
            self.port = gps.GPSD_PORT
        self.progress = lambda x: None
        self.reporter = lambda x: None
        for sig in (signal.SIGQUIT, signal.SIGINT, signal.SIGTERM):
            signal.signal(sig, lambda signal, frame: self.cleanup())
        self.daemon.spawn(background=True, prefix=prefix, port=self.port, options=options)
        self.daemon.wait_pid()
        self.default_predicate = None
        self.fd_set = []
        self.threadlock = None
    def set_predicate(self, pred):
        "Set a default go predicate for the session."
        self.default_predicate = pred
    def gps_add(self, logfile, speed=4800, pred=None):
        "Add a simulated GPS being fed by the specified logfile."
        self.progress("gpsfake: gps_add(%s, %d)\n" % (logfile, speed))
        if logfile not in self.fakegpslist:
            newgps = FakeGPS(logfile, speed=speed, verbose=self.verbose)
            if pred:
                newgps.go_predicate = pred
            elif self.default_predicate:
                newgps.go_predicate = self.default_predicate
            self.fakegpslist[newgps.slave] = newgps
            self.append(newgps)
            newgps.exhausted = 0
        self.daemon.add_device(newgps.slave)
        return newgps.slave
    def gps_remove(self, name):
        "Remove a simulated GPS from the daemon's search list."
        self.progress("gpsfake: gps_remove(%s)\n" % name)
        self.remove(self.fakegpslist[name])
        self.daemon.remove_device(name)
        del self.fakegpslist[name]
    def client_add(self, commands):
        "Initiate a client session and force connection to a fake GPS."
        self.progress("gpsfake: client_add()\n")
        newclient = gps.gps(port=self.port)
        self.append(newclient)
        newclient.id = self.client_id + 1 
        self.client_id += 1
        self.progress("gpsfake: client %d has %s\n" % (self.client_id,newclient.device))
        if commands:
            self.initialize(newclient, commands), 
        return newclient.id
    def client_query(self, id, commands):
        "Ship a command to a client channel, get a response (threaded mode only)."
        self.progress("gpsfake: client_query(%d, %s)\n" % (id, `commands`))
        for object in self.runqueue:
            if isinstance(object, gps.gps) and object.id == id:
                client.query(commands)
                return client.response
        return None
    def client_remove(self, id):
        "Terminate a client session."
        self.progress("gpsfake: client_remove(%d)\n" % id)
        for object in self.runqueue:
            if isinstance(object, gps.gps) and object.id == id:
                self.remove(object)
                return True
        else:
            return False
    def wait(self, seconds):
        "Wait, doing nothing."
        self.progress("gpsfake: wait(%d)\n" % seconds)
        time.sleep(seconds)
    def gather(self, seconds):
        "Wait, doing nothing but watching for sentences."
        self.progress("gpsfake: gather(%d)\n" % seconds)
        #mark = time.time()
        time.sleep(seconds)
        #if self.timings.c_recv_time <= mark:
        #    TestSessionError("no sentences received\n")
    def cleanup(self):
        "We're done, kill the daemon."
        self.progress("gpsfake: cleanup()\n")
        if self.daemon:
            self.daemon.kill()
            self.daemon = None
    def run(self):
        "Run the tests."
        try:
            while self.daemon:
                # We have to read anything that gpsd might have tried
                # to send to the GPS here -- under OpenBSD the
                # TIOCDRAIN will hang, otherwise.
                for device in self.runqueue:
                    if isinstance(device, FakeGPS):
                        device.read()
                had_output = False
                chosen = self.choose()
                if isinstance(chosen, FakeGPS):
                    # Delay a few seconds after a GPS source is exhauseted
                    # to remove it.  This should give its subscribers time
                    # to get gpsd's response before we call cleanup()
                    if chosen.exhausted and (time.time() - chosen.exhausted > TestSession.CLOSE_DELAY):
                        self.remove(chosen)
                        self.progress("gpsfake: GPS %s removed\n" % chosen.slave)
                    elif not chosen.go_predicate(chosen.index, chosen):
                        if chosen.exhausted == 0:
                            chosen.exhausted = time.time()
                            self.progress("gpsfake: GPS %s ran out of input\n" % chosen.slave)
                    else:
                        chosen.feed()
                elif isinstance(chosen, gps.gps):
                    if chosen.enqueued:
                        chosen.send(chosen.enqueued)
                        chosen.enqueued = ""
                    while chosen.waiting():
                        chosen.poll()
                        self.reporter(chosen.response)
                        had_output = True
                else:
                    raise TestSessionError("test object of unknown type")
                if not self.writers and not had_output:
                    break
        finally:
            self.cleanup()

    # All knowledge about locks and threading is below this line,
    # except for the bare fact that self.threadlock is set to None
    # in the class init method.

    def append(self, obj):
        "Add a producer or consumer to the object list."
        if self.threadlock:
            self.threadlock.acquire()
        self.runqueue.append(obj)
        if isinstance(obj, FakeGPS):
            self.writers += 1
        elif isinstance(obj, gps.gps):
            self.readers += 1
        if self.threadlock:
            self.threadlock.release()
    def remove(self, obj):
        "Remove a producer or consumer from the object list."
        if self.threadlock:
            self.threadlock.acquire()
        self.runqueue.remove(obj)
        if isinstance(obj, FakeGPS):
            self.writers -= 1
        elif isinstance(obj, gps.gps):
            self.readers -= 1
        self.index = min(len(self.runqueue)-1, self.index)
        if self.threadlock:
            self.threadlock.release()
    def choose(self):
        "Atomically get the next object scheduled to do something."
        if self.threadlock:
            self.threadlock.acquire()
        chosen = self.index
        self.index += 1
        self.index %= len(self.runqueue)
        if self.threadlock:
            self.threadlock.release()
        return self.runqueue[chosen]
    def initialize(self, client, commands):
        "Arrange for client to ship specified commands when it goes active."
        client.enqueued = ""
        if not self.threadlock:
            client.query(commands)
        else:
            client.enqueued = commands
    def start(self):
        self.threadlock = threading.Lock()
        threading.Thread(target=self.run)

# End
