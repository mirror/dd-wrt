---This configuration file pulls info about a given harddrive

-- Any variable in the 'config' table in smb-psexec.nse can be overriden in the
-- 'overrides' table. Most of them are not really recommended, such as the host,
-- key, etc.
overrides = {}
--overrides.timeout = 40

modules = {
  {
    upload           = false,
    name             = "Drive type",
    program          = "fsutil",
    args             = "fsinfo drivetype $drive",
    req_args         = {"drive"},
    maxtime          = 1,
  },

  {
    upload           = false,
    name             = "Drive info",
    program          = "fsutil",
    args             = "fsinfo ntfsinfo $drive",
    req_args         = {"drive"},
    replace          = {{" :",":"}},
    maxtime          = 1,
  },

  {
    upload           = false,
    name             = "Drive type",
    program          = "fsutil",
    args             = "fsinfo statistics $drive",
    req_args         = {"drive"},
    replace          = {{" :",":"}},
    maxtime          = 1,
  },

  {
    upload           = false,
    name             = "Drive quota",
    program          = "fsutil",
    args             = "quota query $drive",
    req_args         = {"drive"},
    maxtime          = 1,
  },
}
