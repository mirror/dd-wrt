# DAWN Installation
This guide should allow you to get DAWN running, and check its basic
behaviour.

## Setting up Routers
DAWN will typically run in a network where one AP is providing core
networking services like DHCP and internet gateway, while other APs are
being used to extend wifi network range.  For example, the "main" AP near
the telecoms provider entry point, and one or more other AP on the other
side of the house, in the garden, etc.  Or several AP distributed about an
office building.

The term "dumb AP" is typically used for the extending APs, meaning that
they don't directly provide those DHCP type services.  If adding APs for
the first time as a part of the plan to make DAWN work then it's important
to have them configured correctly, and you can find a good guide to that
[here](https://gist.github.com/braian87b/bba9da3a7ac23c35b7f1eecafecdd47d).

Once those basics are in place DAWN is installed on each of the APs
("main" and "dumb") so that a full view of the network can be gathered.

## Installing DAWN
### Install a full version of WPAD
Use the local package manager to install a full version of wpad, for
example on OpenWrt:

    opkg update
    opkg remove wpad-basic wpad-mini
    # Use one of the following, or similar
    opkg install wpad-wolfssl
    opkg install wpad-openssl

Once it is installed you should be able to see the ubus methods that DAWN
uses to ask hostapd to tell clients to do things.

    ubus -v list hostapd.wlan0

You should see at least the first of these two entires in the result (note
that the parameter lists have been removed to make it clear what method names
should be there):

     "wnm_disassoc_imminent":{...}
     "bss_transition_request":{...}

`wnm_disassoc_imminent` is used by the RSSI Comparison algorithm, while
`bss_transition_request` is used by the Absolute RSSI algorihm.  If you don't
see the relevant method then the respective algorithm will not work. See the
Configuration page for furhter details on what this means.

### Install DAWN
The published version of DAWN should install easily:

    opkg install dawn

### Developers
If you want to install versions of DAWN that are not fully packaged yet or
to work on a fork of the code yourself then see the [Developer
Guide](DEVELOPER.md).

## Configuring DAWN

Review the configuration guide to see what you might want to change
[configuration guide](CONFIGURE.md).  At a minimum:

- Be sure to enable the DAWN functions that you want (at least "kicking")

## Check That DAWN Is Running
By default, DAWN won't send much to the system log, so it can be hard to
see if it is working at all.

If you have an 802.11k/v enabled device then if you place it close to one
AP until it connects, and then move it close to another you should see a
message indicating that DAWN is asking the device to move to
the other AP:

    logread -f | grep dawn
    <Message here that shows transfer>

If you'd like to see more messages to help confirm DAWN is interacting
with the relevant services then edit the config file to set the messaging
level to '1'.

## See What DAWN Is Seeing
Once it has been running for a few minutes you can query the running DAWN
instance via UBUS to understand what view of the network it has
constructed.

To get an overview of all connected Clients sorted by the SSID, then AP
BSSID, then device MAC:

    root@OpenWrt:~# ubus call dawn get_network
    {
	    "Free-Cookies": { <-- This SSID...
		    "00:27:19:XX:XX:XX": { <-- ...is provided by this AP BSSID...
			    "78:02:F8:XX:XX:XX": { <-- ...and has these client MACs
				    "freq": 2452,
				    "ht": 1,
				    "vht": 0,
				    "collision_count": 4
			    }
		    },
		    "A4:2B:B0:XX:XX:XX": { <-- The SSID is also provided by this BSSID
			    "48:27:EA:XX:XX:XX: {
				    "freq": 2412,
				    "ht": 1,
				    "vht": 0,
				    "collision_count": 4
			    },
		    }
	    },
	    "Free-Cookies_5G": { <-- Alternate SSIDS may also be offered
    		
	    }
    }

To get the view of which AP each client can see you can use the following.
Note that DAWN will try to move a clent between APs when the difference
between 'score' value for the current AP and an alternative exceeds the
kicking_threshold parameter.

    root@OpenWrt:~# ubus call dawn get_hearing_map
    {
	    "Free-Cookies": {
		    "0E:5B:DB:XX:XX:XX": { <--This is the client device MAC
			    "00:27:19:XX:XX:XX": { <-- This is AP1...
				    "signal": -64,
				    "freq": 2452,
				    "ht_support": true,
				    "vht_support": false,
				    "channel_utilization": 12,
				    "num_sta": 1,
				    "ht": 1,
				    "vht": 0,
				    "score": 10 <-- These scores are compared
			    },
			    "A4:2B:B0:XX:XX:XX": { <-- This is AP2...
				    "signal": -70,
				    "freq": 2412,
				    "ht_support": true,
				    "vht_support": false,
				    "channel_utilization": 71,
				    "num_sta": 3,
				    "ht": 1,
				    "vht": 0,
				    "score": 10
			    }
		    }
	    }
    }
