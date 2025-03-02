OLDPATH=$PATH

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


export PATH=/xfs/toolchains/toolchain-aarch64_thunderx_gcc-7.3.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-4.9-linaro_musl-1.1.1_eabi/bin:$OLDPATH
cd thunderx/src/router && make -f Makefile.thunderx $1-configure && make -f Makefile.thunderx $1-clean && cd ../../../
cd /home/seg/DEV


