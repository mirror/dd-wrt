rm .config
rm .config.old
for i in .config*
do 
    echo COPY $i
    cp $i .config
    sed -i 's/\CONFIG_EXFAT_FS=m/ /g' .config	    
    echo "# CONFIG_EXFAT_FS is not set" >> .config
    echo "# CONFIG_LTO_MENU is not set" >> .config
    echo "# CONFIG_ASN1 is not set" >> .config
    echo "# CONFIG_CRYPTO_DEV_QCOM_ICE is not set" >> .config
    sed -i 's/\CONFIG_IP_VS=m/# CONFIG_IP_VS is not set/g' .config	    
    sed -i 's/\CONFIG_USBIP_CORE=m/# CONFIG_USBIP_CORE is not set/g' .config	    

    cp drivers/net/wireless/Kconfig.mt7621 drivers/net/wireless/Kconfig

    grep "CONFIG_DIR882=y" $i
    if [ $? -eq 0 ] 
	then 
	    cp drivers/net/wireless/Kconfig.dir882 drivers/net/wireless/Kconfig
    fi

    grep "CONFIG_CC_OPTIMIZE_FOR_SIZE=y" $i
    if [ $? -eq 0 ] 
	then
	    sed -i 's/\CONFIG_OPTIMIZE_INLINING=y/# CONFIG_OPTIMIZE_INLINING is not set/g' .config	    
    fi

    grep "CONFIG_X86=y" $i
    if [ $? -eq 0 ] 
	then 
	    echo COPY $i
	    grep "CONFIG_X86_64=y" $i
	    if [ $? -eq 0 ] 
		then 
		    make oldconfig ARCH=x86_64
		else
		    make oldconfig ARCH=i386
		fi
    fi

    grep "CONFIG_ARM=y" $i
    if [ $? -eq 0 ] 
	then 
	    make oldconfig ARCH=arm
    fi

    grep "CONFIG_ARCH_IXP4XX=y" $i
    if [ $? -eq 0 ] 
	then 
	    sed -i 's/\# CONFIG_CPU_BIG_ENDIAN is not set/CONFIG_CPU_BIG_ENDIAN=y/g' .config	    
	    make oldconfig ARCH=arm
    fi


    grep "CONFIG_ARCH_ALPINE=y" $i
    if [ $? -eq 0 ] 
	then 
	    sed -i 's/\CONFIG_CPU_BIG_ENDIAN=y/# CONFIG_CPU_BIG_ENDIAN is not set/g' .config	    
	    make oldconfig ARCH=arm
    fi

    grep "CONFIG_ARCH_QCOM=y" $i
    if [ $? -eq 0 ] 
	then 
	    sed -i 's/\CONFIG_CPU_BIG_ENDIAN=y/# CONFIG_CPU_BIG_ENDIAN is not set/g' .config	    
	    make oldconfig ARCH=arm
    fi

    grep "CONFIG_ARCH_MVEBU=y" $i
    if [ $? -eq 0 ] 
	then 
	    sed -i 's/\CONFIG_CPU_BIG_ENDIAN=y/# CONFIG_CPU_BIG_ENDIAN is not set/g' .config	    
	    make oldconfig ARCH=arm
    fi

    grep "CONFIG_PPC32=y" $i
    if [ $? -eq 0 ] 
	then 
	    make oldconfig ARCH=powerpc
    fi

    grep "CONFIG_MIPS=y" $i
    if [ $? -eq 0 ] 
	then 
	    echo CONFIG_DIR615I=y >> .config
	    echo CONFIG_WPE72=y >> .config
	    echo CONFIG_WA901=y >> .config
	    echo CONFIG_WDR4300=y >> .config
	    echo CONFIG_WDR3500=y >> .config
	    echo CONFIG_WDR2543=y >> .config
	    echo CONFIG_WR841V8=y >> .config
	    echo CONFIG_WR841V9=y >> .config
	    echo CONFIG_WR941V6=y >> .config
	    echo CONFIG_NVRAM_64K=y >> .config
	    echo CONFIG_NVRAM_60K=y >> .config
	    echo CONFIG_ALFANX=y >> .config
	    echo CONFIG_AP135=y >> .config
	    echo CONFIG_DAP3310=y >> .config
	    echo CONFIG_WR1043V2=y >> .config
	    echo CONFIG_ARCHERC7=y >> .config
	    echo CONFIG_DIR859=y >> .config
	    echo CONFIG_CPE880=y >> .config
	    echo CONFIG_MMS344=y >> .config
	    echo CONFIG_DIR862=y >> .config
	    echo CONFIG_ERC=y >> .config
	    echo CONFIG_DAP3662=y >> .config
	    echo CONFIG_DAP2230=y >> .config
	    echo CONFIG_DAP2330=y >> .config
	    echo CONFIG_JWAP606=y >> .config
	    echo CONFIG_UAPAC=y >> .config
	    echo CONFIG_XWLOCO=y >> .config
	    echo CONFIG_WR710=y >> .config
	    echo CONFIG_GL150=y >> .config
	    echo CONFIG_WR650AC=y >> .config
	    echo CONFIG_E355AC=y >> .config
	    echo CONFIG_E325N=y >> .config
	    echo CONFIG_WR615N=y >> .config
	    echo CONFIG_E380AC=y >> .config
	    echo CONFIG_XD3200=y >> .config
	    echo CONFIG_AP120C=y >> .config
	    echo CONFIG_WILLY=y >> .config
	    echo CONFIG_XWM400=y >> .config
	    echo CONFIG_ARCHERC7V4=y >> .config
	    echo CONFIG_UBNTFIX=y >> .config
	    echo CONFIG_WR810N=y >> .config
	    echo CONFIG_LIMA=y >> .config
	    echo CONFIG_PERU=y >> .config
	    echo CONFIG_RAMBUTAN=y >> .config
	    echo CONFIG_WR1043V4=y >> .config
	    echo CONFIG_WR1043V5=y >> .config
	    echo CONFIG_WA7510=y >> .config
	    make oldconfig ARCH=mips
	    sed -i 's/\CONFIG_WR841V8=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR710=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR841V9=y/ /g' .config	    
	    sed -i 's/\CONFIG_WPE72=y/ /g' .config	    
	    sed -i 's/\CONFIG_JWAP606=y/ /g' .config	    
	    sed -i 's/\CONFIG_DIR615I=y/ /g' .config	    
	    sed -i 's/\CONFIG_WA901=y/ /g' .config	    
	    sed -i 's/\CONFIG_WDR4300=y/ /g' .config	    
	    sed -i 's/\CONFIG_WDR3500=y/ /g' .config	    
	    sed -i 's/\CONFIG_WDR2543=y/ /g' .config	    
	    sed -i 's/\CONFIG_NVRAM_64K=y/ /g' .config	    
	    sed -i 's/\CONFIG_NVRAM_60K=y/ /g' .config	    
	    sed -i 's/\CONFIG_ALFANX=y/ /g' .config	    
	    sed -i 's/\CONFIG_AP135=y/ /g' .config	    
	    sed -i 's/\CONFIG_DAP3310=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR1043V2=y/ /g' .config	    
	    sed -i 's/\CONFIG_ARCHERC7=y/ /g' .config	    
	    sed -i 's/\CONFIG_DIR859=y/ /g' .config	    
	    sed -i 's/\CONFIG_CPE880=y/ /g' .config	    
	    sed -i 's/\CONFIG_MMS344=y/ /g' .config	    
	    sed -i 's/\CONFIG_DIR862=y/ /g' .config	    
	    sed -i 's/\CONFIG_ERC=y/ /g' .config	    
	    sed -i 's/\CONFIG_DAP3662=y/ /g' .config	    
	    sed -i 's/\CONFIG_DAP2230=y/ /g' .config	    
	    sed -i 's/\CONFIG_DAP2330=y/ /g' .config	    
	    sed -i 's/\CONFIG_UAPAC=y/ /g' .config	    
	    sed -i 's/\CONFIG_XWLOCO=y/ /g' .config	    
	    sed -i 's/\CONFIG_GL150=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR650AC=y/ /g' .config	    
	    sed -i 's/\CONFIG_E355AC=y/ /g' .config	    
	    sed -i 's/\CONFIG_E325N=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR615N=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR941V6=y/ /g' .config	    
	    sed -i 's/\CONFIG_E380AC=y/ /g' .config	    
	    sed -i 's/\CONFIG_XD3200=y/ /g' .config	    
	    sed -i 's/\CONFIG_AP120C=y/ /g' .config	    
	    sed -i 's/\CONFIG_WILLY=y/ /g' .config	    
	    sed -i 's/\CONFIG_XWM400=y/ /g' .config	    
	    sed -i 's/\CONFIG_ARCHERC7V4=y/ /g' .config	    
	    sed -i 's/\CONFIG_UBNTFIX=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR810N=y/ /g' .config	    
	    sed -i 's/\CONFIG_LIMA=y/ /g' .config	    
	    sed -i 's/\CONFIG_PERU=y/ /g' .config	    
	    sed -i 's/\CONFIG_RAMBUTAN=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR1043V4=y/ /g' .config	    
	    sed -i 's/\CONFIG_WR1043V4=y/ /g' .config	    
	    sed -i 's/\CONFIG_WA7510=y/ /g' .config	    
    fi
    sed -i 's/\# CONFIG_LTO_MENU is not set/ /g' .config	    
    sed -i 's/\# CONFIG_ASN1 is not set/ /g' .config	    
    cp .config $i
done
