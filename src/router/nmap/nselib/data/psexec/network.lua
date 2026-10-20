---More verbose network scripts

-- Any variable in the 'config' table in smb-psexec.nse can be overriden in the
-- 'overrides' table. Most of them are not really recommended, such as the host,
-- key, etc.
overrides = {}
--overrides.timeout = 40

modules = {

  -- Grab the ip and mac address(es) from ipconfig. The output requires quite a bit of cleanup
  -- to end up being usable and pretty.
  {
    upload           = false,
    name             = "IP Address and MAC Address from 'ipconfig.exe'",
    program          = "ipconfig.exe",
    args             = "/all",
    maxtime          = 1,
    find             = {"IP Address", "Physical Address", "Ethernet adapter"},
    replace          = {{"%. ", ""}, {"-", ":"}, {"Physical Address", "MAC Address"}},
  },

  -- Dump the arp cache of the system.
  {
    name             = "ARP Cache from arp.exe",
    program          = 'arp.exe',
    upload           = false,
    args             = '-a',
    remove           = "Interface",
    noblank          = true,
  },

  -- Get the listening/connected ports
  {
    upload           = false,
    name             = "List of listening and established connections (netstat -an)",
    program          = "netstat",
    args             = "-anb",
    maxtime          = 1,
    remove           = {"Active"},
    noblank          = true,
    env              = "SystemRoot=c:\\WINDOWS",
  },

  -- Get the routing table.
  --
  -- Like 'ver', this has to be run through cmd.exe. This also requires the 'PATH' variable to be
  -- set properly, so it isn't going to work against systems with odd paths.
  {
    upload           = false,
    name             = "Full routing table from 'netstat -nr'",
    program          = "cmd.exe",
    args             = "/c \"netstat -nr\"",
    env              = "PATH=C:\\WINDOWS\\system32;C:\\WINDOWS;C:\\WINNT;C:\\WINNT\\system32",
    maxtime          = 1,
    noblank          = true,
  },

  -- Try and ping back to our host. This helps check if there's a firewall in the way for connecting backwards.
  -- Interestingly, in my tests against Windows 2003, ping gives weird output (but still, more or less, worked)
  -- when the SystemRoot environmental variable wasn't set.
  {
    upload           = false,
    name             = "Can the host ping our address?",
    program          = "ping",
    args             = "-n 1 $lhost",
    maxtime          = 5,
    remove           = {"statistics", "Packet", "Approximate", "Minimum"},
    noblank          = true,
    env              = "SystemRoot=c:\\WINDOWS",
  },

  -- Try a traceroute back to our host. I limited it to the first 5 hops in the interest of saving time.
  -- Like ping, if the SystemRoot variable isn't set, the output is a bit strange (but still works)
  {
    upload           = false,
    name             = "Traceroute back to the scanner",
    program          = "tracert",
    args             = "-d -h 5 $lhost",
    maxtime          = 20,
    remove           = {"Tracing route", "Trace complete"},
    noblank          = true,
    env              = "SystemRoot=c:\\WINDOWS",
  },

  -- Ping an arbitrary address given by the user
  {
    upload           = false,
    name             = "Can the host ping $address?",
    program          = "ping",
    args             = "-n 1 $address",
    req_args         = {'address'},
    maxtime          = 5,
    remove           = {"statistics", "Packet", "Approximate", "Minimum"},
    noblank          = true,
    env              = "SystemRoot=c:\\WINDOWS",
  },

  -- Try a traceroute to an address given by the user
  {
    upload           = false,
    name             = "Traceroute to $address (5 hops or less)",
    program          = "tracert",
    args             = "-d -h 5 $address",
    req_args         = {'address'},
    maxtime          = 20,
    remove           = {"Tracing route", "Trace complete"},
    noblank          = true,
    env              = "SystemRoot=c:\\WINDOWS",
  },
}
