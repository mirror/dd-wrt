--------------------------------------------------
chan_dongle channel driver for Huawei UMTS cards
--------------------------------------------------

WARNING:

This channel driver is in alpha stage.
I am not responsible if this channel driver will eat your money on
your SIM card or do any unpredicted things.

Please use a recent Linux kernel, 2.6.33+ recommended.
If you use FreeBSD, 8.0+ recommended.

This channel driver should work with the folowing UMTS cards:
* Huawei K3715
* Huawei E169 / K3520
* Huawei E155X
* Huawei E175X
* Huawei K3765

Check complete list in:
http://code.google.com/p/asteris-chan-dongle/wiki/Supported_devices_eng

Before using the channel driver make sure to:

* Disable PIN code on your SIM card

Supported features:
* Place voice calls and terminate voice calls
* Send SMS and receive SMS
* Send and receive USSD commands / messages

Some useful AT commands:
AT+CCWA=0,0,1                                   #disable call-waiting
AT+CFUN=1,1                                     #reset dongle
AT^CARDLOCK="<code>"                            #unlock code
AT^SYSCFG=13,0,3FFFFFFF,0,3                     #modem 2G only, automatic search any band, no roaming
AT^U2DIAG=0                                     #enable modem function

Here is an example for the dialplan:

[dongle-incoming]
exten => sms,1,Verbose(Incoming SMS from ${CALLERID(num)} ${BASE64_DECODE(${SMS_BASE64})})
exten => sms,n,System(echo '${STRFTIME(${EPOCH},,%Y-%m-%d %H:%M:%S)} - ${DONGLENAME} - ${CALLERID(num)}: ${BASE64_DECODE(${SMS_BASE64})}' >> /var/log/asterisk/sms.txt)
exten => sms,n,Hangup()

exten => ussd,1,Verbose(Incoming USSD: ${BASE64_DECODE(${USSD_BASE64})})
exten => ussd,n,System(echo '${STRFTIME(${EPOCH},,%Y-%m-%d %H:%M:%S)} - ${DONGLENAME}: ${BASE64_DECODE(${USSD_BASE64})}' >> /var/log/asterisk/ussd.txt)
exten => ussd,n,Hangup()

exten => s,1,Dial(SIP/2001@othersipserver)
exten => s,n,Hangup()

[othersipserver-incoming]

exten => _X.,1,Dial(Dongle/r1/${EXTEN})
exten => _X.,n,Hangup

you can also use this:

Call using a specific group:
exten => _X.,1,Dial(Dongle/g1/${EXTEN})

Call using a specific group in round robin:
exten => _X.,1,Dial(Dongle/r1/${EXTEN})

Call using a specific dongle:
exten => _X.,1,Dial(Dongle/dongle0/${EXTEN})

Call using a specific provider name:
exten => _X.,1,Dial(Dongle/p:PROVIDER NAME/${EXTEN})

Call using a specific IMEI:
exten => _X.,1,Dial(Dongle/i:123456789012345/${EXTEN})

Call using a specific IMSI prefix:
exten => _X.,1,Dial(Dongle/s:25099203948/${EXTEN})

How to store your own number:

dongle cmd dongle0 AT+CPBS=\"ON\"
dongle cmd dongle0 AT+CPBW=1,\"+123456789\",145


Other CLI commands:

dongle reset <device>
dongle restart gracefully <device>
dongle restart now <device>
dongle restart when convenient <device>
dongle show device <device>
dongle show devices
dongle show version
dongle sms <device> number message
dongle ussd <device> ussd
dongle stop gracefully <device>
dongle stop now <device>
dongle stop when convenient <device>
dongle start <device>
dongle restart gracefully <device>
dongle restart now <device>
dongle restart when convenient <device>
dongle remove gracefully <device>
dongle remove now <device>
dongle remove when convenient <device>
dongle reload gracefully
dongle reload now
dongle reload when convenient

For reading installation notes please look to INSTALL file.

For additional information about Huawei dongle usage
look to chan_dongle Wiki at http://wiki.e1550.mobi
and chan_dongle project home at http://code.google.com/p/asterisk-chan-dongle/
