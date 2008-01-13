#ifndef _IP_CT_CUSEEME
#define _IP_CT_CUSEEME

#define CUSEEME_PORT 7648

/* These structs come from the 2.2 ip_masq_cuseeme code... */

#pragma pack(1)
/* CuSeeMe data header */
struct cu_header {
	u_int16_t	dest_family;
	u_int16_t	dest_port;
	u_int32_t	dest_addr;
	int16_t		family;
	u_int16_t	port;
	u_int32_t	addr;
	u_int32_t	seq;
	u_int16_t	msg;
	u_int16_t	data_type;
				/* possible values:
				 * 1	small video
				 * 2	big video
				 * 3	audio
				 * 100	acknowledge connectivity when there
				 * 	is nothing else to send
				 * 101	OpenContinue packet
				 * 104	display a text message and 
				 * 	disconnect (used by reflector to
				 * 	kick clients off)
				 * 105	display a text message (welcome
				 * 	message from reflector)
				 * 106	exchanged among reflectors for
				 * 	reflector interoperation
				 * 107	carry aux stream data when there is
				 *	no video to piggy-back on
				 * 108	obsolete (used in Mac alpha version)
				 * 109	obsolete (used in Mac alpha version)
				 * 110	used for data rate control
				 * 111	used for data rate control
				 * 256	aux data control messages
				 * 257	aux data packets
				 * */
	u_int16_t	packet_len;
};

/* Open Continue Header */
struct oc_header {
	struct cu_header	cu_head;
	u_int16_t	client_count; /* Number of client info structs */
	u_int32_t	seq_no;
	char		user_name[20];
	char		stuff[4];     /* Flags, version stuff, etc */
};

/* Client info structures */
struct client_info {
	u_int32_t	address;      /* Client address */
	char		stuff[8];     /* Flags, pruning bitfield, packet counts, etc */
};
#pragma pack()

/* This structure is per expected connection */
struct ip_ct_cuseeme_expect {
};

/* This structure exists only once per master */
struct ip_ct_cuseeme_master {
};

#endif /* _IP_CT_CUSEEME */
