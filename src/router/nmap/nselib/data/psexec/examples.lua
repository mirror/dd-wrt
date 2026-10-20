---This configuration file contains the examples given in smb-psexec.nse.

-- Any variable in the 'config' table in smb-psexec.nse can be overriden in the
-- 'overrides' table. Most of them are not really recommended, such as the host,
-- key, etc.
overrides = {}
overrides.timeout = 40

modules = {

  {
    upload           = false,
    name             = "Membership of 'administrators' from 'net localgroup administrators'",
    program          = "net.exe",
    args             = "localgroup administrators",
  },

  {
    upload           = false,
    name             = "Example 2: Membership of 'administrators', cleaned",
    program          = "net.exe",
    args             = "localgroup administrators",
    remove           = {"The command completed", "%-%-%-%-%-%-%-%-%-%-%-", "Members", "Alias name", "Comment"},
    noblank          = true,
  },

  {
    upload           = false,
    name             = "Example 3: IP Address and MAC Address",
    program          = "ipconfig.exe",
    args             = "/all",
    maxtime          = 1,
    find             = {"IP Address", "Physical Address", "Ethernet adapter"},
    replace          = {{"%. ", ""}, {"-", ":"}, {"Physical Address", "MAC Address"}},
  },

  {
    upload           = false,
    name             = "Example 4: Can the host ping our address?",
    program          = "ping.exe",
    args             = "$lhost",
    remove           = {"statistics", "Packet", "Approximate", "Minimum"},
    noblank          = true,
    env              = "SystemRoot=c:\\WINDOWS",
  },

  {
    upload           = false,
    name             = "Example 5: Can the host ping $host?",
    program          = "ping.exe",
    args             = "$host",
    remove           = {"statistics", "Packet", "Approximate", "Minimum"},
    noblank          = true,
    env              = "SystemRoot=c:\\WINDOWS",
    req_args         = {'host'},
  },

  {
    upload           = true,
    name             = "Example 6: FgDump",
    program          = "fgdump.exe",
    args             = "-c -l fgdump.log",
    url              = "http://www.foofus.net/fizzgig/fgdump/",
    tempfiles        = {"fgdump.log"},
    outfile          = "127.0.0.1.pwdump",
  },
}
