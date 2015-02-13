# file: mme/mme.mak

# ====================================================================
# programs;
# --------------------------------------------------------------------

hpav.o: hpav.c getoptv.h putoptv.h memory.h number.h types.h flags.h error.h qualcomm.h homeplug.h mme.h channel.h mme.h 
hpavd.o: hpavd.c getoptv.h putoptv.h memory.h number.h types.h flags.h error.h qualcomm.h homeplug.h mme.h channel.h mme.h 
mme.o: mme.c getoptv.h putoptv.h flags.h 

# ====================================================================
# fuctions files;
# --------------------------------------------------------------------

EthernetHeader.o: EthernetHeader.c homeplug.h mme.h
QualcommHeader.o: QualcommHeader.c qualcomm.h mme.h memory.h types.h endian.h
FragmentHeader.o: FragmentHeader.c mme.h homeplug.h memory.h types.h endian.h
HomePlugHeader.o: HomePlugHeader.c mme.h homeplug.h memory.h types.h endian.h

# ====================================================================
# functions;
# --------------------------------------------------------------------

ARPCPrint.o: ARPCPrint.c types.h endian.h
ARPCWrite.o: ARPCWrite.c types.h endian.h memory.h
MMEPeek.o: MMEPeek.c mme.h memory.h 
MMEMode.o: MMEMode.c mme.h
MMEName.o: MMEName.c mme.h homeplug.h 
MMECode.o: MMECode.c mme.h
UnwantedMessage.o: UnwantedMessage.c mme.h endian.h types.h 
readmessage.o: readmessage.c error.h plc.h timer.h
sendmessage.o: sendmessage.c plc.h


# ====================================================================
# files;
# --------------------------------------------------------------------

mme.h: ether.h homeplug.h qualcomm.h types.h

