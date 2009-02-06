#include <string>
#include <string.h>
#include "Api.h"

using namespace std;

struct KAI_OPCODE
{
	char*   Opcode;
	int     SegmentCount;
};

KAI_OPCODE UIOpcodes[] = {
	{"KAI_CLIENT_ADD_CONTACT", 2},
	{"KAI_CLIENT_APP_SPECIFIC", 2},
	{"KAI_CLIENT_ARENA_BAN", 2},
	{"KAI_CLIENT_ARENA_BREAK_STREAM", 2},
	{"KAI_CLIENT_ARENA_KICK", 2},
	{"KAI_CLIENT_ARENA_PM", 3},
	{"KAI_CLIENT_ARENA_STATUS", 3},
	{"KAI_CLIENT_ATTACH", 1},
	{"KAI_CLIENT_AVATAR", 2},
	{"KAI_CLIENT_CHAT", 2},
	{"KAI_CLIENT_CHATMODE", 2},
	{"KAI_CLIENT_CREATE_VECTOR", 4},
	{"KAI_CLIENT_DETACH", 1},
	{"KAI_CLIENT_DISCOVER", 1},
	{"KAI_CLIENT_GET_METRICS", 1},
	{"KAI_CLIENT_GET_PROFILE", 2},
	{"KAI_CLIENT_GET_VECTORS", 2},
	{"KAI_CLIENT_GETSTATE", 1},
	{"KAI_CLIENT_LOGIN", 3},
	{"KAI_CLIENT_PM", 3},
	{"KAI_CLIENT_REMOVE_CONTACT", 2},
	{"KAI_CLIENT_SPECIFIC_COUNT", 2},
	{"KAI_CLIENT_SPEEX", 1},	// Special case, doesn't finish with ";"
	{"KAI_CLIENT_SPEEX_OFF", 2},
	{"KAI_CLIENT_SPEEX_ON", 2},
	{"KAI_CLIENT_TAKEOVER", 1},
	{"KAI_CLIENT_VECTOR", 3},
	{"KAI_CLIENT_CAPS", 2},
	{"KAI_CLIENT_INVITE", 3}, 
	{NULL, 0}
};

KAI_UI_OPCODE MsgToUIOpcode(string msg)
{
	int i = 0;
	while(UIOpcodes[i].Opcode)
	{
		if (strcmp(UIOpcodes[i].Opcode, msg.c_str()) == 0)
			return i;
		i++;
	}
	return -1;
}

int SegDiff(KAI_UI_OPCODE id, unsigned int segcount)
{
	return (UIOpcodes[id].SegmentCount - segcount);
}
