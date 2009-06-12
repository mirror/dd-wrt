#default gpio 6
rm -f images/*
make ap51 ap61 RESETBUTTON=0x06
mkdir images_default
cp images/*.rom images_default 

#senao like inverted gpio 5
make ap51 RESETBUTTON=0x15
mkdir images_senao
cp images/*.rom images_senao 

#linksys WRT54G2 v1.1 inverted gpio 7
rm -f images/*
make ap65 RESETBUTTON=0x17
mkdir images_wrt54g2
cp images/*.rom images_wrt54g2