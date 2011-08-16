s/Packet /SFSnortPacket /
s/rules\.h/signature.h/
/signature.h/ a\
#include "sf_snort_packet.h" \
#include "event.h"
s/RspFpList/void/
s/OutputFuncNode/void/
s/TagData/void/
s/RuleType/int/
s/IpAddrSet/void/
s/PortObject/void/
s/ActivateListNode/void/
s/struct _ListHead/void/
/sfutil\/sfghash\.h/d
/sf_types\.h/d
s/SFGHASH/void/g
