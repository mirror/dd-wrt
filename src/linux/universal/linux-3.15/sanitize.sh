rm .config
rm .config.old
for i in .config*
do 
    grep "CONFIG_X86=y" $i
    if [ $? -eq 0 ] 
	then 
	    echo COPY $i
	    cp $i .config
	    grep "CONFIG_X86_64=y" $i
	    if [ $? -eq 0 ] 
		then 
		    make oldconfig ARCH=x86_64
		else
		    make oldconfig ARCH=i386
		fi
	    cp .config $i
    fi

    grep "CONFIG_ARM=y" $i
    if [ $? -eq 0 ] 
	then 
	    echo COPY $i
	    cp $i .config
	    make oldconfig ARCH=arm
	    cp .config $i
    fi

    grep "CONFIG_PPC32=y" $i
    if [ $? -eq 0 ] 
	then 
	    echo COPY $i
	    cp $i .config
	    make oldconfig ARCH=powerpc
	    cp .config $i
    fi

    grep "CONFIG_MIPS=y" $i
    if [ $? -eq 0 ] 
	then 
	    echo COPY $i
	    cp $i .config
	    echo CONFIG_DIR615I=y >> .config
	    echo CONFIG_WPE72=y >> .config
	    echo CONFIG_WA901=y >> .config
	    echo CONFIG_WDR4300=y >> .config
	    echo CONFIG_WDR2543=y >> .config
	    echo CONFIG_WR841V8=y >> .config
	    echo CONFIG_NVRAM_64K=y >> .config
	    echo CONFIG_NVRAM_60K=y >> .config
	    make oldconfig ARCH=mips
	    sed -i 's/\CONFIG_WR841V8=y/ /g' .config	    
	    sed -i 's/\CONFIG_WPE72=y/ /g' .config	    
	    sed -i 's/\CONFIG_DIR615I=y/ /g' .config	    
	    sed -i 's/\CONFIG_WA901=y/ /g' .config	    
	    sed -i 's/\CONFIG_WDR4300=y/ /g' .config	    
	    sed -i 's/\CONFIG_WDR2543=y/ /g' .config	    
	    sed -i 's/\CONFIG_NVRAM_64K=y/ /g' .config	    
	    sed -i 's/\CONFIG_NVRAM_60K=y/ /g' .config	    
	    cp .config $i
    fi
done
