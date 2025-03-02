OLDPATH=$PATH

export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-12.1.0_musl/bin:$OLDPATH
cd rt2880/src/router && make -f  Makefile.rt2880 $1-configure && make -f Makefile.rt2880 $1-clean && cd ../../../
cd /home/seg/DEV


export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-12.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-9.2.0_musl/bin:$OLDPATH
cd mt7621/src/router && make -f  Makefile.mt7621 $1-configure && make -f Makefile.mt7621 $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-mipsel_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd adm5120/src/router && make -f  Makefile.adm5120 $1-configure && make -f Makefile.adm5120 $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-mips_4kec_gcc-10.1.0_musl/bin:$OLDPATH
cd ar531xr2/src/router && make -f  Makefile.ar531x $1-configure && make -f Makefile.ar531x $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd ar531x/src/router && make -f  Makefile.ar531x $1-configure && make -f Makefile.ar531x $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/opt/10.2.0/bin:$OLDPATH
cd broadcom/src/router && make -f Makefile $1-configure && make $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-mipsel_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH
cd broadcom_2_6/src/router && make -f Makefile.brcm26 $1-configure && make -f Makefile.brcm26 $1-clean && cd ../../../
cd /home/seg/DEV


export PATH=/xfs/toolchains/toolchain-mipsel_74kc_gcc-11.1.0_musl/bin:$PATH
cd broadcom_2_6_80211ac/src/router && make -f Makefile.brcm3x $1-configure && make -f Makefile.brcm3x $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-mipsel_mips32_gcc-10.1.0_musl/bin:$PATH
cd broadcom_2_6_80211ac_mipselr1/src/router && make -f Makefile.brcm3x $1-configure && make -f Makefile.brcm3x $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-10.0.1_musl/bin:$OLDPATH
cd danube/src/router && make -f Makefile.danube $1-configure && make -f Makefile.danube $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-arm_arm922t_gcc-7.3.0_musl_eabi/bin:$OLDPATH
cd ks8695/src/router && make -f Makefile.openrisc $1-configure && make -f Makefile.openrisc $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-12.1.0_musl_eabi/bin:$OLDPATH
cd laguna/src/router && make -f Makefile.laguna $1-configure && make -f Makefile.laguna $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-powerpc_603e_gcc-5.4.0_musl-1.1.16/bin:$OLDPATH
cd magicbox/src/router && make -f Makefile.magicbox $1-configure && make -f Makefile.magicbox $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-powerpc_8540_gcc-8.2.0_musl/bin:$OLDPATH
cd mpc85xx/src/router && make -f Makefile.mpc85xx $1-configure && make -f Makefile.mpc85xx $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
cd pb42/src/router && make -f Makefile.pb42 $1-configure && make -f Makefile.pb42 $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-arm_gcc4.2.3/bin:$OLDPATH
cd storm/src/router && make -f Makefile.storm $1-configure && make -f Makefile.storm $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/bin:$OLDPATH
cd x86/src/router && make -f Makefile.x86 $1-configure && make -f Makefile.x86 $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-x86_64_gcc-12.1.0_musl/bin:$OLDPATH
cd x86_64/src/router && make -f Makefile.x64 $1-configure && make -f Makefile.x64 $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-10.0.1_musl/bin:$OLDPATH
cd xscale/src/router && make -f Makefile.armbe $1-configure && make -f Makefile.armbe $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-arm_cortex-a9_gcc-12.1.0_musl_eabi/bin:$OLDPATH
cd northstar/src/router && make -f Makefile.northstar $1-configure && make -f Makefile.northstar $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-12.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-4.9-linaro_musl-1.1.1_eabi/bin:$OLDPATH
cd ventana/src/router && make -f Makefile.ventana $1-configure && make -f Makefile.ventana $1-clean && cd ../../../
cd /home/seg/DEV

export PATH=/xfs/toolchains/toolchain-powerpc_603e_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
cd mpc83xx/src/router && make -f Makefile.mpc83xx $1-configure && make -f Makefile.mpc83xx $1-clean && cd ../../../
cd /home/seg/DEV


export PATH=/xfs/toolchains/toolchain-aarch64_generic_gcc-12.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-aarch64_thunderx_gcc-7.3.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-4.9-linaro_musl-1.1.1_eabi/bin:$OLDPATH
cd thunderx/src/router && make -f Makefile.thunderx $1-configure && make -f Makefile.thunderx $1-clean && cd ../../../
cd /home/seg/DEV


