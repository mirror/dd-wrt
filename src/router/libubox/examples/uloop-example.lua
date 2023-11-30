#!/usr/bin/env lua

local socket = require "socket"

local uloop = require("uloop")
uloop.init()

local udp = socket.udp()
udp:settimeout(0)
udp:setsockname('*', 8080)

-- timer example 1 (will run repeatedly)
local timer
function t()
	print("1000 ms timer run");
	timer:set(1000)
end
timer = uloop.timer(t)
timer:set(1000)

-- timer example 2 (will run once)
uloop.timer(function() print("2000 ms timer run"); end, 2000)

-- timer example 3 (will never run)
uloop.timer(function() print("3000 ms timer run"); end, 3000):cancel()

-- periodic interval timer
local intv
intv = uloop.interval(function()
	print(string.format("Interval expiration #%d - %dms until next expiration",
		intv:expirations(), intv:remaining()))

	-- after 5 expirations, lower interval to 500ms
	if intv:expirations() >= 5 then
		intv:set(500)
	end

	-- cancel after 10 expirations
	if intv:expirations() >= 10 then
		intv:cancel()
	end
end, 1000)

-- process
function p1(r)
	print("Process 1 completed")
	print(r)
end

function p2(r)
	print("Process 2 completed")
	print(r)
end

uloop.timer(
	function()
		uloop.process("uloop_pid_test.sh", {"foo", "bar"}, {"PROCESS=1"}, p1)
	end, 1000
)
uloop.timer(
	function()
		uloop.process("uloop_pid_test.sh", {"foo", "bar"}, {"PROCESS=2"}, p2)
	end, 2000
)

-- SIGINT handler
uloop.signal(function(signo)
	print(string.format("Terminating on SIGINT (#%d)!", signo))

	-- end uloop to terminate program
	uloop.cancel()
end, uloop.SIGINT)

local sig
sig = uloop.signal(function(signo)
	print(string.format("Got SIGUSR2 (#%d)!", signo))

	-- remove signal handler, next SIGUSR2 will terminate program
	sig:delete()
end, uloop.SIGUSR2)

-- Keep udp_ev reference, events will be gc'd, even if the callback is still referenced
-- .delete will manually untrack.
udp_ev = uloop.fd_add(udp, function(ufd, events)
	local words, msg_or_ip, port_or_nil = ufd:receivefrom()
	print('Recv UDP packet from '..msg_or_ip..':'..port_or_nil..' : '..words)
	if words == "Stop!" then
		udp_ev:delete()
	end
end, uloop.ULOOP_READ)

udp_count = 0
udp_send_timer = uloop.timer(
	function()
		local s = socket.udp()
		local words
		if udp_count > 3 then
			words = "Stop!"
			udp_send_timer:cancel()
		else
			words = 'Hello!'
			udp_send_timer:set(1000)
		end
		print('Send UDP packet to 127.0.0.1:8080 :'..words)
		s:sendto(words, '127.0.0.1', 8080)
		s:close()

		udp_count = udp_count + 1
	end, 3000
)

uloop.run()

