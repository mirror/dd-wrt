#ifndef	_API_H_
#define	_API_H_

typedef int KAI_UI_OPCODE;

#define	KAI_CLIENT_ADD_CONTACT			0 	// ;user;
#define	KAI_CLIENT_APP_SPECIFIC			1	// ;question;
#define	KAI_CLIENT_ARENA_BAN			2	// ;user;
#define	KAI_CLIENT_ARENA_BREAK_STREAM	3	// ;user;
#define	KAI_CLIENT_ARENA_KICK			4	// ;user;
#define	KAI_CLIENT_ARENA_PM				5	// ;user;message;
#define	KAI_CLIENT_ARENA_STATUS			6	// ;status;players;
#define	KAI_CLIENT_ATTACH				7
#define	KAI_CLIENT_AVATAR				8	// ;user;
#define	KAI_CLIENT_CHAT					9	// ;msg;
#define	KAI_CLIENT_CHATMODE				10	// ;chatroom;
#define	KAI_CLIENT_CREATE_VECTOR		11	// ;password;maxplayers;description;
#define	KAI_CLIENT_DETACH				12
#define	KAI_CLIENT_DISCOVER				13
#define	KAI_CLIENT_GET_METRICS			14
#define	KAI_CLIENT_GET_PROFILE			15	// ;user;
#define	KAI_CLIENT_GET_VECTORS			16	// ;vector;
#define	KAI_CLIENT_GETSTATE				17
#define	KAI_CLIENT_LOGIN				18	// ;user;password;
#define	KAI_CLIENT_PM					19	// ;user;message;
#define	KAI_CLIENT_REMOVE_CONTACT		20	// ;user;
#define	KAI_CLIENT_SPECIFIC_COUNT		21	// ;vector;
#define	KAI_CLIENT_SPEEX				22	// ;<BINARY>
#define	KAI_CLIENT_SPEEX_OFF			23	// ;user;
#define	KAI_CLIENT_SPEEX_ON				24	// ;user;
#define	KAI_CLIENT_TAKEOVER				25
#define	KAI_CLIENT_VECTOR				26	// ;vector;password;
#define KAI_CLIENT_CAPS				27	// ;caps;
#define KAI_CLIENT_INVITE			28	// ;from;timestamp;

KAI_UI_OPCODE MsgToUIOpcode(std::string msg);
int SegDiff(KAI_UI_OPCODE id, unsigned int segcount);

#endif
