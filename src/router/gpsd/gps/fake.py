# This file is Copyright (c) 2010 by the GPSD project
# BSD terms apply: see the file COPYING in the distribution root for details.
"""
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
allowing a specified number of seconds to elapse.  TestSession.send()
ships commands to an open client session.

TestSession does not currently capture the daemon's log output.  It is
run with -N, so the output will go to stderr (along with, for example,
Valgrind notifications).

Each FakeGPS instance tries to packetize the data from the logfile it
is initialized with. It uses the same packet-getter as the daeomon.

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
import os, time, signal, pty, termios # fcntl, array, struct
import exceptions, threading, socket
import gps
import packet as sniffer

# The two magic numbers below have to be derived from observation.  If
# they're too high you'll slow the tests down a lot.  If they're too low
# you'll get random spurious regression failures that usually look
# like lines missing from the end of the test output relative to the
# check file.  These numbers might have to be adjusted upward on faster
# machines.  The need for them may be symptomatic of race conditions
# in the pty layer or elsewhere.

# Define a per-line delay on writes so we won't spam the buffers in
# the pty layer or gpsd itself.  Removing this entirely was tried but
# caused failures under NetBSD.  Values smaller than the system timer
# tick don't make any difference here.
WRITE_PAD = 0.001

# We delay briefly after a GPS source is exhausted before removing it.
# This should give its subscribers time to get gpsd's response before
# we call the cleanup code. Note that using fractional seconds in
# CLOSE_DELAY may have no effect; Python time.time() returns a float
# value, but it is not guaranteed by Python that the C implementation
# underneath will return with precision finer than 1 second. (Linux
# and *BSD return full precision.) Dropping this to 0.1 has been
# tried but caused failures.
CLOSE_DELAY = 0.2

class TestLoadError(exceptions.Exception):
    def __init__(self, msg):
        self.msg = msg

class TestLoad:
    "Digest a logfile into a list of sentences we can cycle through."
    def __init__(self, logfp, predump=False):
        self.sentences = []	# This is the interesting part
        if type(logfp) == type(""):
            logfp = open(logfp, "r")            
        self.name = logfp.name
        self.logfp = logfp
        self.predump = predump
        self.logfile = logfp.name
        self.type = None
        self.sourcetype = "pty"
        self.serial = None
        # Grab the packets
        getter = sniffer.new()
        #gps.packet.register_report(reporter)
        type_latch = None
        while True:
            (plen, ptype, packet, counter) = getter.get(logfp.fileno())
            if plen <= 0:
                break
            elif ptype == sniffer.COMMENT_PACKET:
                # Some comments are magic
                if "Serial:" in packet:
                    # Change serial parameters
                    packet = packet[1:].strip()
                    try:
                        (xx, baud, params) = packet.split()
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
                    except (ValueError, IndexError):
                        raise TestLoadError("bad serial-parameter spec in %s"%\
                                            logfp.name)                    
                    self.serial = (baud, databits, parity, stopbits)
                elif "UDP" in packet:
                    self.sourcetype = "UDP"
            else:
                if type_latch is None:
                    type_latch = ptype
                if self.predump:
                    print repr(packet)
                if not packet:
                    raise TestLoadError("zero-length packet from %s"%\
                                        logfp.name)                    
                self.sentences.append(packet)
        # Look at the first packet to grok the GPS type
        self.textual = (type_latch == sniffer.NMEA_PACKET)
        if self.textual:
            self.legend = "gpsfake: line %d: "
        else:
            self.legend = "gpsfake: packet %d"

class PacketError(exceptions.Exception):
    def __init__(self, msg):
        self.msg = msg

class FakeGPS:
    def __init__(self, testload, progress=None):
        self.testload = testload
        self.progress = progress
        self.go_predicate = lambda: True
        self.readers = 0
        self.index = 0
        self.progress("gpsfake: %s provides %d sentences\n" % (self.testload.name, len(self.testload.sentences)))

    def write(self, line):
        "Throw an error if this superclass is ever instantiated."
        raise ValueError, line

    def feed(self):
        "Feed a line from the contents of the GPS log to the daemon."
        line = self.testload.sentences[self.index % len(self.testload.sentences)]
        if "%Delay:" in line:
            # Delay specified number of seconds
            delay = line.split()[1]
            time.sleep(int(delay))
        # self.write has to be set by the derived class
        self.write(line)
        if self.progress:
            self.progress("gpsfake: %s feeds %d=%s\n" % (self.testload.name, len(line), repr(line)))
        time.sleep(WRITE_PAD)
        self.index += 1

class FakePTY(FakeGPS):
    "A FakePTY is a pty with a test log ready to be cycled to it."
    def __init__(self, testload,
                 speed=4800, databits=8, parity='N', stopbits=1,
                 progress=None):
        FakeGPS.__init__(self, testload, progress)
        # Allow Serial: header to be overridden by explicit spped.
        if self.testload.serial:
            (speed, databits, parity, stopbits) = self.testload.serial
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
        (self.fd, self.slave_fd) = pty.openpty()
        self.byname = os.ttyname(self.slave_fd)
        (iflag, oflag, cflag, lflag, ispeed, ospeed, cc) = termios.tcgetattr(self.slave_fd)
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
        termios.tcsetattr(self.slave_fd, termios.TCSANOW,
                          [iflag, oflag, cflag, lflag, ispeed, ospeed, cc])
    def read(self):
        "Discard control strings written by gpsd."
        # A tcflush implementation works on Linux but fails on OpenBSD 4.
        termios.tcflush(self.fd, termios.TCIFLUSH)
        # Alas, the FIONREAD version also works on Linux and fails on OpenBSD.
        #try:
        #    buf = array.array('i', [0])
        #    fcntl.ioctl(self.master_fd, termios.FIONREAD, buf, True)
        #    n = struct.unpack('i', buf)[0]
        #    os.read(self.master_fd, n)
        #except IOError:
        #    pass

    def write(self, line):
        os.write(self.fd, line)

    def drain(self):
        "Wait for the associated device to drain (e.g. before closing)."
        termios.tcdrain(self.fd)

class FakeUDP(FakeGPS):
    "A UDP broadcaster with a test log ready to be cycled to it."
    def __init__(self, testload,
                 ipaddr, port,
                 progress=None):
        FakeGPS.__init__(self, testload, progress)
        self.ipaddr = ipaddr
        self.port = port
        self.byname = "udp://" + ipaddr + ":" + port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def read(self):
        "Discard control strings written by gpsd."
        pass

    def write(self, line):
        self.sock.sendto(line, (self.ipaddr, int(self.port)))

    def drain(self):
        "Wait for the associated device to drain (e.g. before closing)."
        pass	# shutdown() fails on UDP

class DaemonError(exceptions.Exception):
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return repr(self.msg)

class DaemonInstance:
    "Control a gpsd instance."
    def __init__(self, control_socket=None):
        self.sockfile = None
        self.pid = None
        self.tmpdir = os.environ.get('TMPDIR', '/tmp')
        if control_socket:
            self.control_socket = control_socket
        else:
            self.control_socket = "%s/gpsfake-%d.sock" % (self.tmpdir, os.getpid())
        self.pidfile = "%s/gpsfake-%d.pid" % (self.tmpdir, os.getpid())
    def spawn(self, options, port, background=False, prefix=""):
        "Spawn a daemon instance."
        self.spawncmd = None

	# Look for gpsd in GPSD_HOME env variable
        if os.environ.get('GPSD_HOME'):
            for path in os.environ['GPSD_HOME'].split(':'):
                _spawncmd = "%s/gpsd" % path
                if os.path.isfile(_spawncmd) and os.access(_spawncmd, os.X_OK):
                    self.spawncmd = _spawncmd
                    break

	# if we could not find it yet try PATH env variable for it
        if not self.spawncmd:
            if not '/usr/sbin' in os.environ['PATH']:
                os.environ['PATH']=os.environ['PATH'] + ":/usr/sbin"
            for path in os.environ['PATH'].split(':'):
                _spawncmd = "%s/gpsd" % path
                if os.path.isfile(_spawncmd) and os.access(_spawncmd, os.X_OK):
                    self.spawncmd = _spawncmd
                    break

        if not self.spawncmd:
            raise DaemonError("Cannot execute gpsd: executable not found. Set GPSD_HOME env variable")
        # The -b option to suppress hanging on probe returns is needed to cope
        # with OpenBSD (and possibly other non-Linux systems) that don't support
        # anything we can use to implement the FakeGPS.read() method
        self.spawncmd += " -b -N -S %s -F %s -P %s %s" % (port, self.control_socket, self.pidfile, options)
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
                time.sleep(0.1)
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
            os.kill(self.pid, 0)
            return True
        except OSError:
            return False
    def add_device(self, path):
        "Add a device to the daemon's internal search list."
        if self.__get_control_socket():
            self.sock.sendall("+%s\r\n\x00" % path)
            self.sock.recv(12)
            self.sock.close()
    def remove_device(self, path):
        "Remove a device from the daemon's internal search list."
        if self.__get_control_socket():
            self.sock.sendall("-%s\r\n\x00" % path)
            self.sock.recv(12)
            self.sock.close()
    def kill(self):
        "Kill the daemon instance."
        if self.pid:
            try:
                os.kill(self.pid, signal.SIGTERM)
                # Raises an OSError for ESRCH when we've killed it.
                while True:
                    os.kill(self.pid, signal.SIGTERM)
                    time.sleep(0.01)
            except OSError:
                pass
            self.pid = None

class TestSessionError(exceptions.Exception):
    def __init__(self, msg):
        self.msg = msg

class TestSession:
    "Manage a session including a daemon with fake GPSes and clients."
    def __init__(self, prefix=None, port=None, options=None, verbose=0, predump=False, udp=False):
        "Initialize the test session by launching the daemon."
        self.prefix = prefix
        self.port = port
        self.options = options
        self.verbose = verbose
        self.predump = predump
        self.udp = udp
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
        self.default_predicate = None
        self.fd_set = []
        self.threadlock = None
    def spawn(self):
        for sig in (signal.SIGQUIT, signal.SIGINT, signal.SIGTERM):
            signal.signal(sig, lambda unused, dummy: self.cleanup())
        self.daemon.spawn(background=True, prefix=self.prefix, port=self.port, options=self.options)
        self.daemon.wait_pid()
    def set_predicate(self, pred):
        "Set a default go predicate for the session."
        self.default_predicate = pred
    def gps_add(self, logfile, speed=19200, pred=None):
        "Add a simulated GPS being fed by the specified logfile."
        self.progress("gpsfake: gps_add(%s, %d)\n" % (logfile, speed))
        if logfile not in self.fakegpslist:
            testload = TestLoad(logfile, predump=self.predump)
            if testload.sourcetype == "UDP" or self.udp:
                newgps = FakeUDP(testload, ipaddr="127.0.0.1", port="5000",
                                   progress=self.progress)
            else:
                newgps = FakePTY(testload, speed=speed, 
                                   progress=self.progress)
            if pred:
                newgps.go_predicate = pred
            elif self.default_predicate:
                newgps.go_predicate = self.default_predicate
            self.fakegpslist[newgps.byname] = newgps
            self.append(newgps)
            newgps.exhausted = 0
        self.daemon.add_device(newgps.byname)
        return newgps.byname
    def gps_remove(self, name):
        "Remove a simulated GPS from the daemon's search list."
        self.progress("gpsfake: gps_remove(%s)\n" % name)
        self.fakegpslist[name].drain()
        self.remove(self.fakegpslist[name])
        self.daemon.remove_device(name)
        del self.fakegpslist[name]
    def client_add(self, commands):
        "Initiate a client session and force connection to a fake GPS."
        self.progress("gpsfake: client_add()\n")
        newclient = gps.gps(port=self.port, verbose=self.verbose)
        self.append(newclient)
        newclient.id = self.client_id + 1 
        self.client_id += 1
        self.progress("gpsfake: client %d has %s\n" % (self.client_id,newclient.device))
        if commands:
            self.initialize(newclient, commands) 
        return self.client_id
    def client_remove(self, cid):
        "Terminate a client session."
        self.progress("gpsfake: client_remove(%d)\n" % cid)
        for obj in self.runqueue:
            if isinstance(obj, gps.gps) and obj.id == cid:
                self.remove(obj)
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
    def cleanup(self):
        "We're done, kill the daemon."
        self.progress("gpsfake: cleanup()\n")
        if self.daemon:
            self.daemon.kill()
            self.daemon = None
    def run(self):
        "Run the tests."
        try:
            self.progress("gpsfake: test loop begins\n")
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
                    if chosen.exhausted and (time.time() - chosen.exhausted > CLOSE_DELAY):
                        self.gps_remove(chosen.byname)
                        self.progress("gpsfake: GPS %s removed\n" % chosen.byname)
                    elif not chosen.go_predicate(chosen.index, chosen):
                        if chosen.exhausted == 0:
                            chosen.exhausted = time.time()
                            self.progress("gpsfake: GPS %s ran out of input\n" % chosen.byname)
                    else:
                        chosen.feed()
                elif isinstance(chosen, gps.gps):
                    if chosen.enqueued:
                        chosen.send(chosen.enqueued)
                        chosen.enqueued = ""
                    while chosen.waiting():
                        chosen.read()
                        if chosen.valid & gps.PACKET_SET:
                            self.reporter(chosen.response)
                        had_output = True
                else:
                    raise TestSessionError("test object of unknown type")
                if not self.writers and not had_output:
                    self.progress("gpsfake: no writers and no output\n")
                    break
            self.progress("gpsfake: test loop ends\n")
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
            client.send(commands)
        else:
            client.enqueued = commands
    def start(self):
        self.threadlock = threading.Lock()
        threading.Thread(target=self.run)

# End
