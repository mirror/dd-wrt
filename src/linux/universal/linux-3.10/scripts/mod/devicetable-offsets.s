	.file	1 "devicetable-offsets.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 3

 # -G value = 0, Arch = mips32r2, ISA = 33
 # GNU C (OpenWrt/Linaro GCC 4.8-2013.04 r36550) version 4.8.1 (mips-openwrt-linux-uclibc)
 #	compiled by GNU C version 4.7.1 20120723 [gcc-4_7-branch revision 189773], GMP version 5.1.1, MPFR version 3.1.1, MPC version 0.9
 # GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
 # options passed:  -nostdinc
 # -I /home/seg/DEV/northstar/src/linux/universal/linux-3.10/arch/mips/include
 # -I arch/mips/include/generated -I include
 # -I /home/seg/DEV/northstar/src/linux/universal/linux-3.10/arch/mips/include/uapi
 # -I arch/mips/include/generated/uapi
 # -I /home/seg/DEV/northstar/src/linux/universal/linux-3.10/include/uapi
 # -I include/generated/uapi
 # -I /home/seg/DEV/northstar/src/linux/universal/linux-3.10/arch/mips/include/asm/mach-ar7240
 # -I /home/seg/DEV/northstar/src/linux/universal/linux-3.10/arch/mips/include/asm/mach-generic
 # -iprefix /home/xfs/toolchains/toolchain-mips_r2_gcc-4.8-linaro_uClibc-0.9.33.2/bin/../lib64/gcc/mips-openwrt-linux-uclibc/4.8.1/
 # -D __KERNEL__ -D VMLINUX_LOAD_ADDRESS=0xffffffff80060000 -D DATAOFFSET=0
 # -D CC_HAVE_ASM_GOTO -D KBUILD_STR(s)=#s
 # -D KBUILD_BASENAME=KBUILD_STR(devicetable_offsets)
 # -D KBUILD_MODNAME=KBUILD_STR(devicetable_offsets)
 # -isystem /home/xfs/toolchains/toolchain-mips_r2_gcc-4.8-linaro_uClibc-0.9.33.2/bin/../lib64/gcc/mips-openwrt-linux-uclibc/4.8.1/include
 # -include /home/seg/DEV/northstar/src/linux/universal/linux-3.10/include/linux/kconfig.h
 # -MD scripts/mod/.devicetable-offsets.s.d
 # scripts/mod/devicetable-offsets.c -G 0 -mno-check-zero-division -mabi=32
 # -mno-abicalls -msoft-float -mno-branch-likely -march=mips32r2
 # -mtune=74kc -mllsc -mplt -mno-shared
 # -auxbase-strip scripts/mod/devicetable-offsets.s -Os -Wall -Wundef
 # -Wstrict-prototypes -Wno-trigraphs -Werror=implicit-function-declaration
 # -Wno-format-security -Wno-maybe-uninitialized -Wframe-larger-than=1024
 # -Wno-unused-but-set-variable -Wdeclaration-after-statement
 # -Wno-pointer-sign -fno-strict-aliasing -fno-common
 # -fno-delete-null-pointer-checks -fno-caller-saves -fno-pic
 # -ffreestanding -fno-stack-protector -fomit-frame-pointer
 # -fno-strict-overflow -fconserve-stack -ffunction-sections
 # -fdata-sections -fverbose-asm
 # options enabled:  -faggressive-loop-optimizations -fauto-inc-dec
 # -fbranch-count-reg -fcombine-stack-adjustments -fcompare-elim
 # -fcprop-registers -fcrossjumping -fcse-follow-jumps -fdata-sections
 # -fdefer-pop -fdelayed-branch -fdevirtualize -fdwarf2-cfi-asm
 # -fearly-inlining -feliminate-unused-debug-types
 # -fexpensive-optimizations -fforward-propagate -ffunction-cse
 # -ffunction-sections -fgcse -fgcse-lm -fgnu-runtime
 # -fguess-branch-probability -fhoist-adjacent-loads -fident
 # -fif-conversion -fif-conversion2 -findirect-inlining -finline
 # -finline-atomics -finline-functions -finline-functions-called-once
 # -finline-small-functions -fipa-cp -fipa-profile -fipa-pure-const
 # -fipa-reference -fipa-sra -fira-hoist-pressure -fira-share-save-slots
 # -fira-share-spill-slots -fivopts -fkeep-static-consts
 # -fleading-underscore -fmath-errno -fmerge-constants
 # -fmerge-debug-strings -fmove-loop-invariants -fomit-frame-pointer
 # -foptimize-register-move -foptimize-sibling-calls -fpartial-inlining
 # -fpcc-struct-return -fpeephole -fpeephole2 -fprefetch-loop-arrays
 # -fregmove -freorder-blocks -freorder-functions -frerun-cse-after-loop
 # -fsched-critical-path-heuristic -fsched-dep-count-heuristic
 # -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
 # -fsched-rank-heuristic -fsched-spec -fsched-spec-insn-heuristic
 # -fsched-stalled-insns-dep -fschedule-insns2 -fshow-column -fshrink-wrap
 # -fsigned-zeros -fsplit-ivs-in-unroller -fsplit-wide-types
 # -fstrict-volatile-bitfields -fsync-libcalls -fthread-jumps
 # -ftoplevel-reorder -ftrapping-math -ftree-bit-ccp
 # -ftree-builtin-call-dce -ftree-ccp -ftree-ch -ftree-coalesce-vars
 # -ftree-copy-prop -ftree-copyrename -ftree-cselim -ftree-dce
 # -ftree-dominator-opts -ftree-dse -ftree-forwprop -ftree-fre
 # -ftree-loop-if-convert -ftree-loop-im -ftree-loop-ivcanon
 # -ftree-loop-optimize -ftree-parallelize-loops= -ftree-phiprop -ftree-pre
 # -ftree-pta -ftree-reassoc -ftree-scev-cprop -ftree-sink
 # -ftree-slp-vectorize -ftree-slsr -ftree-sra -ftree-switch-conversion
 # -ftree-tail-merge -ftree-ter -ftree-vect-loop-version -ftree-vrp
 # -funit-at-a-time -fverbose-asm -fzero-initialized-in-bss -mdivide-traps
 # -mdouble-float -meb -mexplicit-relocs -mextern-sdata -mfp-exceptions
 # -mfp32 -mfused-madd -mgp32 -mgpopt -mllsc -mlocal-sdata -mlong32
 # -mmemcpy -mno-mdmx -mno-mips16 -mno-mips3d -mplt -msoft-float
 # -msplit-addresses -muclibc

	.section	.text.startup.main,"ax",@progbits
	.align	2
	.globl	main
	.set	nomips16
	.ent	main
	.type	main, @function
main:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 10 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_usb_device_id 24 sizeof(struct usb_device_id)	 #
 # 0 "" 2
 # 11 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_match_flags 0 offsetof(struct usb_device_id, match_flags)	 #
 # 0 "" 2
 # 12 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_idVendor 2 offsetof(struct usb_device_id, idVendor)	 #
 # 0 "" 2
 # 13 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_idProduct 4 offsetof(struct usb_device_id, idProduct)	 #
 # 0 "" 2
 # 14 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bcdDevice_lo 6 offsetof(struct usb_device_id, bcdDevice_lo)	 #
 # 0 "" 2
 # 15 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bcdDevice_hi 8 offsetof(struct usb_device_id, bcdDevice_hi)	 #
 # 0 "" 2
 # 16 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bDeviceClass 10 offsetof(struct usb_device_id, bDeviceClass)	 #
 # 0 "" 2
 # 17 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bDeviceSubClass 11 offsetof(struct usb_device_id, bDeviceSubClass)	 #
 # 0 "" 2
 # 18 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bDeviceProtocol 12 offsetof(struct usb_device_id, bDeviceProtocol)	 #
 # 0 "" 2
 # 19 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bInterfaceClass 13 offsetof(struct usb_device_id, bInterfaceClass)	 #
 # 0 "" 2
 # 20 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bInterfaceSubClass 14 offsetof(struct usb_device_id, bInterfaceSubClass)	 #
 # 0 "" 2
 # 21 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bInterfaceProtocol 15 offsetof(struct usb_device_id, bInterfaceProtocol)	 #
 # 0 "" 2
 # 22 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_usb_device_id_bInterfaceNumber 16 offsetof(struct usb_device_id, bInterfaceNumber)	 #
 # 0 "" 2
 # 24 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_hid_device_id 16 sizeof(struct hid_device_id)	 #
 # 0 "" 2
 # 25 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_hid_device_id_bus 0 offsetof(struct hid_device_id, bus)	 #
 # 0 "" 2
 # 26 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_hid_device_id_group 2 offsetof(struct hid_device_id, group)	 #
 # 0 "" 2
 # 27 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_hid_device_id_vendor 4 offsetof(struct hid_device_id, vendor)	 #
 # 0 "" 2
 # 28 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_hid_device_id_product 8 offsetof(struct hid_device_id, product)	 #
 # 0 "" 2
 # 30 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_ieee1394_device_id 24 sizeof(struct ieee1394_device_id)	 #
 # 0 "" 2
 # 31 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ieee1394_device_id_match_flags 0 offsetof(struct ieee1394_device_id, match_flags)	 #
 # 0 "" 2
 # 32 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ieee1394_device_id_vendor_id 4 offsetof(struct ieee1394_device_id, vendor_id)	 #
 # 0 "" 2
 # 33 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ieee1394_device_id_model_id 8 offsetof(struct ieee1394_device_id, model_id)	 #
 # 0 "" 2
 # 34 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ieee1394_device_id_specifier_id 12 offsetof(struct ieee1394_device_id, specifier_id)	 #
 # 0 "" 2
 # 35 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ieee1394_device_id_version 16 offsetof(struct ieee1394_device_id, version)	 #
 # 0 "" 2
 # 37 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_pci_device_id 28 sizeof(struct pci_device_id)	 #
 # 0 "" 2
 # 38 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pci_device_id_vendor 0 offsetof(struct pci_device_id, vendor)	 #
 # 0 "" 2
 # 39 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pci_device_id_device 4 offsetof(struct pci_device_id, device)	 #
 # 0 "" 2
 # 40 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pci_device_id_subvendor 8 offsetof(struct pci_device_id, subvendor)	 #
 # 0 "" 2
 # 41 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pci_device_id_subdevice 12 offsetof(struct pci_device_id, subdevice)	 #
 # 0 "" 2
 # 42 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pci_device_id_class 16 offsetof(struct pci_device_id, class)	 #
 # 0 "" 2
 # 43 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pci_device_id_class_mask 20 offsetof(struct pci_device_id, class_mask)	 #
 # 0 "" 2
 # 45 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_ccw_device_id 12 sizeof(struct ccw_device_id)	 #
 # 0 "" 2
 # 46 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ccw_device_id_match_flags 0 offsetof(struct ccw_device_id, match_flags)	 #
 # 0 "" 2
 # 47 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ccw_device_id_cu_type 2 offsetof(struct ccw_device_id, cu_type)	 #
 # 0 "" 2
 # 48 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ccw_device_id_cu_model 6 offsetof(struct ccw_device_id, cu_model)	 #
 # 0 "" 2
 # 49 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ccw_device_id_dev_type 4 offsetof(struct ccw_device_id, dev_type)	 #
 # 0 "" 2
 # 50 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ccw_device_id_dev_model 7 offsetof(struct ccw_device_id, dev_model)	 #
 # 0 "" 2
 # 52 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_ap_device_id 8 sizeof(struct ap_device_id)	 #
 # 0 "" 2
 # 53 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ap_device_id_dev_type 2 offsetof(struct ap_device_id, dev_type)	 #
 # 0 "" 2
 # 55 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_css_device_id 8 sizeof(struct css_device_id)	 #
 # 0 "" 2
 # 56 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_css_device_id_type 1 offsetof(struct css_device_id, type)	 #
 # 0 "" 2
 # 58 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_serio_device_id 4 sizeof(struct serio_device_id)	 #
 # 0 "" 2
 # 59 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_serio_device_id_type 0 offsetof(struct serio_device_id, type)	 #
 # 0 "" 2
 # 60 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_serio_device_id_proto 3 offsetof(struct serio_device_id, proto)	 #
 # 0 "" 2
 # 61 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_serio_device_id_id 2 offsetof(struct serio_device_id, id)	 #
 # 0 "" 2
 # 62 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_serio_device_id_extra 1 offsetof(struct serio_device_id, extra)	 #
 # 0 "" 2
 # 64 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_acpi_device_id 16 sizeof(struct acpi_device_id)	 #
 # 0 "" 2
 # 65 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_acpi_device_id_id 0 offsetof(struct acpi_device_id, id)	 #
 # 0 "" 2
 # 67 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_pnp_device_id 12 sizeof(struct pnp_device_id)	 #
 # 0 "" 2
 # 68 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pnp_device_id_id 0 offsetof(struct pnp_device_id, id)	 #
 # 0 "" 2
 # 70 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_pnp_card_device_id 76 sizeof(struct pnp_card_device_id)	 #
 # 0 "" 2
 # 71 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pnp_card_device_id_devs 12 offsetof(struct pnp_card_device_id, devs)	 #
 # 0 "" 2
 # 73 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_pcmcia_device_id 52 sizeof(struct pcmcia_device_id)	 #
 # 0 "" 2
 # 74 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pcmcia_device_id_match_flags 0 offsetof(struct pcmcia_device_id, match_flags)	 #
 # 0 "" 2
 # 75 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pcmcia_device_id_manf_id 2 offsetof(struct pcmcia_device_id, manf_id)	 #
 # 0 "" 2
 # 76 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pcmcia_device_id_card_id 4 offsetof(struct pcmcia_device_id, card_id)	 #
 # 0 "" 2
 # 77 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pcmcia_device_id_func_id 6 offsetof(struct pcmcia_device_id, func_id)	 #
 # 0 "" 2
 # 78 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pcmcia_device_id_function 7 offsetof(struct pcmcia_device_id, function)	 #
 # 0 "" 2
 # 79 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pcmcia_device_id_device_no 8 offsetof(struct pcmcia_device_id, device_no)	 #
 # 0 "" 2
 # 80 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_pcmcia_device_id_prod_id_hash 12 offsetof(struct pcmcia_device_id, prod_id_hash)	 #
 # 0 "" 2
 # 82 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_of_device_id 196 sizeof(struct of_device_id)	 #
 # 0 "" 2
 # 83 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_of_device_id_name 0 offsetof(struct of_device_id, name)	 #
 # 0 "" 2
 # 84 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_of_device_id_type 32 offsetof(struct of_device_id, type)	 #
 # 0 "" 2
 # 85 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_of_device_id_compatible 64 offsetof(struct of_device_id, compatible)	 #
 # 0 "" 2
 # 87 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_vio_device_id 64 sizeof(struct vio_device_id)	 #
 # 0 "" 2
 # 88 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_vio_device_id_type 0 offsetof(struct vio_device_id, type)	 #
 # 0 "" 2
 # 89 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_vio_device_id_compat 32 offsetof(struct vio_device_id, compat)	 #
 # 0 "" 2
 # 91 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_input_device_id 160 sizeof(struct input_device_id)	 #
 # 0 "" 2
 # 92 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_flags 0 offsetof(struct input_device_id, flags)	 #
 # 0 "" 2
 # 93 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_bustype 4 offsetof(struct input_device_id, bustype)	 #
 # 0 "" 2
 # 94 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_vendor 6 offsetof(struct input_device_id, vendor)	 #
 # 0 "" 2
 # 95 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_product 8 offsetof(struct input_device_id, product)	 #
 # 0 "" 2
 # 96 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_version 10 offsetof(struct input_device_id, version)	 #
 # 0 "" 2
 # 97 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_evbit 12 offsetof(struct input_device_id, evbit)	 #
 # 0 "" 2
 # 98 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_keybit 16 offsetof(struct input_device_id, keybit)	 #
 # 0 "" 2
 # 99 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_relbit 112 offsetof(struct input_device_id, relbit)	 #
 # 0 "" 2
 # 100 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_absbit 116 offsetof(struct input_device_id, absbit)	 #
 # 0 "" 2
 # 101 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_mscbit 124 offsetof(struct input_device_id, mscbit)	 #
 # 0 "" 2
 # 102 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_ledbit 128 offsetof(struct input_device_id, ledbit)	 #
 # 0 "" 2
 # 103 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_sndbit 132 offsetof(struct input_device_id, sndbit)	 #
 # 0 "" 2
 # 104 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_ffbit 136 offsetof(struct input_device_id, ffbit)	 #
 # 0 "" 2
 # 105 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_input_device_id_swbit 152 offsetof(struct input_device_id, swbit)	 #
 # 0 "" 2
 # 107 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_eisa_device_id 12 sizeof(struct eisa_device_id)	 #
 # 0 "" 2
 # 108 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_eisa_device_id_sig 0 offsetof(struct eisa_device_id, sig)	 #
 # 0 "" 2
 # 110 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_parisc_device_id 8 sizeof(struct parisc_device_id)	 #
 # 0 "" 2
 # 111 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_parisc_device_id_hw_type 0 offsetof(struct parisc_device_id, hw_type)	 #
 # 0 "" 2
 # 112 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_parisc_device_id_hversion 2 offsetof(struct parisc_device_id, hversion)	 #
 # 0 "" 2
 # 113 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_parisc_device_id_hversion_rev 1 offsetof(struct parisc_device_id, hversion_rev)	 #
 # 0 "" 2
 # 114 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_parisc_device_id_sversion 4 offsetof(struct parisc_device_id, sversion)	 #
 # 0 "" 2
 # 116 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_sdio_device_id 12 sizeof(struct sdio_device_id)	 #
 # 0 "" 2
 # 117 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_sdio_device_id_class 0 offsetof(struct sdio_device_id, class)	 #
 # 0 "" 2
 # 118 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_sdio_device_id_vendor 2 offsetof(struct sdio_device_id, vendor)	 #
 # 0 "" 2
 # 119 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_sdio_device_id_device 4 offsetof(struct sdio_device_id, device)	 #
 # 0 "" 2
 # 121 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_ssb_device_id 6 sizeof(struct ssb_device_id)	 #
 # 0 "" 2
 # 122 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ssb_device_id_vendor 0 offsetof(struct ssb_device_id, vendor)	 #
 # 0 "" 2
 # 123 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ssb_device_id_coreid 2 offsetof(struct ssb_device_id, coreid)	 #
 # 0 "" 2
 # 124 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ssb_device_id_revision 4 offsetof(struct ssb_device_id, revision)	 #
 # 0 "" 2
 # 126 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_bcma_device_id 6 sizeof(struct bcma_device_id)	 #
 # 0 "" 2
 # 127 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_bcma_device_id_manuf 0 offsetof(struct bcma_device_id, manuf)	 #
 # 0 "" 2
 # 128 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_bcma_device_id_id 2 offsetof(struct bcma_device_id, id)	 #
 # 0 "" 2
 # 129 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_bcma_device_id_rev 4 offsetof(struct bcma_device_id, rev)	 #
 # 0 "" 2
 # 130 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_bcma_device_id_class 5 offsetof(struct bcma_device_id, class)	 #
 # 0 "" 2
 # 132 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_virtio_device_id 8 sizeof(struct virtio_device_id)	 #
 # 0 "" 2
 # 133 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_virtio_device_id_device 0 offsetof(struct virtio_device_id, device)	 #
 # 0 "" 2
 # 134 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_virtio_device_id_vendor 4 offsetof(struct virtio_device_id, vendor)	 #
 # 0 "" 2
 # 136 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_hv_vmbus_device_id 20 sizeof(struct hv_vmbus_device_id)	 #
 # 0 "" 2
 # 137 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_hv_vmbus_device_id_guid 0 offsetof(struct hv_vmbus_device_id, guid)	 #
 # 0 "" 2
 # 139 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_i2c_device_id 24 sizeof(struct i2c_device_id)	 #
 # 0 "" 2
 # 140 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_i2c_device_id_name 0 offsetof(struct i2c_device_id, name)	 #
 # 0 "" 2
 # 142 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_spi_device_id 36 sizeof(struct spi_device_id)	 #
 # 0 "" 2
 # 143 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_spi_device_id_name 0 offsetof(struct spi_device_id, name)	 #
 # 0 "" 2
 # 145 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_dmi_system_id 332 sizeof(struct dmi_system_id)	 #
 # 0 "" 2
 # 146 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_dmi_system_id_matches 8 offsetof(struct dmi_system_id, matches)	 #
 # 0 "" 2
 # 148 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_platform_device_id 24 sizeof(struct platform_device_id)	 #
 # 0 "" 2
 # 149 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_platform_device_id_name 0 offsetof(struct platform_device_id, name)	 #
 # 0 "" 2
 # 151 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_mdio_device_id 8 sizeof(struct mdio_device_id)	 #
 # 0 "" 2
 # 152 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_mdio_device_id_phy_id 0 offsetof(struct mdio_device_id, phy_id)	 #
 # 0 "" 2
 # 153 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_mdio_device_id_phy_id_mask 4 offsetof(struct mdio_device_id, phy_id_mask)	 #
 # 0 "" 2
 # 155 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_zorro_device_id 8 sizeof(struct zorro_device_id)	 #
 # 0 "" 2
 # 156 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_zorro_device_id_id 0 offsetof(struct zorro_device_id, id)	 #
 # 0 "" 2
 # 158 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_isapnp_device_id 12 sizeof(struct isapnp_device_id)	 #
 # 0 "" 2
 # 159 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_isapnp_device_id_vendor 4 offsetof(struct isapnp_device_id, vendor)	 #
 # 0 "" 2
 # 160 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_isapnp_device_id_function 6 offsetof(struct isapnp_device_id, function)	 #
 # 0 "" 2
 # 162 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_ipack_device_id 12 sizeof(struct ipack_device_id)	 #
 # 0 "" 2
 # 163 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ipack_device_id_format 0 offsetof(struct ipack_device_id, format)	 #
 # 0 "" 2
 # 164 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ipack_device_id_vendor 4 offsetof(struct ipack_device_id, vendor)	 #
 # 0 "" 2
 # 165 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_ipack_device_id_device 8 offsetof(struct ipack_device_id, device)	 #
 # 0 "" 2
 # 167 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_amba_id 12 sizeof(struct amba_id)	 #
 # 0 "" 2
 # 168 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_amba_id_id 0 offsetof(struct amba_id, id)	 #
 # 0 "" 2
 # 169 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_amba_id_mask 4 offsetof(struct amba_id, mask)	 #
 # 0 "" 2
 # 171 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_x86_cpu_id 12 sizeof(struct x86_cpu_id)	 #
 # 0 "" 2
 # 172 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_x86_cpu_id_feature 6 offsetof(struct x86_cpu_id, feature)	 #
 # 0 "" 2
 # 173 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_x86_cpu_id_family 2 offsetof(struct x86_cpu_id, family)	 #
 # 0 "" 2
 # 174 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_x86_cpu_id_model 4 offsetof(struct x86_cpu_id, model)	 #
 # 0 "" 2
 # 175 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_x86_cpu_id_vendor 0 offsetof(struct x86_cpu_id, vendor)	 #
 # 0 "" 2
 # 177 "scripts/mod/devicetable-offsets.c" 1
	
->SIZE_mei_cl_device_id 36 sizeof(struct mei_cl_device_id)	 #
 # 0 "" 2
 # 178 "scripts/mod/devicetable-offsets.c" 1
	
->OFF_mei_cl_device_id_name 0 offsetof(struct mei_cl_device_id, name)	 #
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0	 #,
	.set	macro
	.set	reorder

	.end	main
	.size	main, .-main
	.ident	"GCC: (OpenWrt/Linaro GCC 4.8-2013.04 r36550) 4.8.1"
