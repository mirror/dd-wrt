#!/bin/sh
#./nightly_build.sh > make_log_apr_15_2pm.txt 2>&1
PERFORCE_CLIENT=lnx_build
# Repository home
REPO_HOME=/home/$USER/depot/sw
TGT_DIR=/home/$USER/tgt
TFTP_DIR=/tftpboot
export P4CLIENT=$PERFORCE_CLIENT
LOCK_FILE=$HOME/build.lock

check_and_create_lock() {

if [ -f $LOCK_FILE ]; then
    echo "Lock file $LOCK_FILE exist.Please wait for current build to complete."
    echo "Alternatively you can remove the lock file and run the build."
    echo "Before deleting lock make sure no build or it's child proceses are running."
    exit "1"
else 
    touch $LOCK_FILE
fi

}

delete_lock() {

if [ -e $LOCK_FILE ]; then
    echo "Removing lock file $LOCK_FILE."
    rm $LOCK_FILE
else
    echo "Error:Expecting lock file $LOCK_FILE."
fi

}


perforce_sync() {
pushd $(pwd);cd $REPO_HOME/linuxsrc/
/usr/local/bin/p4 sync
echo "__________________________ SYNC DONE ______________________________"
}


build() {
sudo mv $TFTP_DIR/ $TFTP_DIR_$(date +%D_%T | sed -e "s|/|_|g" | sed -e "s/:/_/g")
pushd $(pwd);cd $REPO_HOME/linuxsrc/src
cd ${REPO_HOME}/linuxsrc/src
sudo mkdir $TFTP_DIR 
make ap30
echo "__________________________ AP30 DONE________________________________"
make ap51
echo "__________________________ AP51 DONE________________________________"
make ap30-ram
echo "__________________________ AP30-RAM DONE____________________________"
make ap51-ram
echo "__________________________ AP51-RAM DONE____________________________"
make ap30-debug
echo "__________________________ AP30-DEBUG DONE____________________________"
make ap51-debug
echo "__________________________ AP51-DEBUG DONE____________________________"
popd

}

copy_tgt_images() {

if [ -d $TGT_DIR ]; then
    echo "Using $TGT_DIR"
else
    echo "Making $TGT_DIR"
    mkdir $TGT_DIR
fi

BUILD_DIR=$TGT_DIR/build
BAKUP_DIR=$TGT_DIR/build_$(date +%D_%T | sed -e "s|/|_|g" | sed -e "s/:/_/g")
echo $BUILD_DIR
echo $BAKUP_DIR
if [ -d $BUILD_DIR ]; then
echo "Backing up $BUILD_DIR to $BAKUP_DIR";mv $BUILD_DIR $BAKUP_DIR
fi

mkdir $BUILD_DIR

pushd $(pwd);cd $REPO_HOME/linuxsrc/src

cp -a $TFTP_DIR/* $BUILD_DIR

}

#lock=check_and_create_lock
#if [ "$lock" -eq 1 ]; then
#exit
#fi

echo $LOCK_FILE
if [ -e $LOCK_FILE ]; then
    echo "Lock file $LOCK_FILE exists.Please wait for current build to complete."
    echo "Alternatively you can remove the lock file and run the build."
    echo "Before deleting lock make sure no build or it's child proceses are running."
    exit "1"
else 
    touch $LOCK_FILE
fi


echo "Sync triggered at " $(date +%D_%T | sed -e "s|/|_|g" | sed -e "s/:/_/g")
perforce_sync
build
copy_tgt_images
delete_lock