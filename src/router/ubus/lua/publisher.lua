#!/usr/bin/env lua

require "ubus"
require "uloop"

--[[
  A demo of ubus publisher binding. Should be run before subscriber.lua
--]]


uloop.init()

local conn = ubus.connect()
if not conn then
	error("Failed to connect to ubus")
end

local ubus_objects = {
	test = {
		hello = {
			function(req, msg)
				conn:reply(req, {message="foo"});
				print("Call to function 'hello'")
				for k, v in pairs(msg) do
					print("key=" .. k .. " value=" .. tostring(v))
				end
			end, {id = ubus.INT32, msg = ubus.STRING }
		},
		hello1 = {
			function(req)
				conn:reply(req, {message="foo1"});
				conn:reply(req, {message="foo2"});
				print("Call to function 'hello1'")
			end, {id = ubus.INT32, msg = ubus.STRING }
		},
		__subscriber_cb = function( subs )
			print("total subs: ", subs )
		end
	}
}

conn:add( ubus_objects )
print("Objects added, starting loop")

-- start time
local timer
local counter = 0
function t()
	counter = counter + 1
	local params = {
		count = counter
	}
	conn:notify( ubus_objects.test.__ubusobj, "test.alarm", params )
	timer:set(10000)
end
timer = uloop.timer(t)
timer:set(1000)


uloop.run()
