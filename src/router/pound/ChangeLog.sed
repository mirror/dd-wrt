# Spell-fixes and other amendments to ChangeLog.  -*- fundamental -*-
# At input, each ChangeLog entry is prefixed with the commit hash followed
# by the # sign.

/^2e5e7beef935b7c68ca073f80092a6443a1b4b37#/,/^[[:xdigit:]]{40}#/s/conbinable/combinable/
/^959c0e43d4df73150e0a49e993a24ca94a28ec2f#/,/^[[:xdigit:]]{40}#/s/arguent/argument/
/^cfee6266ac3467d6e946211d143df504e4750995#/,/^[[:xdigit:]]{40}#/s/handleb/handled/
/^6a87e134c57b94ebbd83d7131422006caa082139#/,/^[[:xdigit:]]{40}#/s/gratiously/graciously/
/^4a53008f99876897a61d6569faf371b637c81b69#/,/^[[:xdigit:]]{40}#/s/stdandard/standard/
/^4c527cc78321b69d8693b60a969fa133f505f1c4#/,/^[[:xdigit:]]{40}#/s/finction/function/
/^f1161a30c5080b464247d8558c1c0b5d159d1786#/,/^[[:xdigit:]]{40}#/s/quaifiers/qualifiers/
/^e3d299b12e118dbf430a46186c26242c4d9d7d2b#/,/^[[:xdigit:]]{40}#/s/parethesized/parenthesized/
/^2536a200d897e3150447445c5ab21b23b532a276#/,/^[[:xdigit:]]{40}#/s/resonse/response/
/^892b9f76f3d23b2d999511df3a468786f08a822d#/,/^[[:xdigit:]]{40}#/s/bakend/backend/
/^eb5d7c041da62581d675f92f6ec3c68784a2e9aa#/,/^[[:xdigit:]]{40}#/s/Decreas\b/Decrease/
/^82eae3a57cf5d9696e56d91a445a776383ba943a#/,/^[[:xdigit:]]{40}#/s/testsing/testing/
/^3cd8178c85bcad723446f5ca0340f327dc86d139#/,/^[[:xdigit:]]{40}#/s/synonims/synonyms/
/^2136f919ac05792f45491a13f696d9b23736c132#/,/^[[:xdigit:]]{40}#/s/signture/signature/
/^45fb45aa690d7e2cd4ec7d7391beb67610cdb276#/,/^[[:xdigit:]]{40}#/s/woul\b/would/
/^5f586916af6d4cabadcec48d34108978c15b836f#/,/^[[:xdigit:]]{40}#/s/Assing/Assign/
/^b9bca1ba5ae0cec6d42fc31609a981fb2263e261#/,/^[[:xdigit:]]{40}#/s/dunamic/dynamic/
/^bf925f135a21055c207d2f7ec10f189f2361951a#/,/^[[:xdigit:]]{40}#/s/coconfiguration/configuration/

# Remove commit hashes.  The file must end with this rule.
s/^[[:xdigit:]]{40}#//
