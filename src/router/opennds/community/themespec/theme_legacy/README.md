# Legacy ThemeSpec
This ThemeSpec enables the legacy splash.html portal page from a Themespec script.

Legacy splash pages used the long deprecated NoDogSplash template system. The code supporting this was removed starting with openNDS version 9.0.0.

Only gatewayname, tok, redir and authaction variables from the original template system are supported.

# WARNING!!
If you use the unedited legacy splash.html file, the client is NOT required to accept a terms of service statement and does not receive a privacy policy notice.

Before using at a public venue, you should edit the file to add a Terms of Service statement and a Privacy Policy notice that comply with local laws and regulations.

Failure to do this may make you personally liable for misuse of the Internet connection. Use unchanged at your own risk at public venues.

For simplicity, it is recommended that the default theme_click-to-continue script is used instead, as this contains default Terms of Service and Privacy policy. The html content of the ThemeSpec splash page sequence can be edited within the script itself.

# Installation (openWRT)
**Copy the themespec (theme_click-to-continue-legacy.sh) file and the splash.html file.**

SSH into a terminal session on the router and use the following commands:

       cd /usr/lib/opennds
       wget https://raw.githubusercontent.com/openNDS/openNDS/master/community/themespec/theme_legacy/theme_click-to-continue-legacy.sh
       cd /etc/opennds/htdocs
       wget https://raw.githubusercontent.com/openNDS/openNDS/master/community/themespec/theme_legacy/splash.html

**Activate the ThemeSpec script.**

Use the following commands:

       chmod 744 /usr/lib/opennds/theme_click-to-continue-legacy.sh
       uci set opennds.@opennds[0].login_option_enabled='3'
       uci set opennds.@opennds[0].themespec_path='/usr/lib/opennds/theme_click-to-continue-legacy.sh'
       uci commit opennds
       service opennds restart

OpenNDS should now be running the legacy click to continue script.

Test it by connecting again with your phone or tablet.
