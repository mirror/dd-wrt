#!/bin/bash
# Author: cdeleon @ Nic Mexico

# Set the LINUX_INDEX to 0
sed -i 's/^LINUX_INDEX=.*$/LINUX_INDEX=0/' config

# Get the configuration from config
. config
>$RESULT_LOG

# Update Jool repository and checkout whichever tree we want to test
echo "Updating Jool repository"|tee -a $RESULT_LOG
./jool-git.sh|tee -a $RESULT_LOG

# Set the default grub to the first kernel version in grub
echo "Configuring Ubuntu to restart on kernel ${LINUX_KERNEL[$LINUX_INDEX]}"|tee -a $RESULT_LOG
sudo sed -i '/^GRUB_DEFAULT=/ s/=.*/="Advanced options for Ubuntu>'"${LINUX_KERNEL[$LINUX_INDEX]}"'"/' /etc/default/grub

# Update GRUB2
sudo update-grub

# Add jool-test.sh to rc.local before rebooting
echo "Adding jool-init.sh to rc.local"|tee -a $RESULT_LOG
sudo sed -i '/^exit 0$/ i\cd '"$(pwd)" /etc/rc.local
sudo sed -i '/^exit 0$/ i\./jool-test.sh' /etc/rc.local

# Finally, reboot and let jool-test.sh roll out
echo "Rebooting on kernel ${LINUX_KERNEL[$LINUX_INDEX]}"|tee -a $RESULT_LOG
sudo reboot
