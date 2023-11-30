#!/usr/bin/env lua

require "ubus"
require "uloop"

uloop.init()

local conn = ubus.connect()
if not conn then
	error("Failed to connect to ubusd")
end

local namespaces = conn:objects()
for i, n in ipairs(namespaces) do
	print("namespace=" .. n)
	local signatures = conn:signatures(n)
	for p, s in pairs(signatures) do
		print("\tprocedure=" .. p)
		for k, v in pairs(s) do
			print("\t\tattribute=" .. k .. " type=" .. v)
		end
	end
end

local status = conn:call("test", "hello", { msg = "eth0" })

for k, v in pairs(status) do
	print("key=" .. k .. " value=" .. tostring(v))
end

local status = {conn:call("test", "hello1", { msg = "eth0" })}

for a = 1, #status do
	for k, v in pairs(status[a]) do
		print("key=" .. k .. " value=" .. tostring(v))
	end
end

conn:send("test", { foo = "bar"})

uloop.run()
