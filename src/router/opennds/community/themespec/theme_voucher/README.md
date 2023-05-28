# Vouchers ThemeSpec
This ThemeSpec provides a simple portal requiring a voucher to login.

Vouchers can be generated according to voucher specifications in any way, either manually or programmatically.

An example voucher file is provided, along with a python script to generate more.

# Installation (openWRT)
**Copy the themespec (theme_voucher.sh) file and the voucher.txt file.**

SSH into a terminal session on the router and use the following commands:

       cd /usr/lib/opennds
       wget https://raw.githubusercontent.com/openNDS/openNDS/master/community/themespec/theme_voucher/theme_voucher.sh
       cd /tmp/ndslog
       wget https://raw.githubusercontent.com/openNDS/openNDS/master/community/themespec/theme_voucher/vouchers.txt

**Now you need to activate the voucher script.**

Use the following commands:

       chmod 744 /usr/lib/opennds/theme_voucher.sh
       uci set opennds.@opennds[0].login_option_enabled='3'
       uci set opennds.@opennds[0].themespec_path='/usr/lib/opennds/theme_voucher.sh'
       uci commit opennds
       service opennds restart

OpenNDS should now be running the voucher script.

Test it by connecting again with your phone or tablet.
You will need a voucher code from the example vouchers.txt file you downloaded.

**There is also a voucher roll generator.**

You can run this on your computer or even on the router (but you will need python installed wherever you run it).

Download it like this:

       wget https://raw.githubusercontent.com/openNDS/openNDS/master/community/themespec/theme_voucher/voucher_generator.py

The generator is a very simple program and could easily be re-written in any programming language.

## Voucher Roll
The vouchers are contained in a "voucher roll", the path to which is defined in the themespec file and on OpenWrt defaults to `/tmp/ndslog/vouchers.txt`.

It should be changed to an external storage medium. (See the last section of the `theme_voucher.sh` file, entitled "Customise the Logfile location").

### Flash Wearout Notice
**WARNING**

 * The voucher roll is written to on every login
 * If its location is on router flash, this **WILL** result in non-repairable failure of the flash memory and therefore the router itself.
 * Failure will happen, most likely within several months depending on the number of logins.
 * A safe location is set by default to be the same location as the openNDS log (logdir) ie on the tmpfs (ramdisk) of the operating system.
 * Files stored here will not survive a reboot.
 * In a production system, the mountpoint for logdir should be changed to the mount point of some external storage
eg a usb stick, an external drive, a network shared drive etc.

### Voucher Specifications
File MUST be:

* CSV style table, with comma (",") separators. No headers.
* 7 Columns: voucher code, speed limit down, speed limit up, quota down, quota up, voucher validity (minutes), 0 (placeholder for when voucher is used)

The Voucher-Code MUST respect the following:

* 9 characters
* alphanumeric or dash ("-") character (eg. 12345abcd, 1234-abcd, abcd-efgh)

Each Voucher entry will be on a separate line in the vouchers.txt file
