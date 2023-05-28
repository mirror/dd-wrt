How to Compile openNDS
######################

Linux/Unix - Compile in Place on Target Hardware
************************************************

Make sure the development suite for your Linux distribution is installed:

**Debian/Ubuntu**
```
sudo apt-get install git subversion g++ libncurses5-dev gawk zlib1g-dev build-essential
```
**OpenSuse**
```
sudo zypper install -t pattern devel_basis
```
**Dependency**

The libmicrohttpd library (MHD) is a dependency of openNDS so compiling and installing this is a prerequisite.

**First**, create a working directory and "cd" into it.

**Next, Download and un-tar the libmicrohttpd source files.**

You can find a version number for MHD at https://ftp.gnu.org/gnu/libmicrohttpd/

The version number for MHD must not exceed 0.9.70 for versions of openNDS less than 6.0.0

```
 wget https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.71.tar.gz
 tar  -xf libmicrohttpd-0.9.71.tar.gz
 cd libmicrohttpd-0.9.71

```

where "0.9.71" is the MHD version number we are using in this example.

**Now configure and compile:**

```
 ./configure --disable-https
 make
 sudo rm /usr/local/lib/libmicrohttpd*
 sudo make install
 sudo rm /etc/ld.so.cache
 sudo ldconfig -v
 cd ..
 
```

**Then proceed to download the opennds source files.**

You can find a release version number for openNDS at https://github.com/openNDS/openNDS/releases

```
 wget https://codeload.github.com/opennds/opennds/tar.gz/v7.0.1
 tar -xf v7.0.1
 cd openNDS-7.0.1
 make
 sudo make install
 sudo systemctl enable opennds
 
```

Where "7.0.1" is the openNDS version we are using in this example.

openNDS should now start automatically at boot time.

It can be manually started, restarted, stopped or disabled with the following commands:

```
 sudo systemctl start opennds

 sudo systemctl restart opennds

 sudo systemctl stop opennds

 sudo systemctl disable opennds

```

The status of openNDS can be checked with the following command:

```
 sudo ndsctl status

```

On most Linux distributions you can read the last few entries for openNDS in the system message log with the command:

```
 sudo systemctl status opennds

```

If openNDS fails to start, check for error messages with the command:

```
 sudo journalctl -e

```