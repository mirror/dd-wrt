# Unit tests

The unit tests are implemented as several kernel modules, each of which pummel Jool's inner subroutines directly.

They do not require a successful Jool installation, but you need to ensure they are run in a whitespace-free directory.

```bash
make
./test.sh
make clean # optional
```

The `test.sh` script will run each test and show the results using `less` (so keep scrolling and hitting _q_). The last line of the output will indicate if any errors were found.

Some tests (most notably, the Filtering and Updating test) might appear like they are failing but the ending verdict will report no issues. This is normal; it is the result of failure pipelines being tested. Watch out for kernel warnings or panics, however. If they happen, the framework might fail to detect them. (This is the reason why I'm asking you to scroll as opposed to automating the spitting of an error code.)

In case you're wondering, kernel warnings and panics tend to look somewhat like this (assuming the kernel does not crash too badly):

	[   69.510310] ------------[ cut here ]------------
	[   69.510517] kernel BUG at /home/user/Jool/test/unit/hashtable/hash_table_test.c:227!
	[   69.510675] invalid opcode: 0000 [#1] SMP 
	[   69.511558] Modules linked in: hashtable(OF+) vboxsf(OF) snd_intel8x0 snd_ac97_codec ac97_bus snd_pcm snd_page_alloc snd_seq_midi snd_seq_midi_event openvswitch snd_rawmidi gre vxlan ip_tunnel libcrc32c snd_seq vboxvideo(OF) snd_seq_device drm snd_timer snd joydev serio_raw rfcomm bnep i2c_piix4 bluetooth soundcore vboxguest(OF) video parport_pc mac_hid ppdev lp parport hid_generic usbhid hid psmouse ahci libahci pata_acpi e1000
	[   69.512452] CPU: 0 PID: 2458 Comm: insmod Tainted: GF          O 3.13.0-24-generic #47-Ubuntu
	[   69.512452] Hardware name: innotek GmbH VirtualBox/VirtualBox, BIOS VirtualBox 12/01/2006
	[   69.512452] task: de7dcda0 ti: d85ce000 task.ti: d85ce000
	[   69.512452] EIP: 0060:[<e210149f>] EFLAGS: 00010296 CPU: 0
	[   69.512452] EIP is at init_module+0x1f/0x30 [hashtable]
	[   69.512452] EAX: 00000026 EBX: 00000000 ECX: dfbe2d68 EDX: dfbe15c4
	[   69.512452] ESI: e2101480 EDI: 00000d6d EBP: d85cfdfc ESP: d85cfdf4
	[   69.512452]  DS: 007b ES: 007b FS: 00d8 GS: 00e0 SS: 0068
	[   69.512452] CR0: 8005003b CR2: b748724c CR3: 1d559000 CR4: 000006f0
	[   69.512452] Stack:
	[   69.512452]  e2102660 e21027a4 d85cfe78 c1002122 00000000 00000000 80000000 00000000
	[   69.512452]  00000000 00000001 00000001 000185bc 00000000 00000000 00000000 e0a33000
	[   69.512452]  00000000 00000d6d d85cfe64 c104c87f 00000000 80000000 00000000 00000000
	[   69.512452] Call Trace:
	[   69.512452]  [<c1002122>] do_one_initcall+0xd2/0x190
	[   69.512452]  [<c104c87f>] ? set_memory_nx+0x5f/0x70
	[   69.512452]  [<c164769a>] ? set_section_ro_nx+0x54/0x59
	[   69.512452]  [<c10c3e9a>] load_module+0x111a/0x18e0
	[   69.512452]  [<c10c47c5>] SyS_finit_module+0x75/0xc0
	[   69.512452]  [<c11396db>] ? vm_mmap_pgoff+0x7b/0xa0
	[   69.512452]  [<c1659bcd>] sysenter_do_call+0x12/0x28
	[   69.512452] Code: 66 66 66 90 b8 9a 27 10 e2 5d c3 90 55 89 e5 83 ec 08 66 66 66 66 90 c7 44 24 04 a4 27 10 e2 c7 04 24 60 26 10 e2 e8 13 5f 54 df <0f> 0b eb 0d 90 90 90 90 90 90 90 90 90 90 90 90 90 55 89 e5 66
	[   69.512452] EIP: [<e210149f>] init_module+0x1f/0x30 [hashtable] SS:ESP 0068:d85cfdf4
	[   69.563408] ---[ end trace a091235ab099aedf ]---

Please [report any issues](https://github.com/NICMx/Jool/issues).

