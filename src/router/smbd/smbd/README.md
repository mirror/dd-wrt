
# Content

- [What is CIFSD?](#what-is-cifsd)
- [Under PFIF](#under-pfif)
- [Git](#git)
- [Maintainers](#maintainers)
- [Bug reports or contribution](#Bug-reports-or-contribution)
- [Features](#features)
- [Supported Linux Kernel Versions](#supported-linux-kernel-versions)
- [CIFSD architecture](#cifsd-architecture)


## What is CIFSD?

CIFSD is an opensource In-kernel CIFS/SMB3 server created by Namjae Jeon for Linux Kernel. It's an implementation of SMB/CIFS protocol in kernel space for sharing files and IPC services over network. Initially the target is to provide improved file I/O performances, but the bigger goal is to have some new features which are much easier to develop and maintain inside the kernel and expose the layers fully. Directions can be attributed to sections where SAMBA is moving to few modules inside the kernel to have features like RDMA(Remote direct memory access) to work with actual performance gain.


## Under PFIF

This code was developed in participation with the Protocol Freedom Information Foundation.

Please see
* http://protocolfreedom.org/
* http://samba.org/samba/PFIF/
for more details.


## Git

The development git tree is available at
* https://github.com/cifsd-team/cifsd
* https://github.com/cifsd-team/cifsd-tools


## Maintainers

* Namjae Jeon <linkinjeon@kernel.org>
* Sergey Senozhatsky <sergey.senozhatsky@gmail.com>


## Bug reports or contribution

For reporting bugs and sending patches, please send the patches to the following mail address:

* linux-cifsd-devel@lists.sourceforge.net
* linkinjeon@kernel.org
* sergey.senozhatsky@gmail.com

or open issues/send PRs to [CIFSD](https://github.com/cifsd-team/cifsd).


## Installing as a stand-alone module

Install prerequisite package for Fedora, RHEL:
```
	yum install kernel-devel-$(uname -r)
```

Build step:
```
	make
	sudo make install
```

To load the driver manually, run this as root:
```
	modprobe ksmbd
```


## Installing as a part of the kernel

1. Let's take [linux] as the path to your kernel source dir.
```
	cd [linux]
	cp -ar cifsd [linux]/fs/
```

2. edit [linux]/fs/Kconfig
```
	source "fs/cifs/Kconfig"
	+source "fs/cifsd/Kconfig"
	source "fs/coda/Kconfig"
```

3. edit [linux]/fs/Makefile
```
	obj-$(CONFIG_CIFS)              += cifs/
	+obj-$(CONFIG_SMB_SERVER)       += cifsd/
	obj-$(CONFIG_HPFS_FS)           += hpfs/
```
4. make menuconfig and set cifsd
```
	[*] Network File Systems  --->
		<M>   SMB server support
```

build your kernel


## Features

*Implemented*
1. SMB1(CIFS), SMB2/3 protocols for basic file sharing
2. Dynamic crediting
3. Compound requests
4. Durable handle
5. oplock/lease
6. Large MTU
7. NTLM/NTLMv2
8. Auto negotiation
9. HMAC-SHA256 Signing
10. Secure negotiate
11. Signing Update
12. Pre-authentication integrity(SMB 3.1.1)
13. SMB3 encryption(CCM, GCM)
14. SMB direct(RDMA)

*Planned*
1. Multi-channel
2. Durable handle v2
3. Kerberos
4. Persistent handles
5. Directory lease
6. Win-ACL


## Supported Linux Kernel Versions

* Linux Kernel 4.1 or later


## CIFSD architecture

```
               |--- ...
       --------|--- ksmbd/3 - Client 3
       |-------|--- ksmbd/2 - Client 2
       |       |         _____________________________________________________
       |       |        |- Client 1                                           |
<--- Socket ---|--- ksmbd/1   <<= Authentication : NTLM/NTLM2, Kerberos(TODO)|
       |       |      | |      <<= SMB : SMB1, SMB2, SMB2.1, SMB3, SMB3.0.2,  |
       |       |      | |                SMB3.1.1                             |
       |       |      | |_____________________________________________________|
       |       |      |
       |       |      |--- VFS --- Local Filesystem
       |       |
KERNEL |--- ksmbd/0(forker kthread)
---------------||---------------------------------------------------------------
USER           ||
               || communication using NETLINK
               ||  ______________________________________________
               || |                                              |
        ksmbd.mountd <<= DCE/RPC, WINREG                         |
               ^  |  <<= configure shares setting, user accounts |
               |  |______________________________________________|
               |
               |------ smb.conf(config file)
               |
               |------ ksmbdpwd.db(user account/password file)
                            ^
  ksmbd.adduser ---------------|

```
