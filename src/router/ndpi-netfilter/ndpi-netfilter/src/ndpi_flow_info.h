#ifndef FLOW_DATA_H
#define FLOW_DATA_H

struct flow_data_common {
	uint8_t			rec_type:2, // 0 - proto name,
					    // 1 - start, 2 - flow, 3 - lost traffic
				family:1,   // 0 - ipv4, 1 - ipv6
				nat_flags:3,// 1 - snat, 2 - dnat, 4 - userid
				extflag:2,  // 0 - old, 1 - new, 2,3 - reserved
				proto,      // transport protocol
				opt_len,    // option length (0 - none)
				host_len;   // host name length (0 - none)
					    // cert_len + host_len < 510;
					    // 4 bytes
	uint32_t		time_start; // 4 bytes
	/* 0 - in, 1 - out */
	uint32_t		p[2]; // 8 bytes
	uint64_t		b[2]; // 16 bytes
	uint32_t		time_end; // 4 bytes

	uint16_t		ifidx,ofidx; // 4 bytes
	uint16_t		proto_master,proto_app; // 4 bytes
	uint32_t		connmark; // 4 bytes
} __attribute ((packed)); // 48 bytes

struct flow_data_v4 {
	uint32_t		ip_s,ip_d; // 8 bytes
	uint32_t		ip_snat,ip_dnat; // 8 bytes
	uint16_t		sport,dport,sport_nat,dport_nat; // 8 bytes
}  __attribute ((packed)); // 24 bytes

struct flow_data_v6 {
	uint8_t			ip_s[16],ip_d[16]; // 32 bytes
	uint16_t		sport,dport; // 4 bytes
}  __attribute ((packed)); // 36 bytes

struct flow_data {
	struct flow_data_common	c;
	union {
		struct flow_data_v4 v4;
		struct flow_data_v6 v6;
	} d;
} __attribute ((packed));

#define flow_data_v4_size (sizeof(struct flow_data_common) + sizeof(struct flow_data_v4))
#define flow_data_v6_size (sizeof(struct flow_data_common) + sizeof(struct flow_data_v6))

/*
 * start record: 8 bytes (rec_type+family,cert_len,host_len,nat_flags,time_start)
 *
 * ipv4 info: >= 72 bytes
 *   flow_data_common + flow_data_v4  + cert_name_len + host_name_len 
 * 
 * ipv6 info: >= 84 bytes
 *   flow_data_common + flow_data_v6  + cert_name_len + host_name_len 
 *
 * lost trafic record: 48 bytes
 *
 * name of protocols records: 4 bytes + proto_name_len
 *          byte 0: 0,
 *          byte 1: proto_id & 0xff,
 *          byte 2: proto_id >> 8,
 *          byte 3: proto_name_len
 *          
 */
#endif
