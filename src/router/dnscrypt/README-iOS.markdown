
DNSCrypt client proxy for iOS
=============================

This currently requires a jailbroken device.

Pre-compiled binaries for iOS 5.1.1 and later are available at
https://download.dnscrypt.org/

1) Add the content of the `bin` directory of the archive into the `bin`
directory of the device. Ditto for the `sbin` and `share` directories.

2) Edit the `dnscrypt-proxy.conf` file as well as the
`org.dnscrypt.osx.DNSCryptProxy.plist` service file. You might have to
change `LocalCache` to `no`.

3) Copy the `org.dnscrypt.osx.DNSCryptProxy.plist` file into
`/Libary/LaunchDaemons` on the device.

4) Reboot or type:

    launchctl load org.dnscrypt.osx.DNSCryptProxy.plist
    
5) Edit your Wifi settings to use `127.0.0.1` as a DNS resolver.
