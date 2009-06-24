#default gpio 6
rm -f images/*
make ap51 ap61 RESETBUTTON=0x06 FIS=0
mkdir images_default
cp images/*.rom images_default 

rm -f images/*
make ap61 RESETBUTTON=0x06 FIS=1
mkdir images_dir300
cp images/*.rom images_dir300 

#senao like inverted gpio 5
make ap51 RESETBUTTON=0x15 FIS=2
mkdir images_senao
cp images/*.rom images_senao 

#linksys WRT54G2 v1.1 inverted gpio 7
rm -f images/*
make ap65 RESETBUTTON=0x17
mkdir images_wrt54g2
cp images/*.rom images_wrt54g2

rm -f images/*
make ap48 RESETBUTTON=0x16 FIS=0
mkdir images_senao
cp images/*.rom images_senao

rm -f images/*
make ap48 RESETBUTTON=0x06 FIS=0
mkdir images_default
cp images/*.rom images_default


cp -rv images_senao /GruppenLW/releases/images
cp -rv images_default /GruppenLW/releases/images
cp -rv images_dir300 /GruppenLW/releases/images
cp -rv images_wrt54g2 /GruppenLW/releases/images