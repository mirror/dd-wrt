#!/bin/bash
# Author: Cristobal De Leon @ Nic Mexico

# Load configuration file
. config

echo "Current Kernel should be ${LINUX_KERNEL[$LINUX_INDEX]}"|tee -a $RESULT_LOG
echo -e "Current Kernel is $(uname -r)\n"|tee -a $RESULT_LOG

# Make sure we booted on the correct kernel. If not, there's an issue, so we need to abort
if [[ ${LINUX_KERNEL[$LINUX_INDEX]} != *$(uname -r)* ]]; then
	echo "Something's wrong, aborting"
	sudo sed -i '/cd '"${pwd/\//.}"'/d' /etc/rc.local
        sudo sed -i '/jool-test.sh/d' /etc/rc.local
        IFS=$OLDIFS
	exit 1
fi

# Compile Jool on the current kernel
./jool-compile.sh|tee -a $RESULT_LOG

# Catch compilation exit code
COMPILE_STATUS=${PIPESTATUS[0]}

# Only after a successful compilation can we proceed with the tests
if [ ${COMPILE_STATUS} -eq 0 ]; then
	# Compile and run unit tests
	echo -e "\nRunning Unit test suite.\n"|tee -a $RESULT_LOG
	./jool-unit.sh|tee -a $RESULT_LOG
	# Compile and run graybox tests
	echo -e "\nRunning Graybox test suite.\n"|tee -a $RESULT_LOG
	./jool-graybox.sh|tee -a $RESULT_LOG
else
	echo "Compilation failed. Aborting tests."|tee -a $RESULT_LOG
fi

# When done, increase the kernel index by one
LINUX_INDEX=$((LINUX_INDEX+2))

MESSAGE="\n**********************************************************************************************\n\n"

# Verificar que el índice no supere la cantidad de kernels instalados, y aumentarlo en 1 (para iterar el siguiente kernel)
if [[ "$LINUX_INDEX" -ge "${#LINUX_KERNEL[@]}" ]]; then
        sudo sed -i '/cd '"${pwd/\//.}"'/d' /etc/rc.local
        sudo sed -i '/jool-test.sh/d' /etc/rc.local
	IFS=$OLDIFS
        MESSAGE="$MESSAGE We are done"
	echo -e $MESSAGE
	exit 0
else
        sed -i 's/^LINUX_INDEX=.$/LINUX_INDEX='"$LINUX_INDEX"'/' config
        sudo sed -i '/^GRUB_DEFAULT=/ s/=.*/="Advanced options for Ubuntu>'"${LINUX_KERNEL[$LINUX_INDEX]}"'"/' /etc/default/grub
        sudo update-grub
        MESSAGE="$MESSAGE Rebooting on kernel ${LINUX_KERNEL[$LINUX_INDEX]}\n**********************************************************************************************\n\n"
fi

echo -e $MESSAGE|tee -a $RESULT_LOG
sudo reboot
