from k5test import *

# Set the maximum UDP reply size very low, so that all replies go
# through the RESPONSE_TOO_BIG path.
kdc_conf = {'kdcdefaults': {'kdc_max_dgram_reply_size': '10'}}
realm = K5Realm(kdc_conf=kdc_conf, get_creds=False)

msgs = ('Sending initial UDP request',
        'Received answer',
        'Request or response is too big for UDP; retrying with TCP',
        ' to KRBTEST.COM (tcp only)',
        'Initiating TCP connection',
        'Sending TCP request',
        'Terminating TCP connection')
realm.kinit(realm.user_princ, password('user'), expected_trace=msgs)
realm.run([kvno, realm.host_princ], expected_trace=msgs)

success('Large KDC replies')
