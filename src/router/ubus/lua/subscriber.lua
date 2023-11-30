#!/usr/bin/env lua

--[[
  A demo of ubus subscriber binding. Should be run after publisher.lua
--]]

require "ubus"
require "uloop"

uloop.init()

local conn = ubus.connect()
if not conn then
	error("Failed to connect to ubus")
end

local sub = {
	notify = function( msg, name )
		print("name:", name)
		print("  count:", msg["count"])
	end,
}

conn:subscribe( "test", sub )

uloop.run()
