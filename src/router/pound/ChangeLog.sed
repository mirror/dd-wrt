# Spell-fixes and other amendments to ChangeLog.  -*- fundamental -*-
# At input, each ChangeLog entry is prefixed with the commit hash followed
# by the # sign.

/^892b9f76f3d23b2d999511df3a468786f08a822d#/,/^[[:xdigit:]]{40}#/s/bakend/backend/
/^eb5d7c041da62581d675f92f6ec3c68784a2e9aa#/,/^[[:xdigit:]]{40}#/s/Decreas\b/Decrease/
/^82eae3a57cf5d9696e56d91a445a776383ba943a#/,/^[[:xdigit:]]{40}#/s/testsing/testing/

# Remove commit hashes.  The file must end with this rule.
s/^[[:xdigit:]]{40}#//
