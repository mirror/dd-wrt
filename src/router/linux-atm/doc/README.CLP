Setting the CLP (Cell Loss Priority) bit
========================================

At the socket API: Enabled via the socket option SO_SETCLP (in SOL_ATM).
Default is off.

Above the driver interface: before calling dev->ops->send on an skb,
vcc->atm_options must be copied into skb->atm.atm_options. This is
similar to the initialization of skb->atm.iovcnt.

At the driver interface: Bit ATM_ATMOPT_CLP is set in skb->atm.atm_options
at the time dev->ops->send is called.

A driver is not required to honor ATM_ATMOPT_CLP.

ATM_ATMOPT_CLP may only be applied to AAL5 at the moment. It may or may
not work for AAL0. (In the future, attempting to set SO_SETCLP for AAL0
may yield an error.)

Drivers currently supporting CLP: ENI and ZATM.

Note that CLP support is completely untested yet.
