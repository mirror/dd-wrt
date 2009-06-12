#!/bin/bash -x

# Checkout and build gnu toolchain for OpenRISC (or32 target)

# Usage:
# build_or32_elf_tools.sh [-c]
#   -c  Controls whether CVS checkout is done prior to build
#       You probably only want to use this command-line option the first time you build.

# Directory in which to checkout sources and build
BUILD_DIR=$HOME/or32-elf-src

# Target architecture/OS
TARGET=or32-elf

# Directory in which to put compiled files, exported headers, etc.
INSTALL_PREFIX=$HOME/tools/i386-redhat-linux

########################################################################

if [ $1foo = -cfoo ]; then DO_CVS_CHECKOUT=1; else DO_CVS_CHECKOUT=0; fi

#
# Some common variables
#
OK_STR="Build OK"
FAIL_STR="Build Failed"

mkdir -p $BUILD_DIR
cd $BUILD_DIR

#
# Start with binutils
#
if [ $DO_CVS_CHECKOUT != 0 ]; then
date > checkout_binutils.log 2>&1
cvs -d :pserver:cvs@cvs.opencores.org:/home/oc/cvs -z9 co -d binutils or1k/binutils >> \
 checkout_binutils.log 2>&1
fi

mkdir -p b-b
cd b-b
date > ../build_binutils.log 2>&1

../binutils/configure --target=$TARGET --prefix=$INSTALL_PREFIX >> ../build_binutils.log 2>&1

make all install >> ../build_binutils.log 2>&1
BUILD_BINUTILS_STATUS=$?
export PATH=$INSTALL_PREFIX/bin:$PATH
cd ..

#
# Check if binutils was built and installed correctly
#
if [ $BUILD_BINUTILS_STATUS = 0 ]; then
        echo "$OK_STR (`date`)" >> build_binutils.log
else
        echo "$FAIL_STR (`date`)" >>  build_binutils.log
fi

#
# Build gdb
#
if [ $DO_CVS_CHECKOUT != 0 ]; then
date > checkout_gdb.log 2>&1
cvs -d :pserver:cvs@cvs.opencores.org:/home/oc/cvs -z9 co -d gdb or1k/gdb-5.0 >> checkout_gdb.log 2>&1
fi

mkdir -p b-gdb
cd b-gdb
date > ../build_gdb.log 2>&1
# Current version of readline has a configuration bug, so you must not specify
# the prefix
#../gdb/configure --target=$TARGET --prefix=$INSTALL_PREFIX >> ../build_gdb.log 2>&1
../gdb/configure --target=$TARGET >> ../build_gdb.log 2>&1
make all >> ../build_gdb.log 2>&1
BUILD_GDB_STATUS=$?
cp gdb/gdb $INSTALL_PREFIX/bin/$TARGET-gdb
cd ..


#
# Check if gdb was built and installed correctly
#
if [ $BUILD_GDB_STATUS = 0 ]; then
        echo "$OK_STR (`date`)" >> build_gdb.log
else
        echo "$FAIL_STR (`date`)" >>  build_gdb.log
fi

#
# Build or1k simulator
#
if [ $DO_CVS_CHECKOUT != 0 ]; then
date > checkout_or1ksim.log 2>&1
cvs -d :pserver:cvs@cvs.opencores.org:/home/oc/cvs -z9 co -d or1ksim or1k/or1ksim >> checkout_or1ksim.log 2>&1
fi

cd or1ksim
date > ../build_or1ksim.log 2>&1
../or1ksim/configure --target=$TARGET --prefix=$INSTALL_PREFIX >> ../build_or1ksim.log 2>&1
make all install >> ../build_or1ksim.log 2>&1
BUILD_OR1KSIM_STATUS=$?
cp sim $INSTALL_PREFIX/bin/or32-elf-sim
cd ..

#
# Check if or1ksim was built and installed correctly
#
if [ $BUILD_OR1KSIM_STATUS = 0 ]; then
        echo "$OK_STR (`date`)" >> build_or1ksim.log
else
        echo "$FAIL_STR (`date`)" >>  build_or1ksim.log
fi

# For now, bail here
#exit

#
# Build gcc
#
if [ $DO_CVS_CHECKOUT != 0 ]; then
date > checkout_gcc.log 2>&1
cvs -d :pserver:cvs@cvs.opencores.org:/home/oc/cvs -z9 co -d gcc or1k/gcc-3.1 >> checkout_gcc.log 2>&1
fi

# The config script looks for libraries in a weird place.  Instead of figuring out what's wrong,
# I just placate it.

pushd $INSTALL_PREFIX
cp -pr lib $TARGET
popd

mkdir -p b-gcc
cd b-gcc
date > ../build_gcc.log 2>&1
../gcc/configure --target=$TARGET \
        --with-gnu-as --with-gnu-ld --verbose \
        --enable-threads --prefix=$INSTALL_PREFIX \
        --enable-languages="c,c++" >> ../build_gcc.log 2>&1
make all install >> ../build_gcc.log 2>&1
BUILD_GCC_STATUS=$?

#
# Check if gcc was built and installed correctly
#
if [ $BUILD_GCC_STATUS = 0 ]; then
        echo "$OK_STR (`date`)" >> build_gcc.log
else
        echo "$FAIL_STR (`date`)" >>  build_gcc.log
fi

# Install even though g++ build fails due to inability to build libg++ without C library.
# (How do we prevent building of libg++ ?)
make install

