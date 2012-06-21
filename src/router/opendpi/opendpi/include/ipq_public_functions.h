/*
 * ipq_public_functions.h
 * Copyright (C) 2009-2011 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#ifndef __IPOQUE_API_INCLUDE_FILE__
#error CANNOT INCLUDE THIS .H FILE, INCLUDE IPQ_API.H
#endif

#ifndef __IPQ_PUBLIC_FUNCTIONS_H__
#define __IPQ_PUBLIC_FUNCTIONS_H__

#ifdef __cplusplus
extern "C" {
#endif
	/**
	 * struct for a unique ipv4 flow address
	 */
	typedef struct ipoque_unique_flow_ipv4_address_struct {
		/**
		 * lower ip
		 */
		u32 lower_ip;
		/**
		 * upper ip
		 */
		u32 upper_ip;
		/* we need 3 dummies to fill up to ipv6 address size */
		/**
		 * this is only needed to become the same size like a unique ipv6 struct
		 */
		u64 dummy[3];
	} ipoque_unique_flow_ipv4_address_struct_t;

	/**
	 * struct for a unique ipv6 flow address
	 */
	typedef struct ipoque_unique_flow_ipv6_address_struct {
		/**
		 * lower ip
		 */
		u64 lower_ip[2];
		/**
		 * upper ip
		 */
		u64 upper_ip[2];
	} ipoque_unique_flow_ipv6_address_struct_t;

	/**
	 * struct for a unique ipv4 and ipv6 5-tuple (ip,ip,port,port,protocol)
	 */
	typedef struct ipoque_unique_flow_ipv4_and_6_struct {
		/* only ip addresses are different, to minimize compare operations for hash tables, store ipv4 or ipv6 always in the first bit */
		/**
		 * saves if it is a ipv6, if it false so it is a ipv4
		 */
		u16 is_ip_v6;
		/**
		 * the protocol, 16 bit wide for alignemt reasons
		 */
		u16 protocol;			/* for alignment reason, protocol is 16 bit, not 8 bit */
		/**
		 * the port of the lower ip address
		 */
		u16 lower_port;
		/**
		 * the port of the upper ip address
		 */
		u16 upper_port;
		union {
			/**
			 * the ipv4 flow address struct. use the same memory area like ipv6 (union)
			 */
			struct ipoque_unique_flow_ipv4_address_struct ipv4;
			/**
			 * the ipv6 flow address struct. use the same memory area like ipv4 (union)
			 */
			struct ipoque_unique_flow_ipv6_address_struct ipv6;
		} ip;
	} ipoque_unique_flow_ipv4_and_6_struct_t;

	typedef enum {

		IPQ_LOG_ERROR,
		IPQ_LOG_TRACE,
		IPQ_LOG_DEBUG
	} ipq_log_level_t;

	typedef void (*ipoque_debug_function_ptr) (u32 protocol,
											   void *module_struct, ipq_log_level_t log_level, const char *format, ...);

	/**
	 * This function returns the size of the flow struct
	 * @return the size of the flow struct
	 */
	u32 ipoque_detection_get_sizeof_ipoque_flow_struct(void);

	/**
	 * This function returns the size of the id struct
	 * @return the size of the id struct
	 */
	u32 ipoque_detection_get_sizeof_ipoque_id_struct(void);


	/**
	 * This function returns a new initialized detection module. 
	 * @param ticks_per_second the timestamp resolution per second (like 1000 for millisecond resolution)
	 * @param ipoque_malloc function pointer to a memory allocator
	 * @param ipoque_debug_printf a function pointer to a debug output function, use NULL in productive envionments
	 * @return the initialized detection module
	 */
	struct ipoque_detection_module_struct *ipoque_init_detection_module(u32 ticks_per_second, void
																		*(*ipoque_malloc)
																		 (unsigned
																		  long size),
																		ipoque_debug_function_ptr ipoque_debug_printf);
	/**
	 * This function destroys the detection module
	 * @param ipoque_struct the to clearing detection module
	 * @param ipoque_free function pointer to a memory free function
	 */
	void
	 ipoque_exit_detection_module(struct ipoque_detection_module_struct
								  *ipoque_struct, void (*ipoque_free) (void *ptr));

	/**
	 * This function sets the protocol bitmask2
	 * @param ipoque_struct the detection module
	 * @param detection_bitmask the protocol bitmask
	 */
	void
	 ipoque_set_protocol_detection_bitmask2(struct
											ipoque_detection_module_struct
											*ipoque_struct, const IPOQUE_PROTOCOL_BITMASK * detection_bitmask);
	/**
	 * This function will processes one packet and returns the ID of the detected protocol.
	 * This is the main packet processing function. 
	 *
	 * @param ipoque_struct the detection module
	 * @param flow void pointer to the connection state machine
	 * @param packet the packet as unsigned char pointer with the length of packetlen. the pointer must point to the Layer 3 (IP header)
	 * @param packetlen the length of the packet
	 * @param current_tick the current timestamp for the packet
	 * @param src void pointer to the source subscriber state machine
	 * @param dst void pointer to the destination subscriber state machine
	 * @return returns the detected ID of the protocol
	 */
	unsigned int
	 ipoque_detection_process_packet(struct ipoque_detection_module_struct
									 *ipoque_struct, void *flow,
									 const unsigned char *packet,
									 const unsigned short packetlen,
									 const IPOQUE_TIMESTAMP_COUNTER_SIZE current_tick, void *src, void *dst);

#define IPOQUE_DETECTION_ONLY_IPV4 ( 1 << 0 )
#define IPOQUE_DETECTION_ONLY_IPV6 ( 1 << 1 )

	/**
	 * query the pointer to the layer 4 packet
	 *
	 * @param l3 pointer to the layer 3 data
	 * @param l3_len length of the layer 3 data
	 * @param l4_return filled with the pointer the layer 4 data if return value == 0, undefined otherwise
	 * @param l4_len_return filled with the length of the layer 4 data if return value == 0, undefined otherwise
	 * @param l4_protocol_return filled with the protocol of the layer 4 data if return value == 0, undefined otherwise
	 * @param flags limit operation on ipv4 or ipv6 packets, possible values are IPOQUE_DETECTION_ONLY_IPV4 or IPOQUE_DETECTION_ONLY_IPV6; 0 means any
	 * @return 0 if correct layer 4 data could be found, != 0 otherwise
	 */
	u8 ipoque_detection_get_l4(const u8 * l3, u16 l3_len, const u8 ** l4_return, u16 * l4_len_return,
							   u8 * l4_protocol_return, u32 flags);
	/**
	 * build the unique key of a flow
	 *
	 * @param l3 pointer to the layer 3 data
	 * @param l3_len length of the layer 3 data
	 * @param l4 pointer to the layer 4 data
	 * @param l4_len length of the layer 4 data
	 * @param l4_protocol layer 4 protocol
	 * @param key_return filled with the unique key if return value == 0, undefined otherwise
	 * @param dir_return filled with a direction flag (0 or 1), can be NULL
	 * @param flags limit operation on ipv4 or ipv6 packets, possible values are IPOQUE_DETECTION_ONLY_IPV4 or IPOQUE_DETECTION_ONLY_IPV6; 0 means any
	 * @return 0 if key could be built, != 0 otherwise
	 */
	u8 ipoque_detection_build_key(const u8 * l3, u16 l3_len, const u8 * l4, u16 l4_len, u8 l4_protocol,
								  struct ipoque_unique_flow_ipv4_and_6_struct *key_return, u8 * dir_return, u32 flags);
	/**
	 * returns the real protocol for the flow of the last packet given to the detection.
	 * if no real protocol could be found, the unknown protocol will be returned.
	 * 
	 * @param ipoque_struct the detection module
	 * @return the protocol id of the last real protocol found in the protocol history of the flow
	 */
	u16 ipoque_detection_get_real_protocol_of_flow(struct ipoque_detection_module_struct *ipoque_struct);

	/**
	 * returns true if the protocol history of the flow of the last packet given to the detection
	 * contains the given protocol.
	 * 
	 * @param ipoque_struct the detection module
	 * @return 1 if protocol has been found, 0 otherwise
	 */
	u8 ipoque_detection_flow_protocol_history_contains_protocol(struct ipoque_detection_module_struct *ipoque_struct,
																u16 protocol_id);
#ifdef HAVE_NTOP
  unsigned int ntop_find_port_based_protocol(u8 proto, u32 shost, u16 sport, u32 dhost, u16 dport);  
  unsigned int ntop_guess_undetected_protocol(u8 proto, u32 shost, u16 sport, u32 dhost, u16 dport);
  char* ntop_strnstr(const char *s, const char *find, size_t slen);
  int matchStringProtocol(struct ipoque_detection_module_struct *ipoque_struct, char *string_to_match, u_int string_to_match_len);
#endif

#ifdef __cplusplus
}
#endif
#endif
