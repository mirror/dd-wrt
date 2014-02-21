# $Id: 100_simple.py 369517 2012-07-01 17:28:57Z file $
import inc_sip as sip
import inc_sdp as sdp

pjsua = "--null-audio --id=sip:CLIENT --registrar sip:127.0.0.1:$PORT " + \
	"--auto-update-nat=0"

req1 = sip.RecvfromTransaction("Registration", 200,
				include=["REGISTER sip"], 
				exclude=["Authorization"],
				resp_hdr=["Server: Snake Registrar", "Expires: 221", "Contact: sip:localhost"],
				expect="registration success"
			  )

recvfrom_cfg = sip.RecvfromCfg("Simple registration test",
			       pjsua, [req1])
