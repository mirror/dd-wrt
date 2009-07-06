#default gpio 6
rm -f images/*
make ap51 ap61 RESETBUTTON=0x06 FIS=0
mkdir images_default
cp images/*.rom images_default 

rm -f images/*
make ap61 RESETBUTTON=0x06 FIS=1
mkdir images_dir300
cp images/*.rom images_dir300 


rm -f images/*
make ap61 RESETBUTTON=0x06 FIS=0 LED1_PIN=0 LED2_PIN=1 LED3_PIN=3 LED4_PIN=4
mkdir images_ubnt
cp images/redboot_ap61_16M_4M_kendin.rom images_ubnt/BS2_LC2.rom
cp images/redboot_ap61_32M_8M_kendin.rom images_ubnt/PICO2.rom

rm -f images/*
make ap51 RESETBUTTON=0x06 FIS=0 LED1_PIN=0 LED2_PIN=1 LED3_PIN=3 LED4_PIN=4
mkdir images_ubnt
cp images/redboot_ap51_16M_4M_icplus.rom images_ubnt/LS2_PS2.rom
cp images/redboot_ap51_16M_4M_kendin.rom images_ubnt/NS2.rom

rm -f images/*
make ap48 RESETBUTTON=0x06 FIS=0 LED1_PIN=7 LED2_PIN=5 LED3_PIN=4 LED4_PIN=3
mkdir images_ubnt
cp images/redboot_ap48_16M_4M_kendin.rom images_ubnt/LS5_NS5_PS5_BS5_LC5_PICO5.rom

#senao like inverted gpio 5
rm -f images/*
make ap51 RESETBUTTON=0x15 FIS=2  LED1_PIN=4 LED2_PIN=6 LED3_PIN=7
mkdir images_senao
cp images/*.rom images_senao 

#linksys WRT54G2 v1.1 inverted gpio 7
rm -f images/*
make ap65 RESETBUTTON=0x17
mkdir images_wrt54g2
cp images/*.rom images_wrt54g2

rm -f images/*
make ap48 RESETBUTTON=0x16 FIS=0 LED1_PIN=0 LED2_PIN=2 LED3_PIN=1
mkdir images_senao
cp images/*.rom images_senao

rm -f images/*
make ap48 RESETBUTTON=0x06 FIS=0
mkdir images_default
cp images/*.rom images_default


cp -rv images_senao /GruppenLW/releases/images
cp -rv images_default /GruppenLW/releases/images
cp -rv images_dir300 /GruppenLW/releases/images
cp -rv images_ubnt /GruppenLW/releases/images
cp -rv images_wrt54g2 /GruppenLW/releases/images