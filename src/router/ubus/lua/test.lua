#!/usr/bin/env lua

require "ubus"
require "uloop"

uloop.init()

local conn = ubus.connect()
if not conn then
	error("Failed to connect to ubus")
end

local my_method = {
	broken = {
		hello = 1,
		hello1 = {
			function(req)
			end, {id = "fail" }
		},
	},
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
		deferred = {
			function(req)
				conn:reply(req, {message="wait for it"})
				local def_req = conn:defer_request(req)
				uloop.timer(function()
						conn:reply(def_req, {message="done"})
						conn:complete_deferred_request(def_req, 0)
						print("Deferred request complete")
					end, 2000)
				print("Call to function 'deferred'")
			end, {}
		}
	}
}

conn:add(my_method)

local my_event = {
	test = function(msg)
		print("Call to test event")
		for k, v in pairs(msg) do
			print("key=" .. k .. " value=" .. tostring(v))
		end
	end,
}

conn:listen(my_event)

uloop.run()
