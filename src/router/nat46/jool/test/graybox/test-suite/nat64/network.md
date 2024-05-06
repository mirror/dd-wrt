# NAT64 Network

This is the network used by all the current NAT64 Graybox tests.

It can be temporarily created by running

	../namespace-create.sh
	./setup.sh

Improvise a test run with

	./test.sh

To clean up:

	./end.sh
	../namespace-destroy.sh

## Diagram

	+----+
	| n6 |
	+----+
	  | ::5
	  |
	  | 2001:db8::/96
	  |
	  | ::1
	+----+
	| j  |
	+----+
	  | .1
	  |
	  | 192.0.2/24
	  |
	  | .5
	+----+
	| n4 |
	+----+

All tests are packet exchanges between `n6` and `n4`, or by `n6` and `n6`, via `j`.

## Configuration

	n6
		Addresses:
			2001:db8::5/96
		Routes:
			64:ff9b::/96 via j

	j
		Addresses:
			2001:db8::1/96
			192.0.2.2/24
		Routes:
			203.0.113.0/24 via n4
		pool6:
			64:ff9b::/96
		pool4:
			192.0.2.2 1-3000 (TCP, UDP, ICMP)
		BIB:
			192.0.2.2#2000  2001:db8::5#2000    (TCP, UDP)
			192.0.2.2#1     2001:db8::5#1       (ICMP)
			192.0.2.2#1000  2001:db8:1::5#1001  (UDP)
			192.0.2.2#1002  2001:db8::5#1003    (UDP) (commented out)

	n4
		Addresses:
			192.0.2.5/24
		Routes:
			-

## Quick Interactions

Easy ping from n6 to n4:

	sudo ip netns exec client6ns ping6 64:ff9b::192.0.2.5

Netcat server in n4:

	sudo ip netns exec client4ns nc -ls 192.0.2.5 -p 1234

Netcat client from n6:

	sudo ip netns exec client6ns nc 64:ff9b::192.0.2.5 1234

