#!/bin/bash

. ../../../prepare.inc.sh
. ../../../toolbox.inc.sh


# ---- do the actual testing ----

result=PASS
echo "++++ BEGINNING TEST" >$OUTPUTFILE

# create a pair of keyrings and attach them to the session keyring
marker "ADD KEYRING"
create_keyring wibble @s
expect_keyid keyringid

create_keyring wibble2 @s
expect_keyid keyring2id

# stick a key in the keyring
marker "ADD KEY"
create_key user lizard gizzard $keyringid
expect_keyid keyid

# check that we can list it
marker "LIST KEYRING WITH ONE"
list_keyring $keyringid
expect_keyring_rlist rlist $keyid

# search the session keyring for a non-existent key
marker "SEARCH SESSION FOR NON-EXISTENT KEY"
search_for_key --fail @s user snake
expect_error ENOKEY

# search the session keyring for the key
marker "SEARCH SESSION"
search_for_key @s user lizard
expect_keyid found $keyid

# search the session keyring for the key and attach to second keyring
marker "SEARCH SESSION AND ATTACH"
search_for_key @s user lizard $keyring2id
expect_keyid found $keyid

# check it's attached to the second keyring
marker "CHECK ATTACHED"
list_keyring $keyring2id
expect_keyring_rlist rlist $keyid

# check the key contains what we expect
marker "CHECK PAYLOAD"
print_key $keyid
expect_payload payload "gizzard"

# detach the attachment just made
marker "DETACH KEY"
unlink_key $found $keyring2id

# create an overlapping key in the second keyring
create_key user lizard skin $keyring2id
expect_keyid keyid2

# check the two keys contain what we expect
marker "CHECK PAYLOADS"
print_key $keyid
expect_payload payload "gizzard"
print_key $keyid2
expect_payload payload "skin"

# a search from the session keyring should find the first key
marker "SEARCH SESSION AGAIN"
search_for_key @s user lizard
expect_keyid found $keyid

# a search from the first keyring should find the first key
marker "SEARCH FIRST KEYRING"
search_for_key $keyringid user lizard
expect_keyid found $keyid

# a search from the second keyring should find the second key
marker "SEARCH SECOND KEYRING"
search_for_key $keyring2id user lizard
expect_keyid found $keyid2

# link the second keyring to the first
marker "LINK FIRST KEYRING TO SECOND"
link_key $keyring2id $keyringid

# a search from the first keyring should again find the first key
marker "SEARCH FIRST KEYRING AGAIN"
search_for_key $keyringid user lizard
expect_keyid found $keyid

# revoking the first key should cause the second key to be available
revoke_key $keyid
search_for_key $keyringid user lizard
expect_keyid found $keyid2

# get rid of the dead key
marker "UNLINK FIRST KEY"
unlink_key $keyid $keyringid

# a search from the first keyring should now find the second key
marker "SEARCH FIRST KEYRING AGAIN 2"
search_for_key $keyringid user lizard
expect_keyid found $keyid2

# a search from the session keyring should now find the second key
marker "SEARCH SESSION KEYRING AGAIN 2"
search_for_key @s user lizard
expect_keyid found $keyid2

# unlink the second keyring from the first
marker "UNLINK SECOND KEYRING FROM FIRST"
unlink_key $keyring2id $keyringid

# a search from the first keyring should now fail
marker "SEARCH FIRST KEYRING FOR FAIL"
search_for_key --fail $keyringid user lizard
expect_error ENOKEY

# a search from the session keyring should still find the second key
marker "SEARCH SESSION KEYRING AGAIN 3"
search_for_key @s user lizard
expect_keyid found $keyid2

# move the second keyring into the first
marker "MOVE SECOND KEYRING INTO FIRST"
link_key $keyring2id $keyringid
unlink_key $keyring2id @s

# a search from the first keyring should now find the second key once again
marker "SEARCH FIRST KEYRING AGAIN 4"
search_for_key $keyringid user lizard
expect_keyid found $keyid2

# removing search permission on the second keyring should hide the key
marker "SEARCH WITH NO-SEARCH KEYRING"
set_key_perm $keyring2id 0x370000
search_for_key --fail $keyringid user lizard
expect_error ENOKEY

# putting search permission on the second keyring back again should make it
# available again
set_key_perm $keyring2id 0x3f0000
search_for_key $keyringid user lizard
expect_keyid found $keyid2

# removing search permission on the second key should hide the key
marker "SEARCH WITH NO-SEARCH KEYRING2"
set_key_perm $keyring2id 0x370000
search_for_key --fail $keyringid user lizard
expect_error ENOKEY

# putting search permission on the second key back again should make it
# available again
set_key_perm $keyring2id 0x3f0000
search_for_key $keyringid user lizard
expect_keyid found $keyid2

# revoking the key should make the key unavailable
revoke_key $keyid2
search_for_key --fail $keyringid user lizard
kver=`uname -r`
case $kver in
    *.el7*)
	expect_error EKEYREVOKED
	;;
    *)
	if kernel_older_than 3.13
	then
	    expect_error ENOKEY
	else
	    expect_error EKEYREVOKED
	fi
	;;
esac

# remove the keyrings we added
marker "UNLINK KEYRING"
unlink_key $keyringid @s

echo "++++ FINISHED TEST: $result" >>$OUTPUTFILE

# --- then report the results in the database ---
toolbox_report_result $TEST $result
