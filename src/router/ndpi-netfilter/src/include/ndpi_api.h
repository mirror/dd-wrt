/*
 * ndpi_api.h
 *
 * Copyright (C) 2011-20 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef __NDPI_API_H__
#define __NDPI_API_H__

#include "ndpi_main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SIZEOF_FLOW_STRUCT                    ( sizeof(struct ndpi_flow_struct) )

#define NDPI_DETECTION_ONLY_IPV4              ( 1 << 0 )
#define NDPI_DETECTION_ONLY_IPV6              ( 1 << 1 )

#define ADD_TO_DETECTION_BITMASK              1
#define NO_ADD_TO_DETECTION_BITMASK           0
#define SAVE_DETECTION_BITMASK_AS_UNKNOWN     1
#define NO_SAVE_DETECTION_BITMASK_AS_UNKNOWN  0

//  extern int ndpi_debug_print_level;
  /*
    In case a custom DGA function is used, the fucntion
    below must be overwritten,
  */
//  extern ndpi_custom_dga_predict_fctn ndpi_dga_function;

//  extern ndpi_debug_function_ptr ndpi_debug_print_init;
//  extern ndpi_log_level_t ndpi_debug_level_init;
  /**
   * Check if a string is encoded with punycode
   * ( https://tools.ietf.org/html/rfc3492 )
   *
   * @par    buff = pointer to the string to ckeck
   * @par    len  = len of the string
   * @return 1 if the string is punycoded;
   *         else 0
   *
   */
  NDPI_STATIC int ndpi_check_punycode_string(char *buff, int len);


  /**
   * Get the size of the flow struct
   *
   * @return the size of the flow struct
   *
   */
 NDPI_STATIC  u_int32_t ndpi_detection_get_sizeof_ndpi_flow_struct(void);


  /**
   * Get the size of the flow tcp struct
   *
   * @return the size of the flow tcp struct
   *
   */
NDPI_STATIC   u_int32_t ndpi_detection_get_sizeof_ndpi_flow_tcp_struct(void);


  /**
   * Get the size of the flow udp struct
   *
   * @return the size of the flow udp struct
   *
   */
NDPI_STATIC   u_int32_t ndpi_detection_get_sizeof_ndpi_flow_udp_struct(void);

  /*
    Same as the API call above but used for matching raw id's added
    via ndpi_add_string_value_to_automa()
  */
NDPI_STATIC   int ndpi_match_string_value(void *_automa, char *string_to_match,
			      u_int match_len, u_int32_t *num);

  /**
   * Return the protocol error code of a given flow
   *
   * @par    flow    = the flow to analyze
   * @return the error code or 0 otherwise
   *                        
   */
NDPI_STATIC   u_int32_t ndpi_get_flow_error_code(struct ndpi_flow_struct *flow);
  
  /**
   * nDPI personal allocation and free functions
   **/
NDPI_STATIC   void * ndpi_malloc(size_t size);
NDPI_STATIC   void * ndpi_calloc(unsigned long count, size_t size);
NDPI_STATIC   void * ndpi_realloc(void *ptr, size_t old_size, size_t new_size);
NDPI_STATIC   char * ndpi_strdup(const char *s);
NDPI_STATIC   void   ndpi_free(void *ptr);
NDPI_STATIC   void * ndpi_flow_malloc(size_t size);
NDPI_STATIC   void   ndpi_flow_free(void *ptr);
NDPI_STATIC   u_int32_t ndpi_get_tot_allocated_memory(void);

  /**
   * Search the first occurrence of substring -find- in -s-
   * The search is limited to the first -slen- characters of the string
   *
   * @par    s     = string to parse
   * @par    find  = string to match with -s-
   * @par    slen  = max length to match between -s- and -find-
   * @return a pointer to the beginning of the located substring;
   *         NULL if the substring is not found
   *
   */
NDPI_STATIC   char* ndpi_strnstr(const char *s, const char *find, size_t slen);

  /**
   * Same as ndpi_strnstr but case insensitive
   *
   * @par    s     = string to parse
   * @par    find  = string to match with -s-
   * @par    slen  = max length to match between -s- and -find-
   * @return a pointer to the beginning of the located substring;
   *         NULL if the substring is not found
   *
   */
NDPI_STATIC   const char* ndpi_strncasestr(const char *s, const char *find, size_t slen);

  /**
   * Returns the nDPI protocol id for IP-based protocol detection
   *
   * @par    ndpi_struct  = the struct created for the protocol detection
   * @par    pin          = IP host address (MUST BE in network byte order):
   *                        See man(7) ip for details
   * @return the nDPI protocol ID
   *
   */
NDPI_STATIC   u_int16_t ndpi_network_ptree_match(struct ndpi_detection_module_struct *ndpi_struct,
				     struct in_addr *pin);

  /**
   * Returns the nDPI protocol id for IP+port-based protocol detection
   *
   * @par    ndpi_struct  = the struct created for the protocol detection
   * @par    pin          = IP host address (MUST BE in network byte order):
   *                        See man(7) ip for details
   * @par    port         = The port (MUST BE in network byte order) or
   *                        0 if ignored
   * @return the nDPI protocol ID
   *
   */
NDPI_STATIC   u_int16_t ndpi_network_port_ptree_match(struct ndpi_detection_module_struct *ndpi_struct,
					  struct in_addr *pin /* network byte order */,
					  u_int16_t port /* network byte order */);

  /**
   * Init single protocol match
   *
   * @par ndpi_mod  = the struct created for the protocol detection
   * @par match     = the struct passed to match the protocol
   *
   * @return  0 - success, -1 - error
   *
   */
NDPI_STATIC   int ndpi_init_protocol_match(struct ndpi_detection_module_struct *ndpi_mod,
				ndpi_protocol_match *match);

  /**
   * Returns a new initialized detection module
   * Note that before you can use it you can still load
   * hosts and do other things. As soon as you are ready to use
   * it do not forget to call first ndpi_finalize_initialization()
   *
   * You can call this function multiple times, (i.e. to create multiple
   * indipendent detection contexts) but all these calls MUST NOT run
   * in parallel
   *
   * @par prefs = load preferences
   * @return  the initialized detection module
   *
   */
NDPI_STATIC   struct ndpi_detection_module_struct *ndpi_init_detection_module(ndpi_init_prefs prefs);

  /**
   * Completes the initialization (2nd step)
   *
   * @par ndpi_str = the struct created for the protocol detection
   *
   */
NDPI_STATIC   void ndpi_finalize_initialization(struct ndpi_detection_module_struct *ndpi_str);

  /**
   * Frees the dynamic memory allocated members in the specified flow
   *
   * @par flow  = the flow struct which dynamic allocated members should be deallocated
   *
   */
NDPI_STATIC   void ndpi_free_flow_data(struct ndpi_flow_struct *flow);

  /**
   * Frees the dynamic memory allocated members in the specified flow and the flow struct itself
   *
   * @par flow  = the flow struct and its dynamic allocated members that should be deallocated
   *
   */
NDPI_STATIC   void ndpi_free_flow(struct ndpi_flow_struct *flow);

  /**
   * Enables cache support.
   * In nDPI is used for some protocol (i.e. Skype)
   *
   * @par ndpi_mod  = the struct created for the protocol detection
   * @par host      = string for the host name
   * @par port      = unsigned int for the port number
   *
   */
NDPI_STATIC   void ndpi_enable_cache(struct ndpi_detection_module_struct *ndpi_mod,
			 char* host, u_int port);

  /**
   * Destroys the detection module
   *
   * @par ndpi_struct  = the struct to clearing for the detection module
   *
   */
NDPI_STATIC   void ndpi_exit_detection_module(struct ndpi_detection_module_struct *ndpi_struct);

  /**
   * Sets a single protocol bitmask
   * This function does not increment the index of the callback_buffer
   *
   * @par label                    = string for the protocol name
   * @par ndpi_struct              = the detection module
   * @par idx                      = the index of the callback_buffer
   * @par func                     = function pointer of the protocol search
   * @par ndpi_selection_bitmask   = the protocol selected bitmask
   * @par b_save_bitmask_unknow    = if set as "true" save the detection bitmask as unknow
   * @par b_add_detection_bitmask  = if set as "true" add the protocol bitmask to the detection bitmask
   *
   */
NDPI_STATIC   void ndpi_set_bitmask_protocol_detection(char *label,
					   struct ndpi_detection_module_struct *ndpi_struct,
					   const u_int32_t idx,
					   u_int16_t ndpi_protocol_id,
					   void (*func) (struct ndpi_detection_module_struct *,
							 struct ndpi_flow_struct *flow),
					   const NDPI_SELECTION_BITMASK_PROTOCOL_SIZE ndpi_selection_bitmask,
					   u_int8_t b_save_bitmask_unknow,
					   u_int8_t b_add_detection_bitmask);

  /**
   * Sets the protocol bitmask2
   *
   * @par ndpi_struct        = the detection module
   * @par detection_bitmask  = the protocol bitmask to set
   *
   */
NDPI_STATIC  int ndpi_set_protocol_detection_bitmask2(struct ndpi_detection_module_struct *ndpi_struct,
 					    const NDPI_PROTOCOL_BITMASK * detection_bitmask);

  /**
   *  Function to be called before we give up with detection for a given flow.
   *  This function reduces the NDPI_UNKNOWN_PROTOCOL detection
   *
   * @par    ndpi_struct  = the detection module
   * @par    flow         = the flow given for the detection module
   * @par    enable_guess = guess protocol if unknown
   * @par    protocol_was_guessed = 1 if the protocol was guesses (requires enable_guess = 1), 0 otherwise
   * @return the detected protocol even if the flow is not completed;
   *
   */
NDPI_STATIC   ndpi_protocol ndpi_detection_giveup(struct ndpi_detection_module_struct *ndpi_struct,
				      struct ndpi_flow_struct *flow,
				      u_int8_t enable_guess,
				      u_int8_t *protocol_was_guessed);

  /**
   * Processes an extra packet in order to get more information for a given protocol
   * (like SSL getting both client and server certificate even if we already know after
   * seeing the client certificate what the protocol is)
   *
   * @par    ndpi_struct    = the detection module
   * @par    flow           = pointer to the connection state machine
   * @par    packet         = unsigned char pointer to the Layer 3 (IP header)
   * @par    packetlen      = the length of the packet
   * @par    packet_time_ms = the current timestamp for the packet (expressed in msec)
   * @return void
   *
   */
NDPI_STATIC void ndpi_process_extra_packet(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *flow,
				 const unsigned char *packet,
				 const unsigned short packetlen,
				 const u_int64_t packet_time_ms,
				 const struct ndpi_flow_input_info *input_info);

  /**
   * Processes one packet and returns the ID of the detected protocol.
   * This is the MAIN PACKET PROCESSING FUNCTION.
   *
   * @par    ndpi_struct    = the detection module
   * @par    flow           = pointer to the connection state machine
   * @par    packet         = unsigned char pointer to the Layer 3 (IP header)
   * @par    packetlen      = the length of the packet
   * @par    packet_time_ms = the current timestamp for the packet (expressed in msec)
   * @return the detected ID of the protocol
   *
   */
NDPI_STATIC  ndpi_protocol ndpi_detection_process_packet(struct ndpi_detection_module_struct *ndpi_struct,
					      struct ndpi_flow_struct *flow,
					      const unsigned char *packet,
					      const unsigned short packetlen,
					      const u_int64_t packet_time_ms,
					      const struct ndpi_flow_input_info *input_info);
  /**
   * Get the main protocol of the passed flows for the detected module
   *
   *
   * @par    ndpi_struct  = the detection module
   * @par    flow         = the flow given for the detection module
   * @return the ID of the master protocol detected
   *
   */
NDPI_STATIC   u_int16_t ndpi_get_flow_masterprotocol(struct ndpi_detection_module_struct *ndpi_struct,
					 struct ndpi_flow_struct *flow);

  /**
   * Get the app protocol of the passed flows for the detected module
   *
   *
   * @par    ndpi_struct  = the detection module
   * @par    flow         = the flow given for the detection module
   * @return the ID of the app protocol detected
   *
   */
NDPI_STATIC   u_int16_t ndpi_get_flow_appprotocol(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow);

  /**
   * Get the category of the passed flows for the detected module
   *
   *
   * @par    ndpi_struct  = the detection module
   * @par    flow         = the flow given for the detection module
   * @return the ID of the category
   *
   */
NDPI_STATIC   ndpi_protocol_category_t ndpi_get_flow_category(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow);

  /**
   * Get the ndpi protocol data of the passed flows for the detected module
   *
   *
   * @par    ndpi_struct  = the detection module
   * @par    flow         = the flow given for the detection module
   * @par    ndpi_proto   = the output struct where to store the requested information
   *
   */
NDPI_STATIC   void ndpi_get_flow_ndpi_proto(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow,
                    struct ndpi_proto * ndpi_proto);

  /**
   * API call that is called internally by ndpi_detection_process_packet or by apps
   * that want to avoid calling ndpi_detection_process_packet as they have already
   * parsed the packet and thus want to avoid this.
   *
   *
   * @par    ndpi_struct              = the detection module
   * @par    flow                     = the flow given for the detection module
   * @par    ndpi_selection_bitmask   = the protocol selected bitmask
   * @return number of protocol dissector that have been tried (0 = no more dissectors)
   *
   */
NDPI_STATIC   u_int32_t ndpi_check_flow_func(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *flow,
				 NDPI_SELECTION_BITMASK_PROTOCOL_SIZE *ndpi_selection_packet);


  /**
   * Query the pointer to the layer 4 packet
   *
   * @par    l3 = pointer to the layer 3 data
   * @par    l3_len = length of the layer 3 data
   * @par    l4_return = address to the pointer of the layer 4 data if return value == 0, else undefined
   * @par    l4_len_return = length of the layer 4 data if return value == 0, else undefined
   * @par    l4_protocol_return = protocol of the layer 4 data if return value == 0, undefined otherwise
   * @par    flags = limit operation on ipv4 or ipv6 packets. Possible values: NDPI_DETECTION_ONLY_IPV4 - NDPI_DETECTION_ONLY_IPV6 - 0 (any)
   * @return 0 if layer 4 data could be found correctly;
   else != 0
   *
   */
NDPI_STATIC   u_int8_t ndpi_detection_get_l4(const u_int8_t *l3, u_int16_t l3_len, const u_int8_t **l4_return, u_int16_t *l4_len_return,
				 u_int8_t *l4_protocol_return, u_int32_t flags);

  /**
   * Search and return the protocol based on matched ports
   *
   * @par    ndpi_struct  = the detection module
   * @par    shost        = source address in host byte order
   * @par    sport        = source port number
   * @par    dhost        = destination address in host byte order
   * @par    dport        = destination port number
   * @return the struct ndpi_protocol that match the port base protocol
   *
   */
NDPI_STATIC   ndpi_protocol ndpi_find_port_based_protocol(struct ndpi_detection_module_struct *ndpi_struct/* , u_int8_t proto */,
					      u_int32_t shost,
					      u_int16_t sport,
					      u_int32_t dhost,
					      u_int16_t dport);
  /**
   * Search and return the protocol guessed that is undetected
   *
   * @par    ndpi_struct  = the detection module
   * @par    flow         = the flow we're trying to guess, NULL if not available
   * @par    proto        = the l4 protocol number
   * @return the struct ndpi_protocol that match the port base protocol
   *
   */
NDPI_STATIC   ndpi_protocol ndpi_guess_undetected_protocol(struct ndpi_detection_module_struct *ndpi_struct,
					       struct ndpi_flow_struct *flow,
					       u_int8_t proto);
  /**
   * Check if the string passed match with a protocol
   *
   * @par    ndpi_struct         = the detection module
   * @par    string_to_match     = the string to match
   * @par    string_to_match_len = the length of the string
   * @par    ret_match           = completed returned match information
   * @return the ID of the matched subprotocol;
   *         -1 if automa is not finalized;
   *         -2 if automa==NULL or string_to_match==NULL or empty string_to_match
   *
   */
NDPI_STATIC   int ndpi_match_string_subprotocol(struct ndpi_detection_module_struct *ndpi_struct,
				    char *string_to_match,
				    u_int string_to_match_len,
				    ndpi_protocol_match_result *ret_match);
  /**
   * Check if the host passed match with a protocol
   *
   * @par    ndpi_struct         = the detection module
   * @par    flow                = the flow where match the host
   * @par    string_to_match     = the string to match
   * @par    string_to_match_len = the length of the string
   * @par    ret_match           = completed returned match information
   * @par    master_protocol_id  = value of the ID associated to the master protocol detected
   * @return the ID of the matched subprotocol
   *
   */
NDPI_STATIC   u_int16_t ndpi_match_host_subprotocol(struct ndpi_detection_module_struct *ndpi_struct,
					struct ndpi_flow_struct *flow,
					char *string_to_match,
					u_int string_to_match_len,
					ndpi_protocol_match_result *ret_match,
					u_int16_t master_protocol_id);

  /**
   * Check if the string content passed match with a protocol
   *
   * @par    ndpi_struct         = the detection module
   * @par    flow                = the flow where match the host
   * @par    subprotocol_id      = subprotocol id
   */
NDPI_STATIC   void ndpi_check_subprotocol_risk(struct ndpi_detection_module_struct *ndpi_str,
				 struct ndpi_flow_struct *flow, u_int16_t subprotocol_id);

  /**
   * Exclude protocol from search
   *
   * @par    ndpi_struct         = the detection module
   * @par    flow                = the flow where match the host
   * @par    master_protocol_id  = value of the ID associated to the master protocol detected
   *
   */
NDPI_STATIC   void ndpi_exclude_protocol(struct ndpi_detection_module_struct *ndpi_struct,
			     struct ndpi_flow_struct *flow,
			     u_int16_t master_protocol_id,
			     const char *_file, const char *_func,int _line);
  /**
   * Check if the string -bigram_to_match- match with a bigram of -automa-
   *
   * @par     ndpi_mod         = the detection module
   * @par     automa           = the struct ndpi_automa for the bigram
   * @par     bigram_to_match  = the bigram string to match
   * @return  0
   *
   */
NDPI_STATIC   int ndpi_match_bigram(const char *bigram_to_match);

  /**
   * Write the protocol name in the buffer -buf- as master_protocol.protocol
   *
   * @par     ndpi_mod      = the detection module
   * @par     proto         = the struct ndpi_protocol contain the protocols name
   * @par     buf           = the buffer to write the name of the protocols
   * @par     buf_len       = the length of the buffer
   * @return  the buffer contains the master_protocol and protocol name
   *
   */
NDPI_STATIC   char* ndpi_protocol2name(struct ndpi_detection_module_struct *ndpi_mod,
			   ndpi_protocol proto, char *buf, u_int buf_len);

  /**
   * Same as ndpi_protocol2name() with the difference that the numeric protocol
   * name is returned
   *
   * @par     ndpi_mod      = the detection module
   * @par     proto         = the struct ndpi_protocol contain the protocols name
   * @par     buf           = the buffer to write the name of the protocols
   * @par     buf_len       = the length of the buffer
   * @return  the buffer contains the master_protocol and protocol name
   *
   */
NDPI_STATIC   char* ndpi_protocol2id(struct ndpi_detection_module_struct *ndpi_mod,
			 ndpi_protocol proto, char *buf, u_int buf_len);

  /**
   * Find out if a given category is custom/user-defined
   *
   * @par     category      = the category associated to the protocol
   * @return  1 if this is a custom user category, 0 otherwise
   *
   */
NDPI_STATIC   int ndpi_is_custom_category(ndpi_protocol_category_t category);

  /**
   * Overwrite a protocol category defined by nDPI with the custom category
   *
   * @par     ndpi_mod      = the detection module
   * @par     protoId       = the protocol identifier to overwrite
   * @par     breed         = the breed to be associated to the protocol
   *
   */
NDPI_STATIC   void ndpi_set_proto_breed(struct ndpi_detection_module_struct *ndpi_mod,
			    u_int16_t protoId, ndpi_protocol_breed_t breed);

  /**
   * Overwrite a protocol category defined by nDPI with the custom category
   *
   * @par     ndpi_mod      = the detection module
   * @par     protoId       = the protocol identifier to overwrite
   * @par     category      = the category associated to the protocol
   *
   */
NDPI_STATIC   void ndpi_set_proto_category(struct ndpi_detection_module_struct *ndpi_mod,
			       u_int16_t protoId, ndpi_protocol_category_t protoCategory);

  /**
   * Check if subprotocols of the specified master protocol are just
   * informative (and not real)
   *
   * @par     mod           = the detection module
   * @par     protoId       = the (master) protocol identifier to query
   * @return  1 = the subprotocol is informative, 0 otherwise.
   *
   */
NDPI_STATIC   u_int8_t ndpi_is_subprotocol_informative(struct ndpi_detection_module_struct *ndpi_mod,
					   u_int16_t protoId);

  /**
   * Set hostname-based protocol
   *
   * @par ndpi_mod          = the detection module
   * @par flow              = the flow to which this communication belongs to
   * @par master_protocol   = the master protocol for this flow
   * @par name              = the host name
   * @par name_len          = length of the host name
   *
   */
NDPI_STATIC   int ndpi_match_hostname_protocol(struct ndpi_detection_module_struct *ndpi_mod,
				   struct ndpi_flow_struct *flow,
				   u_int16_t master_protocol,
				   char *name, u_int name_len);

NDPI_STATIC   u_int16_t ndpi_get_proto_by_name(struct ndpi_detection_module_struct *ndpi_mod,
		  		   const char *name);

  /**
   * Get classification confidence as string
   *
   * @par     confidence      = the confidence value
   * @return  the string name of the confidence result
   *
   */
NDPI_STATIC   const char* ndpi_confidence_get_name(ndpi_confidence_t confidence);

#ifndef __KERNEL__
  /**
   * Get protocol category as string
   *
   * @par     mod           = the detection module
   * @par     category      = the category associated to the protocol
   * @return  the string name of the category
   *
   */
NDPI_STATIC   const char* ndpi_category_get_name(struct ndpi_detection_module_struct *ndpi_mod,
				     ndpi_protocol_category_t category);

  /**
   * Set protocol category string
   *
   * @par     mod           = the detection module
   * @par     category      = the category associated to the protocol
   * @paw     name          = the string name of the category
   *
   */
NDPI_STATIC   void ndpi_category_set_name(struct ndpi_detection_module_struct *ndpi_mod,
			      ndpi_protocol_category_t category, char *name);

  /**
   * Get protocol category
   *
   * @par     ndpi_mod      = the detection module
   * @par     proto         = the struct ndpi_protocol contain the protocols name
   * @return  the protocol category
   */
NDPI_STATIC   ndpi_protocol_category_t ndpi_get_proto_category(struct ndpi_detection_module_struct *ndpi_mod,
						   ndpi_protocol proto);

#endif
  /**
   * Get the protocol name associated to the ID
   *
   * @par     mod           = the detection module
   * @par     proto_id      = the ID of the protocol
   * @return  the buffer contains the master_protocol and protocol name
   *
   */
NDPI_STATIC   char* ndpi_get_proto_name(struct ndpi_detection_module_struct *mod, u_int16_t proto_id);


  /**
   * Return the protocol breed ID associated to the protocol
   *
   * @par     ndpi_struct   = the detection module
   * @par     proto         = the ID of the protocol
   * @return  the breed ID associated to the protocol
   *
   */
NDPI_STATIC   ndpi_protocol_breed_t ndpi_get_proto_breed(struct ndpi_detection_module_struct *ndpi_struct,
					     u_int16_t proto);

  /**
   * Return the string name of the protocol breed
   *
   * @par     ndpi_struct   = the detection module
   * @par     breed_id      = the breed ID associated to the protocol
   * @return  the string name of the breed ID
   *
   */
NDPI_STATIC   char* ndpi_get_proto_breed_name(struct ndpi_detection_module_struct *ndpi_struct,
				  ndpi_protocol_breed_t breed_id);

  /**
   * Return the ID of the protocol
   *
   * @par     ndpi_mod   = the detection module
   * @par     proto      = the protocol name
   * @return  the ID of the protocol
   *
   */
NDPI_STATIC   int ndpi_get_protocol_id(struct ndpi_detection_module_struct *ndpi_mod, char *proto);

  /**
   * Return the ID of the category
   *
   * @par     ndpi_mod   = the detection module
   * @par     proto      = the category name
   * @return  the ID of the category
   *
   */
NDPI_STATIC   int ndpi_get_category_id(struct ndpi_detection_module_struct *ndpi_mod, char *cat);

#ifndef __KERNEL__
  /**
   * Write the list of the supported protocols
   *
   * @par  ndpi_mod = the detection module
   */
NDPI_STATIC   void ndpi_dump_protocols(struct ndpi_detection_module_struct *mod);

  /**
   * Generate Options list used in OPNsense firewall plugin
   *
   * @par  opt = The Option list to generate
   */
NDPI_STATIC   void ndpi_generate_options(u_int opt);

  /**
   * Write the list of the scores and their associated risks
   *
   * @par  ndpi_mod = the detection module
   */
NDPI_STATIC   void ndpi_dump_risks_score(void);

#endif
  /**
   * Read a file and load the protocols
   *
   * Format: <tcp|udp>:<port>,<tcp|udp>:<port>,.....@<proto>
   *
   * Example:
   * tcp:80,tcp:3128@HTTP
   * udp:139@NETBIOS
   *
   * @par     ndpi_mod = the detection module
   * @par     path     = the path of the file
   * @return  0 if the file is loaded correctly;
   *          -1 else
   *
   */
NDPI_STATIC   int ndpi_load_protocols_file(struct ndpi_detection_module_struct *ndpi_mod,
			       const char* path);

  /**
   * Add an IP-address based risk mask
   *
   * @par     ndpi_mod = the detection module
   * @par     ip       = the IP address for which you wanna set the mask
   * @par     mask     = the IP risk mask
   * @return  0 if the rule is loaded correctly;
   *          -1 else
   */
NDPI_STATIC   int ndpi_add_ip_risk_mask(struct ndpi_detection_module_struct *ndpi_mod, char *ip, ndpi_risk mask);

  /**
   * Add a host-address based risk mask
   *
   * @par     ndpi_mod = the detection module
   * @par     host     = the hostname/domain for which you wanna set the mask
   * @par     mask     = the host risk mask
   * @return  0 if the rule is loaded correctly;
   *          -1 else
   */
NDPI_STATIC   int ndpi_add_host_risk_mask(struct ndpi_detection_module_struct *ndpi_mod, char *host, ndpi_risk mask);

  /**
   * Add a trusted certificate issuer DN
   *
   * @par     ndpi_mod = the detection module
   * @par     dn       = the issuer DN as it appears in the certificate (example "CN=813845657003339838, O=Code42, OU=TEST, ST=MN, C=US")
   * @return  0 if the rule is loaded correctly; < 0 in case an error is detected
   */
NDPI_STATIC   int ndpi_add_trusted_issuer_dn(struct ndpi_detection_module_struct *ndpi_mod, char *dn);

  /**
   * Read a file and load the categories
   *
   * @par     ndpi_mod = the detection module
   * @par     path     = the path of the file
   * @par     user_data = pointer to some user data value
   * @return  0 if the file is loaded correctly;
   *          -1 else
   */
NDPI_STATIC   int ndpi_load_categories_file(struct ndpi_detection_module_struct *ndpi_str, const char* path, void *user_data);

  /**
   * Read a file and load the list of risky domains
   *
   * @par     ndpi_mod = the detection module
   * @par     path     = the path of the file
   * @return  0 if the file is loaded correctly;
   *          -1 else
   */
  NDPI_STATIC int ndpi_load_risk_domain_file(struct ndpi_detection_module_struct *ndpi_str, const char* path);

  /**
   * Read a file and load the list of malicious JA3 signatures
   *
   * @par     ndpi_mod = the detection module
   * @par     path     = the path of the file
   * @return  0 if the file is loaded correctly;
   *          -1 else
   */
  NDPI_STATIC int ndpi_load_malicious_ja3_file(struct ndpi_detection_module_struct *ndpi_str, const char *path);

  /**
   * Read a file and load the list of malicious SSL certificate SHA1 fingerprints.
   * @par     ndpi_mod = the detection module
   * @par     path     = the path of the file
   * @return  0 if the file is loaded correctly;
   *          -1 else
   */
  NDPI_STATIC int ndpi_load_malicious_sha1_file(struct ndpi_detection_module_struct *ndpi_str, const char *path);

  /**
   * Get the total number of the supported protocols
   *
   * @par     ndpi_mod = the detection module
   * @return  the number of protocols
   *
   */
  NDPI_STATIC u_int ndpi_get_num_supported_protocols(struct ndpi_detection_module_struct *ndpi_mod);

  /**
   * Get the nDPI version release
   *
   * @return the NDPI_GIT_RELEASE
   *
   */
  NDPI_STATIC char* ndpi_revision(void);

  /**
   * Set the automa for the protocol search
   *
   * @par ndpi_struct = the detection module
   * @par automa      = the automa to match
   *
   */
  NDPI_STATIC void ndpi_set_automa(struct ndpi_detection_module_struct *ndpi_struct,
		       void* automa);

  /* NDPI_PROTOCOL_HTTP */
  /**
   * Retrieve information for HTTP flows
   *
   * @par     ndpi_mod = the detection module
   * @par     flow     = the detected flow
   * @return  the HTTP method information about the flow
   *
   */
  NDPI_STATIC ndpi_http_method ndpi_get_http_method(struct ndpi_detection_module_struct *ndpi_mod,
					struct ndpi_flow_struct *flow);

  /**
   * Get the HTTP url
   *
   * @par     ndpi_mod = the detection module
   * @par     flow     = the detected flow
   * @return  the HTTP method information about the flow
   *
   */
  NDPI_STATIC char* ndpi_get_http_url(struct ndpi_detection_module_struct *ndpi_mod,
			  struct ndpi_flow_struct *flow);

  /**
   * Get the HTTP content-type
   *
   * @par     ndpi_mod = the detection module
   * @par     flow     = the detected flow
   * @return  the HTTP method information about the flow
   *
   */
  NDPI_STATIC char* ndpi_get_http_content_type(struct ndpi_detection_module_struct *ndpi_mod,
				   struct ndpi_flow_struct *flow);

  /* NDPI_PROTOCOL_TOR */
  /**
   * Check if the flow could be detected as TOR protocol
   *
   * @par     ndpi_struct = the detection module
   * @par     flow = the detected flow
   * @par     certificate = the SSL/TLS certificate
   * @return  1 if the flow is TOR;
   *          0 else
   *
   */
  NDPI_STATIC int ndpi_is_tls_tor(struct ndpi_detection_module_struct *ndpi_struct,
		      struct ndpi_flow_struct *flow, char *certificate);

  /* Wrappers functions */
  /**
   * Init Aho-Corasick automata
   *
   * @return  The requested automata, or NULL if an error occurred
   *
   */
  NDPI_STATIC void* ndpi_init_automa(void);

  /**
   * Free Aho-Corasick automata allocated with ndpi_init_automa();
   *
   * @par     The automata initialized with ndpi_init_automa();
   *
   */
  NDPI_STATIC void ndpi_free_automa(void *_automa);

  NDPI_STATIC int ndpi_string_to_automa(struct ndpi_detection_module_struct *ndpi_str,
                           void *ac_automa, const char *value,
                           u_int16_t protocol_id, ndpi_protocol_category_t category,
                           ndpi_protocol_breed_t breed, uint8_t level,
                           u_int8_t add_ends_with);
  /**
   * Get the automa statistics
   *
   * @par     The automata initialized with ndpi_init_automa();
   *
   */

  NDPI_STATIC void ndpi_automa_get_stats(void *_automa, struct ndpi_automa_stats *stats);

  /**
   * Get the statistics of one of the automas used internally by the library
   *
   * @par     ndpi_mod = the detection module
   * @par     automa_type = of which automa we want the stats
   * @par     stats = buffer where to save the stats
   * @return  0 in case of no error, or -1 if an error occurred.
   *
   */

  NDPI_STATIC int ndpi_get_automa_stats(struct ndpi_detection_module_struct *ndpi_struct,
			    automa_type automa_type,
			    struct ndpi_automa_stats *stats);
  /**
   * Add a string to match to an automata
   *
   * @par     The automata initialized with ndpi_init_automa();
   * @par     The (sub)string to search (malloc'ed memory)
   * @par     The number associated with this string
   * @return  0 in case of no error, or -2 if the string has been already addeed, or -1 if an error occurred.
   *
   */
  NDPI_STATIC int ndpi_add_string_value_to_automa(void *_automa, char *str, u_int32_t num);

  /**
   * Add a string to match to an automata. Same as ndpi_add_string_value_to_automa() with num set to 1
   *
   * @par     The automata initialized with ndpi_init_automa();
   * @par     The (sub)string to search (malloc'ed memory)
   * @return  0 in case of no error, or -1 if an error occurred.
   *
   */
  NDPI_STATIC int ndpi_add_string_to_automa(void *_automa, char *str);

  /**
   * Finalize the automa (necessary before start searching)
   *
   * @par     The automata initialized with ndpi_init_automa();
   *
   */
  NDPI_STATIC void ndpi_finalize_automa(void *_automa);

  NDPI_STATIC void *ndpi_automa_host(struct ndpi_detection_module_struct *ndpi_struct);
  NDPI_STATIC void **ndpi_get_automata(struct ndpi_detection_module_struct *ndpi_str);

  /**
   * Add a string to match to an automata
   *
   * @par     The automata initialized with ndpi_init_automa();
   * @par     The (sub)string to search
   * @return  0 in case of match, or -1 if no match, or -2 if an error occurred.
   *
   */
  NDPI_STATIC int ndpi_match_string(void *_automa, char *string_to_match);

  NDPI_STATIC int ndpi_load_ip_category(struct ndpi_detection_module_struct *ndpi_struct,
			    const char *ip_address_and_mask, ndpi_protocol_category_t category,
			    void *user_data);
  NDPI_STATIC int ndpi_load_hostname_category(struct ndpi_detection_module_struct *ndpi_struct,
				  const char *name_to_add, ndpi_protocol_category_t category);
#ifndef __KERNEL__
  NDPI_STATIC int ndpi_load_category(struct ndpi_detection_module_struct *ndpi_struct,
			 const char *ip_or_name, ndpi_protocol_category_t category,
			 void *user_data);
  NDPI_STATIC int ndpi_enable_loaded_categories(struct ndpi_detection_module_struct *ndpi_struct);
  NDPI_STATIC void* ndpi_find_ipv4_category_userdata(struct ndpi_detection_module_struct *ndpi_str,
					 u_int32_t saddr);
  NDPI_STATIC int ndpi_fill_ip_protocol_category(struct ndpi_detection_module_struct *ndpi_struct,
				     u_int32_t saddr,
				     u_int32_t daddr,
				     ndpi_protocol *ret);
  NDPI_STATIC int ndpi_match_custom_category(struct ndpi_detection_module_struct *ndpi_struct,
				 char *name, u_int name_len, ndpi_protocol_category_t *id);
  NDPI_STATIC int ndpi_get_custom_category_match(struct ndpi_detection_module_struct *ndpi_struct,
				     char *name_or_ip, u_int name_len,
				     ndpi_protocol_category_t *id);
  void ndpi_self_check_host_match(FILE *error_out);
#endif
  NDPI_STATIC void ndpi_fill_protocol_category(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *flow,
				   ndpi_protocol *ret);
  NDPI_STATIC int ndpi_set_detection_preferences(struct ndpi_detection_module_struct *ndpi_mod,
				     ndpi_detection_preference pref,
				     int value);

  NDPI_STATIC u_int16_t ndpi_map_user_proto_id_to_ndpi_id(struct ndpi_detection_module_struct *ndpi_str,
					      u_int16_t user_proto_id);
  NDPI_STATIC u_int16_t ndpi_map_ndpi_id_to_user_proto_id(struct ndpi_detection_module_struct *ndpi_str,
					      u_int16_t ndpi_proto_id);

  /* Tells to called on what l4 protocol given application protocol can be found */
  NDPI_STATIC ndpi_l4_proto_info ndpi_get_l4_proto_info(struct ndpi_detection_module_struct *ndpi_struct, u_int16_t ndpi_proto_id);
  NDPI_STATIC const char* ndpi_get_l4_proto_name(ndpi_l4_proto_info proto);

  NDPI_STATIC u_int16_t ndpi_get_lower_proto(ndpi_protocol proto);
  NDPI_STATIC u_int16_t ndpi_get_upper_proto(ndpi_protocol proto);

  NDPI_STATIC ndpi_proto_defaults_t* ndpi_get_proto_defaults(struct ndpi_detection_module_struct *ndpi_mod);
  NDPI_STATIC u_int ndpi_get_ndpi_num_supported_protocols(struct ndpi_detection_module_struct *ndpi_mod);
  NDPI_STATIC u_int ndpi_get_ndpi_num_custom_protocols(struct ndpi_detection_module_struct *ndpi_mod);
  NDPI_STATIC u_int ndpi_get_ndpi_detection_module_size(void);
  NDPI_STATIC void ndpi_set_log_level(struct ndpi_detection_module_struct *ndpi_mod, u_int l);
  NDPI_STATIC void ndpi_set_debug_bitmask(struct ndpi_detection_module_struct *ndpi_mod, NDPI_PROTOCOL_BITMASK debug_bitmask);

  /* Simple helper to get current time, in sec */
  NDPI_STATIC u_int32_t ndpi_get_current_time(struct ndpi_flow_struct *flow);

  /* LRU cache */
  NDPI_STATIC struct ndpi_lru_cache* ndpi_lru_cache_init(u_int32_t num_entries, u_int32_t ttl);
  NDPI_STATIC void ndpi_lru_free_cache(struct ndpi_lru_cache *c);
  NDPI_STATIC u_int8_t ndpi_lru_find_cache(struct ndpi_lru_cache *c, u_int32_t key,
			       u_int16_t *value, u_int8_t clean_key_when_found, u_int32_t now_sec);
  NDPI_STATIC void ndpi_lru_add_to_cache(struct ndpi_lru_cache *c, u_int32_t key, u_int16_t value, u_int32_t now_sec);
  NDPI_STATIC void ndpi_lru_get_stats(struct ndpi_lru_cache *c, struct ndpi_lru_cache_stats *stats);

  NDPI_STATIC int ndpi_get_lru_cache_stats(struct ndpi_detection_module_struct *ndpi_struct,
			       lru_cache_type cache_type,
			       struct ndpi_lru_cache_stats *stats);

  NDPI_STATIC int ndpi_get_lru_cache_size(struct ndpi_detection_module_struct *ndpi_struct,
			      lru_cache_type cache_type,
			      u_int32_t *num_entries);
  NDPI_STATIC int ndpi_set_lru_cache_size(struct ndpi_detection_module_struct *ndpi_struct,
			      lru_cache_type cache_type,
			      u_int32_t num_entries);

  NDPI_STATIC int ndpi_set_lru_cache_ttl(struct ndpi_detection_module_struct *ndpi_struct,
			     lru_cache_type cache_type,
			     u_int32_t ttl);
  NDPI_STATIC int ndpi_get_lru_cache_ttl(struct ndpi_detection_module_struct *ndpi_struct,
			     lru_cache_type cache_type,
			     u_int32_t *ttl);


  NDPI_STATIC int ndpi_set_opportunistic_tls(struct ndpi_detection_module_struct *ndpi_struct,
				 u_int16_t proto, int value);
  NDPI_STATIC int ndpi_get_opportunistic_tls(struct ndpi_detection_module_struct *ndpi_struct,
				 u_int16_t proto);

  NDPI_STATIC int ndpi_set_protocol_aggressiveness(struct ndpi_detection_module_struct *ndpi_struct,
                                       u_int16_t proto, u_int32_t value);
  NDPI_STATIC u_int32_t ndpi_get_protocol_aggressiveness(struct ndpi_detection_module_struct *ndpi_struct,
                                             u_int16_t proto);

  /**
   * Find a protocol id associated with a string automata
   *
   * @par     The automata initialized with ndpi_init_automa();
   * @par     The (sub)string to search
   * @par     The (sub)string length
   * @par     The protocol id associated with the matched string or 0 id not found.
   * @return  0 in case of match, or -1 if no match, or -2 if an error occurred.
   *
   */
  NDPI_STATIC int ndpi_match_string_protocol_id(void *_automa, char *string_to_match, u_int match_len,
				    u_int16_t *protocol_id,
				    ndpi_protocol_category_t *category,
				    ndpi_protocol_breed_t *breed);

  NDPI_STATIC int ndpi_handle_rule(struct ndpi_detection_module_struct *ndpi_str, char *rule, u_int8_t do_add);

  /**
   * Specifies the threshold used to trigger the NDPI_TLS_CERTIFICATE_ABOUT_TO_EXPIRE
   * flow risk that by default is set to 30 days
   *
   * @par    ndpi_struct  = the struct created for the protocol detection
   * @par    days         = the number of days threshold for emitting the alert
   *
   */
  NDPI_STATIC void ndpi_set_tls_cert_expire_days(struct ndpi_detection_module_struct *ndpi_str,
				     u_int8_t days);


  /* Utility functions to set ndpi malloc/free/print wrappers */
  NDPI_STATIC void set_ndpi_ticks_per_second(u_int32_t ticks_per_second);
  NDPI_STATIC void set_ndpi_malloc(void* (*__ndpi_malloc)(size_t size));
  NDPI_STATIC void set_ndpi_free(void  (*__ndpi_free)(void *ptr));
  NDPI_STATIC void set_ndpi_flow_malloc(void* (*__ndpi_flow_malloc)(size_t size));
  NDPI_STATIC void set_ndpi_flow_free(void  (*__ndpi_flow_free)(void *ptr));
  NDPI_STATIC void set_ndpi_debug_function(struct ndpi_detection_module_struct *ndpi_str,
			       ndpi_debug_function_ptr ndpi_debug_printf);
  NDPI_STATIC u_int16_t ndpi_get_api_version(void);
  NDPI_STATIC const char *ndpi_get_gcrypt_version(void);

  /* https://github.com/corelight/community-id-spec */
  NDPI_STATIC int ndpi_flowv4_flow_hash(u_int8_t l4_proto, u_int32_t src_ip, u_int32_t dst_ip, u_int16_t src_port, u_int16_t dst_port,
			    u_int8_t icmp_type, u_int8_t icmp_code, u_char *hash_buf, u_int8_t hash_buf_len);
  NDPI_STATIC int ndpi_flowv6_flow_hash(u_int8_t l4_proto, struct ndpi_in6_addr *src_ip, struct ndpi_in6_addr *dst_ip,
			    u_int16_t src_port, u_int16_t dst_port, u_int8_t icmp_type, u_int8_t icmp_code,
			    u_char *hash_buf, u_int8_t hash_buf_len);
  NDPI_STATIC u_int8_t ndpi_extra_dissection_possible(struct ndpi_detection_module_struct *ndpi_struct,
					  struct ndpi_flow_struct *flow);
  NDPI_STATIC u_int8_t ndpi_is_safe_ssl_cipher(u_int32_t cipher);
  NDPI_STATIC u_int16_t ndpi_guess_host_protocol_id(struct ndpi_detection_module_struct *ndpi_struct,
					struct ndpi_flow_struct *flow);
//  NDPI_STATIC int ndpi_has_human_readeable_string(struct ndpi_detection_module_struct *ndpi_struct,
//				      char *buffer, u_int buffer_size,
//				      u_int8_t min_string_match_len, /* Will return 0 if no string > min_string_match_len have been found */
//				      char *outbuf, u_int outbuf_len);
  /* Return a flow info string (summarized). Does only work for DNS/HTTP/TLS/QUIC. */
  NDPI_STATIC const char* ndpi_get_flow_info(struct ndpi_flow_struct const * const flow,
                                 ndpi_protocol const * const l7_protocol);
  NDPI_STATIC char* ndpi_ssl_version2str(char *buf, int buf_len,
                             u_int16_t version, u_int8_t *unknown_tls_version);
  NDPI_STATIC int ndpi_netbios_name_interpret(u_char *in, u_int in_len, u_char *out, u_int out_len);
  NDPI_STATIC void ndpi_patchIPv6Address(char *str);
  NDPI_STATIC void ndpi_user_pwd_payload_copy(u_int8_t *dest, u_int dest_len, u_int offset,
				  const u_int8_t *src, u_int src_len);
  NDPI_STATIC u_char* ndpi_base64_decode(const u_char *src, size_t len, size_t *out_len);
  NDPI_STATIC char* ndpi_base64_encode(unsigned char const* bytes_to_encode, size_t in_len); /* NOTE: caller MUST free the returned pointer */
  NDPI_STATIC void ndpi_string_sha1_hash(const u_int8_t *message, size_t len, u_char *hash /* 20-bytes */);

  NDPI_STATIC int ndpi_load_ipv4_ptree(struct ndpi_detection_module_struct *ndpi_str,
			   const char *path, u_int16_t protocol_id);
#ifndef __KERNEL__    
  NDPI_STATIC const char* ndpi_cipher2str(u_int32_t cipher, char unknown_cipher[8]);
  NDPI_STATIC const char* ndpi_tunnel2str(ndpi_packet_tunnel tt);
  NDPI_STATIC int ndpi_has_human_readeable_string(struct ndpi_detection_module_struct *ndpi_struct,
				      char *buffer, u_int buffer_size,
				      u_int8_t min_string_match_len, /* Will return 0 if no string > min_string_match_len have been found */
				      char *outbuf, u_int outbuf_len);
  NDPI_STATIC int ndpi_dpi2json(struct ndpi_detection_module_struct *ndpi_struct,
		    struct ndpi_flow_struct *flow,
		    ndpi_protocol l7_protocol,
		    ndpi_serializer *serializer);
  NDPI_STATIC int ndpi_flow2json(struct ndpi_detection_module_struct *ndpi_struct,
		     struct ndpi_flow_struct *flow,
		     u_int8_t ip_version,
		     u_int8_t l4_protocol,
		     u_int32_t src_v4, u_int32_t dst_v4,
		     struct ndpi_in6_addr *src_v6, struct ndpi_in6_addr *dst_v6,
		     u_int16_t src_port, u_int16_t dst_port,
		     ndpi_protocol l7_protocol,
		     ndpi_serializer *serializer);
#endif

  NDPI_STATIC char *ndpi_get_ip_proto_name(u_int16_t ip_proto, char *name, unsigned int name_len);
  NDPI_STATIC void ndpi_md5(const u_char *data, size_t data_len, u_char hash[16]);
  NDPI_STATIC u_int32_t ndpi_quick_hash(unsigned char *str, u_int str_len);

  NDPI_STATIC const char* ndpi_http_method2str(ndpi_http_method m);
  NDPI_STATIC ndpi_http_method ndpi_http_str2method(const char* method, u_int16_t method_len);

  /* Utility functions to fill prefix (used by the patricia tree) */
  NDPI_STATIC int ndpi_fill_prefix_v4(ndpi_prefix_t *p, const struct in_addr *a, int b, int mb);
  NDPI_STATIC int ndpi_fill_prefix_v6(ndpi_prefix_t *prefix, const struct in6_addr *addr, int bits, int maxbits);
  NDPI_STATIC int ndpi_fill_prefix_mac(ndpi_prefix_t *prefix, u_int8_t *mac, int bits, int maxbits);

  /* Patricia tree API (radix tree supporting IPv4/IPv6/MAC) */
  NDPI_STATIC ndpi_patricia_tree_t *ndpi_patricia_new(u_int16_t maxbits);
  NDPI_STATIC ndpi_patricia_tree_t *ndpi_patricia_clone (const ndpi_patricia_tree_t * const from);
  NDPI_STATIC void ndpi_patricia_destroy(ndpi_patricia_tree_t *patricia, ndpi_void_fn_t func);

  NDPI_STATIC ndpi_patricia_node_t *ndpi_patricia_search_exact(ndpi_patricia_tree_t *patricia, ndpi_prefix_t *prefix);
  NDPI_STATIC ndpi_patricia_node_t *ndpi_patricia_search_best(ndpi_patricia_tree_t *patricia, ndpi_prefix_t *prefix);
  NDPI_STATIC ndpi_patricia_node_t *ndpi_patricia_lookup(ndpi_patricia_tree_t *patricia, ndpi_prefix_t *prefix);
  NDPI_STATIC size_t ndpi_patricia_walk_tree_inorder(ndpi_patricia_tree_t *patricia, ndpi_void_fn3_t func, void *data);
  NDPI_STATIC size_t ndpi_patricia_walk_inorder(ndpi_patricia_node_t *node, ndpi_void_fn3_t func, void *data);
  NDPI_STATIC void ndpi_patricia_remove(ndpi_patricia_tree_t *patricia, ndpi_patricia_node_t *node);

  NDPI_STATIC void ndpi_patricia_set_node_u64(ndpi_patricia_node_t *node, u_int64_t value);
  NDPI_STATIC u_int64_t ndpi_patricia_get_node_u64(ndpi_patricia_node_t *node);
  NDPI_STATIC void ndpi_patricia_set_node_data(ndpi_patricia_node_t *node, void *data);
  NDPI_STATIC void *ndpi_patricia_get_node_data(ndpi_patricia_node_t *node);
  NDPI_STATIC ndpi_prefix_t *ndpi_patricia_get_node_prefix(ndpi_patricia_node_t *node);
  NDPI_STATIC u_int16_t ndpi_patricia_get_node_bits(ndpi_patricia_node_t *node);
  NDPI_STATIC u_int16_t ndpi_patricia_get_maxbits(ndpi_patricia_tree_t *tree);
  NDPI_STATIC void ndpi_patricia_get_stats(ndpi_patricia_tree_t *tree, struct ndpi_patricia_tree_stats *stats);

  NDPI_STATIC int ndpi_get_patricia_stats(struct ndpi_detection_module_struct *ndpi_struct,
                              ptree_type ptree_type,
                              struct ndpi_patricia_tree_stats *stats);

  /* ptree (trie) API - a wrapper on top of Patricia that seamlessly handle IPv4 and IPv6 */
  NDPI_STATIC ndpi_ptree_t* ndpi_ptree_create(void);
  NDPI_STATIC int ndpi_ptree_insert(ndpi_ptree_t *tree, const ndpi_ip_addr_t *addr, u_int8_t bits, u_int64_t user_data);
  NDPI_STATIC int ndpi_ptree_match_addr(ndpi_ptree_t *tree, const ndpi_ip_addr_t *addr, u_int64_t *user_data);
  NDPI_STATIC void ndpi_ptree_destroy(ndpi_ptree_t *tree);

  /* General purpose utilities */
  NDPI_STATIC u_int64_t ndpi_htonll(u_int64_t v);
  NDPI_STATIC u_int64_t ndpi_ntohll(u_int64_t v);
  NDPI_STATIC u_int8_t ndpi_is_valid_protoId(u_int16_t protoId);
  NDPI_STATIC u_int8_t ndpi_is_encrypted_proto(struct ndpi_detection_module_struct *ndpi_str, ndpi_protocol proto);

  /* DGA */
  NDPI_STATIC int ndpi_check_dga_name(struct ndpi_detection_module_struct *ndpi_str,
			  struct ndpi_flow_struct *flow,
			  char *name, u_int8_t is_hostname, u_int8_t check_subproto);
#ifndef __KERNEL__    

  /* Serializer (supports JSON, TLV, CSV) */

  /**
   * Initialize a serializer handle (allocated by the caller).
   * @param serializer The serializer handle
   * @param fmt The serialization format (ndpi_serialization_format_json, ndpi_serialization_format_tlv, ndpi_serialization_format_csv)
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_init_serializer(ndpi_serializer *serializer, ndpi_serialization_format fmt);

  /**
   * Initialize a serializer handle. Same as ndpi_init_serializer, but with some low-level settings.
   * @param serializer The serializer handle
   * @param fmt The serialization format (ndpi_serialization_format_json, ndpi_serialization_format_tlv, ndpi_serialization_format_csv)
   * @param buffer_size The initial internal buffer_size
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_init_serializer_ll(ndpi_serializer *serializer, ndpi_serialization_format fmt, u_int32_t buffer_size);

  /**
   * Release all allocated data structure.
   * @param serializer The serializer handle
   */
  NDPI_STATIC void ndpi_term_serializer(ndpi_serializer *serializer);

  /**
   * Reset the serializer (cleanup the internal buffer to start a new serialization)
   * @param serializer The serializer handle
   */
  NDPI_STATIC void ndpi_reset_serializer(ndpi_serializer *serializer);

  /**
   * Hint to not create the header (used to avoid creaign the header when not used)
   * @param serializer The serializer handle
   */
  NDPI_STATIC void ndpi_serializer_skip_header(ndpi_serializer *serializer);

  /**
   * Serialize a 32-bit unsigned int key and a 32-bit unsigned int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_uint32_uint32(ndpi_serializer *serializer, u_int32_t key, u_int32_t value);

  /**
   * Serialize a 32-bit unsigned int key and a 64-bit unsigned int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_uint32_uint64(ndpi_serializer *serializer, u_int32_t key, u_int64_t value);

  /**
   * Serialize a 32-bit unsigned int key and a 32-bit signed int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_uint32_int32(ndpi_serializer *serializer, u_int32_t key, int32_t value);

  /**
   * Serialize a 32-bit unsigned int key and a 64-bit signed int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_uint32_int64(ndpi_serializer *serializer, u_int32_t key, int64_t value);

  /**
   * Serialize a 32-bit unsigned int key and a float value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param format The float value format
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_uint32_float(ndpi_serializer *serializer, u_int32_t key, float value, const char *format /* e.f. "%.2f" */);

  /**
   * Serialize a 32-bit unsigned int key and a double value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param format The float value format
   * @return 0 on success, a negative number otherwise
   */
  int ndpi_serialize_uint32_double(ndpi_serializer *serializer, u_int32_t key, double value, const char *format /* e.f. "%.2f" */);

  /**
   * Serialize a 32-bit unsigned int key and a double value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param format The double value format
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_uint32_double(ndpi_serializer *serializer, u_int32_t key, double value, const char *format /* e.f. "%.2f" */);

  /**
   * Serialize a 32-bit unsigned int key and a string value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_uint32_string(ndpi_serializer *serializer, u_int32_t key, const char *value);

  /**
   * Serialize a 32-bit unsigned int key and a boolean value (JSON/CSV only, not supported by TLV)
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_uint32_boolean(ndpi_serializer *serializer, u_int32_t key, u_int8_t value);

  /**
   * Serialize a 32-bit unsigned int and an unterminated string value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param vlen The value length
   * @return 0 on success, a negative number otherwise
   */
  int ndpi_serialize_uint32_binary(ndpi_serializer *serializer, u_int32_t key, const char *_value, u_int16_t vlen);

  /**
   * Serialize an unterminated string key and a 32-bit signed int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_binary_int32(ndpi_serializer *_serializer, const char *key, u_int16_t klen, int32_t value);

  /**
   * Serialize a string key and a 32-bit signed int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_int32(ndpi_serializer *serializer, const char *key, int32_t value);

  /**
   * Serialize an unterminated string key and a 64-bit signed int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_binary_int64(ndpi_serializer *_serializer, const char *key, u_int16_t klen, int64_t value);

  /**
   * Serialize a string key and a 64-bit signed int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_int64(ndpi_serializer *serializer, const char *key, int64_t value);

  /**
   * Serialize an unterminated string key and a 32-bit unsigned int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_binary_uint32(ndpi_serializer *_serializer, const char *key, u_int16_t klen, u_int32_t value);

  /**
   * Serialize a string key and a 32-bit unsigned int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_uint32(ndpi_serializer *serializer, const char *key, u_int32_t value);

  /**
   * Serialize a string key and a float value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param format The float format
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_uint32_format(ndpi_serializer *serializer, const char *key, u_int32_t value, const char *format);

  /**
   * Serialize an unterminated string key and a 64-bit unsigned int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_binary_uint64(ndpi_serializer *_serializer, const char *key, u_int16_t klen, u_int64_t value);

  /**
   * Serialize a string key and a 64-bit unsigned int value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_uint64(ndpi_serializer *serializer, const char *key, u_int64_t value);

  /**
   * Serialize an unterminated string key and an unterminated string value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @param value The field value
   * @param vlen The value length
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_binary_binary(ndpi_serializer *_serializer, const char *key, u_int16_t klen, const char *_value, u_int16_t vlen);

  /**
   * Serialize a string key and a string value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_string(ndpi_serializer *serializer, const char *key, const char *value);

  /**
   * Serialize a string key and a string value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param value_len The field value length
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_string_len(ndpi_serializer *serializer, const char *key,
				       const char *value, u_int16_t value_len);

  /**
   * Serialize a string key and an unterminated string value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param vlen The value length
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_binary(ndpi_serializer *serializer, const char *key, const char *_value, u_int16_t vlen);

  /**
   * Serialize a string key and a raw value (this is a string which is added to the JSON without any quote or escaping)
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param vlen The value length
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_raw(ndpi_serializer *_serializer, const char *key, const char *_value, u_int16_t vlen);



  /**
   * Serialize an unterminated string key and a float value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @param value The field value
   * @param format The float format
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_binary_float(ndpi_serializer *_serializer, const char *key, u_int16_t klen, float value, const char *format /* e.f. "%.2f" */);

  /**
   * Serialize a string key and a a float value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @param format The float format
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_float(ndpi_serializer *serializer, const char *key, float value, const char *format /* e.f. "%.2f" */);

  /**
   * Serialize an unterminated string key and a double value
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @param value The field value
   * @param format The double format
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC ndpi_serialize_binary_double(ndpi_serializer *_serializer, const char *key, u_int16_t klen, double value, const char *format /* e.f. "%.2f" */);

  /**
   * Serialize an unterminated string key and a boolean value (JSON/CSV only, not supported by TLV)
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_binary_boolean(ndpi_serializer *_serializer, const char *key, u_int16_t klen, u_int8_t value);

  /**
   * Serialize a string key and a boolean value (JSON/CSV only, not supported by TLV)
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_string_boolean(ndpi_serializer *serializer, const char *key, u_int8_t value);

  /**
   * Serialize a raw record in an array (this is a low-level function and its use is not recommended)
   * @param serializer The serializer handle
   * @param record The record value
   * @param record_len The record length
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_raw_record(ndpi_serializer *_serializer, u_char *record, u_int32_t record_len);

  /**
   * Serialize an End-Of-Record (the current object becomes is terminated and added to an array,
   * and a new object is created where the next items will be added)
   * @param serializer The serializer handle
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_end_of_record(ndpi_serializer *serializer);

  /**
   * Serialize the start of a list with an unterminated string key, where the next serialized items
   * will be added (note: keys for the new items are ignored)
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_start_of_list_binary(ndpi_serializer *_serializer, const char *key, u_int16_t klen);

  /**
   * Serialize the start of a list, where the next serialized items will be added (note: keys for
   * the new items are ignored)
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_start_of_list(ndpi_serializer *serializer, const char *key);

  /**
   * Serialize the end of a list
   * @param serializer The serializer handle
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_end_of_list(ndpi_serializer *serializer);

  /**
   * Serialize the start of a block with an unterminated string key
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param klen The key length
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_start_of_block_binary(ndpi_serializer *_serializer, const char *key, u_int16_t klen);

  /**
   * Serialize the start of a block with a string key
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_start_of_block(ndpi_serializer *serializer, const char *key);

  /**
   * Serialize the start of a block with a numeric key
   * @param serializer The serializer handle
   * @param key The numeric key as 32-bit unsigned integer.
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_start_of_block_uint32(ndpi_serializer *serializer, u_int32_t key);

  /**
   * Serialize the end of a block
   * @param serializer The serializer handle
   * @param key The field name or ID
   * @param value The field value
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serialize_end_of_block(ndpi_serializer *serializer);

  /**
   * Return the serialized buffer
   * @param serializer The serializer handle
   * @param buffer_len The buffer length (out)
   * @return The buffer
   */
  NDPI_STATIC char* ndpi_serializer_get_buffer(ndpi_serializer *serializer, u_int32_t *buffer_len);

  /**
   * Return the current serialized buffer length
   * @param serializer The serializer handle
   * @return The buffer length
   */
  NDPI_STATIC u_int32_t ndpi_serializer_get_buffer_len(ndpi_serializer *serializer);

  /**
   * Return the real internal buffer size (containing the serialized buffer)
   * @param serializer The serializer handle
   * @return The internal buffer size
   */
  NDPI_STATIC u_int32_t ndpi_serializer_get_internal_buffer_size(ndpi_serializer *serializer);

  /**
   * Change the serializer buffer length
   * @param serializer The serializer handle
   * @param l The new buffer length
   * @return 0 on success, a negative number otherwise
   */
  NDPI_STATIC int ndpi_serializer_set_buffer_len(ndpi_serializer *serializer, u_int32_t l);

  /**
   * Return the configured serialization format
   * @param serializer The serializer handle
   * @return The serialization format
   */
  NDPI_STATIC ndpi_serialization_format ndpi_serializer_get_format(ndpi_serializer *serializer);

  /**
   * Set the CSV separator
   * @param serializer The serializer handle
   * @param separator The separator
   */
  NDPI_STATIC void ndpi_serializer_set_csv_separator(ndpi_serializer *serializer, char separator);

  /**
   * Return the header automatically built from keys (CSV only)
   * @param serializer The serializer handle
   * @param buffer_len The buffer length (out)
   * @return The header
   */
  NDPI_STATIC char* ndpi_serializer_get_header(ndpi_serializer *serializer, u_int32_t *buffer_len);

  /**
   * Create a snapshot of the internal buffer for later rollback (ndpi_serializer_rollback_snapshot)
   * @param serializer The serializer handle
   */
  NDPI_STATIC void ndpi_serializer_create_snapshot(ndpi_serializer *serializer);

  /**
   * Rollback to the latest snapshot
   * @param serializer The serializer handle
   */
  NDPI_STATIC void ndpi_serializer_rollback_snapshot(ndpi_serializer *serializer);

  /* Deserializer (supports TLV only) */

  NDPI_STATIC int ndpi_init_deserializer(ndpi_deserializer *deserializer,
			     ndpi_serializer *serializer);
  NDPI_STATIC int ndpi_init_deserializer_buf(ndpi_deserializer *deserializer,
				 u_int8_t *serialized_buffer,
				 u_int32_t serialized_buffer_len);

  ndpi_serialization_format ndpi_deserialize_get_format(ndpi_deserializer *_deserializer);
  ndpi_serialization_type ndpi_deserialize_get_item_type(ndpi_deserializer *deserializer, ndpi_serialization_type *key_type);
  NDPI_STATIC int ndpi_deserialize_next(ndpi_deserializer *deserializer);

  NDPI_STATIC int ndpi_deserialize_key_uint32(ndpi_deserializer *deserializer, u_int32_t *key);
  NDPI_STATIC int ndpi_deserialize_key_string(ndpi_deserializer *deserializer, ndpi_string *key);

  NDPI_STATIC int ndpi_deserialize_value_uint32(ndpi_deserializer *deserializer, u_int32_t *value);
  NDPI_STATIC int ndpi_deserialize_value_uint64(ndpi_deserializer *deserializer, u_int64_t *value);
  NDPI_STATIC int ndpi_deserialize_value_int32(ndpi_deserializer *deserializer, int32_t *value);
  NDPI_STATIC int ndpi_deserialize_value_int64(ndpi_deserializer *deserializer, int64_t *value);
  NDPI_STATIC int ndpi_deserialize_value_float(ndpi_deserializer *deserializer, float *value);
  NDPI_STATIC int ndpi_deserialize_value_double(ndpi_deserializer *deserializer, double *value);
  NDPI_STATIC int ndpi_deserialize_value_string(ndpi_deserializer *deserializer, ndpi_string *value);

  NDPI_STATIC int ndpi_deserialize_clone_item(ndpi_deserializer *deserializer, ndpi_serializer *serializer);
  NDPI_STATIC int ndpi_deserialize_clone_all(ndpi_deserializer *deserializer, ndpi_serializer *serializer);

  /*
   * Escape a string to be suitable for a JSON value, adding double quotes, and terminating the string with a null byte.
   * It is recommended to provide a destination buffer (dst) which is as large as double the source buffer (src) at least.
   * Upon successful return, these functions return the number of characters printed (excluding the null byte used to terminate the string).
   */
  NDPI_STATIC int ndpi_json_string_escape(const char *src, int src_len, char *dst, int dst_max_len);

  /* Data analysis */
  struct ndpi_analyze_struct* ndpi_alloc_data_analysis(u_int16_t _max_series_len);
  NDPI_STATIC void ndpi_init_data_analysis(struct ndpi_analyze_struct *s, u_int16_t _max_series_len);
  NDPI_STATIC void ndpi_free_data_analysis(struct ndpi_analyze_struct *d, u_int8_t free_pointer);
  NDPI_STATIC void ndpi_reset_data_analysis(struct ndpi_analyze_struct *d);
  NDPI_STATIC void ndpi_data_add_value(struct ndpi_analyze_struct *s, const u_int32_t value);

  /* Sliding-window only */
  NDPI_STATIC float ndpi_data_window_average(struct ndpi_analyze_struct *s);
  NDPI_STATIC float ndpi_data_window_variance(struct ndpi_analyze_struct *s);
  NDPI_STATIC float ndpi_data_window_stddev(struct ndpi_analyze_struct *s);

  /* All data */
  NDPI_STATIC float ndpi_data_average(struct ndpi_analyze_struct *s);
  NDPI_STATIC float ndpi_data_entropy(struct ndpi_analyze_struct *s);
  NDPI_STATIC float ndpi_data_variance(struct ndpi_analyze_struct *s);
  NDPI_STATIC float ndpi_data_stddev(struct ndpi_analyze_struct *s);
  NDPI_STATIC float ndpi_data_mean(struct ndpi_analyze_struct *s);
  NDPI_STATIC u_int32_t ndpi_data_last(struct ndpi_analyze_struct *s);
  NDPI_STATIC u_int32_t ndpi_data_min(struct ndpi_analyze_struct *s);
  NDPI_STATIC u_int32_t ndpi_data_max(struct ndpi_analyze_struct *s);
  NDPI_STATIC float ndpi_data_ratio(u_int32_t sent, u_int32_t rcvd);

  /* ******************************* */

  NDPI_STATIC int ndpi_alloc_rsi(struct ndpi_rsi_struct *s, u_int16_t num_learning_values);
  NDPI_STATIC void  ndpi_free_rsi(struct ndpi_rsi_struct *s);
  NDPI_STATIC float ndpi_rsi_add_value(struct ndpi_rsi_struct *s, const u_int32_t value);

  /* ******************************* */

  NDPI_STATIC int  ndpi_hw_init(struct ndpi_hw_struct *hw, u_int16_t num_periods, u_int8_t additive_seeasonal,
		    double alpha, double beta, double gamma, float significance);
  NDPI_STATIC void ndpi_hw_free(struct ndpi_hw_struct *hw);
  NDPI_STATIC int  ndpi_hw_add_value(struct ndpi_hw_struct *hw, const u_int64_t value, double *forecast,  double *confidence_band);
  NDPI_STATIC void ndpi_hw_reset(struct ndpi_hw_struct *hw);

  /* ******************************* */

  NDPI_STATIC int ndpi_ses_init(struct ndpi_ses_struct *ses, double alpha, float significance);
  NDPI_STATIC int ndpi_ses_add_value(struct ndpi_ses_struct *ses, const double _value, double *forecast, double *confidence_band);
  NDPI_STATIC void ndpi_ses_fitting(double *values, u_int32_t num_values, float *ret_alpha);
  NDPI_STATIC void ndpi_ses_reset(struct ndpi_ses_struct *ses);

  /* ******************************* */

  NDPI_STATIC u_int32_t ndpi_crc32(const void* data, size_t n_bytes);

  /* ******************************* */
  NDPI_STATIC int ndpi_des_init(struct ndpi_des_struct *des, double alpha, double beta, float significance);
  NDPI_STATIC int ndpi_des_add_value(struct ndpi_des_struct *des, const double _value, double *forecast, double *confidence_band);
  NDPI_STATIC void ndpi_des_fitting(double *values, u_int32_t num_values, float *ret_alpha, float *ret_beta);
  NDPI_STATIC void ndpi_des_reset(struct ndpi_des_struct *des);

  /* ******************************* */

  NDPI_STATIC int   ndpi_jitter_init(struct ndpi_jitter_struct *hw, u_int16_t num_periods);
  NDPI_STATIC void  ndpi_jitter_free(struct ndpi_jitter_struct *hw);
  NDPI_STATIC float ndpi_jitter_add_value(struct ndpi_jitter_struct *s, const float value);

  /* ******************************* */

  NDPI_STATIC const char* ndpi_data_ratio2str(float ratio);

  NDPI_STATIC void ndpi_data_print_window_values(struct ndpi_analyze_struct *s); /* debug */

  NDPI_STATIC ndpi_risk_enum ndpi_validate_url(char *url);

  NDPI_STATIC u_int8_t ndpi_is_protocol_detected(struct ndpi_detection_module_struct *ndpi_str,
				     ndpi_protocol proto);
  NDPI_STATIC void ndpi_serialize_risk(ndpi_serializer *serializer, ndpi_risk risk);
  NDPI_STATIC void ndpi_serialize_risk_score(ndpi_serializer *serializer, ndpi_risk_enum risk);
  NDPI_STATIC void ndpi_serialize_confidence(ndpi_serializer *serializer, ndpi_confidence_t confidence);
  NDPI_STATIC void ndpi_serialize_proto(struct ndpi_detection_module_struct *ndpi_struct,
                            ndpi_serializer *serializer,
                            ndpi_risk risk,
                            ndpi_confidence_t confidence,
                            ndpi_protocol l7_protocol);
#endif /* KERNEL */
  NDPI_STATIC const char* ndpi_risk2str(ndpi_risk_enum risk);
  NDPI_STATIC const char* ndpi_severity2str(ndpi_risk_severity s);
  NDPI_STATIC ndpi_risk_info* ndpi_risk2severity(ndpi_risk_enum risk);
  NDPI_STATIC u_int16_t ndpi_risk2score(ndpi_risk risk,
			    u_int16_t *client_score, u_int16_t *server_score);

  NDPI_STATIC u_int8_t ndpi_check_issuerdn_risk_exception(struct ndpi_detection_module_struct *ndpi_str,
					      char *issuerDN);    
  NDPI_STATIC u_int8_t ndpi_check_flow_risk_exceptions(struct ndpi_detection_module_struct *ndpi_str,
					   u_int num_params,
					   ndpi_risk_params params[]);
  
  /* ******************************* */

  /* HyperLogLog cardinality estimator */

  /* Memory lifecycle */
  NDPI_STATIC int ndpi_hll_init(struct ndpi_hll *hll, u_int8_t bits);
  NDPI_STATIC void ndpi_hll_destroy(struct ndpi_hll *hll);
  NDPI_STATIC void ndpi_hll_reset(struct ndpi_hll *hll);

  /* Add values */
  NDPI_STATIC int ndpi_hll_add(struct ndpi_hll *hll, const char *data, size_t data_len);
  NDPI_STATIC int ndpi_hll_add_number(struct ndpi_hll *hll, u_int32_t value) ;

  /* Get cardinality estimation */
  NDPI_STATIC double ndpi_hll_count(struct ndpi_hll *hll);

  /* ******************************* */

  NDPI_STATIC int  ndpi_init_bin(struct ndpi_bin *b, enum ndpi_bin_family f, u_int16_t num_bins);
  NDPI_STATIC void ndpi_free_bin(struct ndpi_bin *b);
  NDPI_STATIC struct ndpi_bin* ndpi_clone_bin(struct ndpi_bin *b);
  NDPI_STATIC void ndpi_inc_bin(struct ndpi_bin *b, u_int16_t slot_id, u_int64_t val);
  NDPI_STATIC void ndpi_set_bin(struct ndpi_bin *b, u_int16_t slot_id, u_int64_t value);
  NDPI_STATIC u_int64_t ndpi_get_bin_value(struct ndpi_bin *b, u_int16_t slot_id);
  NDPI_STATIC void ndpi_reset_bin(struct ndpi_bin *b);
  NDPI_STATIC void ndpi_normalize_bin(struct ndpi_bin *b);
  NDPI_STATIC char* ndpi_print_bin(struct ndpi_bin *b, u_int8_t normalize_first, char *out_buf, u_int out_buf_len);
  NDPI_STATIC float ndpi_bin_similarity(struct ndpi_bin *b1, struct ndpi_bin *b2,
			    u_int8_t normalize_first, float similarity_max_threshold);
  NDPI_STATIC int ndpi_cluster_bins(struct ndpi_bin *bins, u_int16_t num_bins,
			u_int8_t num_clusters, u_int16_t *cluster_ids,
			struct ndpi_bin *centroids);

  /* ******************************* */

  /*
   * Finds outliers using Z-score
   * Z-Score = (Value - Mean) / StdDev
   *
   * @par    values      = pointer to the individual values to be analyzed [in]
   * @par    outliers    = pointer to a list of outliers identified        [out]
   * @par    num_values  = lenght of values and outliers that MUST have the same lenght [in]
   *
   * @return The number of outliers found
  */
  NDPI_STATIC u_int ndpi_find_outliers(u_int32_t *values, bool *outliers, u_int32_t num_values);

  /* ******************************* */

  NDPI_STATIC u_int32_t ndpi_quick_16_byte_hash(u_int8_t *in_16_bytes_long);

//  extern int ndpi_stun_cache_enable;

  /**
   * Initialize the hashmap.
   *
   * @par    h            = pointer to the hash map [in, out]
   *
   * @return 0 on success, 1 otherwise
   *
   */
  NDPI_STATIC int ndpi_hash_init(ndpi_str_hash **h);

  /**
   * Free the hashmap.
   *
   * @par    h            = pointer to the hash map [in, out]
   * @par    cleanup_func = pointer to a optional callback function
   *                        called for each element in the hashmap [in]
   *
   */
  NDPI_STATIC void ndpi_hash_free(ndpi_str_hash **h, void (*cleanup_func)(ndpi_str_hash *h));

  /**
   * Search for an entry in the hashmap.
   *
   * @par    h            = pointer to the hash map [in]
   * @par    key          = character string (no '\0' required) [in]
   * @par    key_len      = length of the character string @key [in]
   * @par    value        = pointer to a pointer to the value, which contains a
   *                        previously added hash entry [in, out]
   *
   * @return 0 if an entry with that key was found, 1 otherwise
   *
   */
  NDPI_STATIC int ndpi_hash_find_entry(ndpi_str_hash *h, char *key, u_int key_len, void **value);

  /**
   * Add an entry to the hashmap.
   *
   * @par    h            = pointer to the hash map [in, out]
   * @par    key          = character string (no '\0' required) [in]
   * @par    key_len      = length of the character string @key [in]
   * @par    value        = pointer to the value to add [in]
   *
   * @return 0 if the entry was added, 1 otherwise
   *
   */
  NDPI_STATIC int ndpi_hash_add_entry(ndpi_str_hash **h, char *key, u_int8_t key_len, void *value);

  /* ******************************* */

#ifndef __KERNEL__
  NDPI_STATIC int ndpi_load_geoip(struct ndpi_detection_module_struct *ndpi_str,
		      const char *ip_city_data, const char *ip_as_data);
  NDPI_STATIC void ndpi_free_geoip(struct ndpi_detection_module_struct *ndpi_str);
  NDPI_STATIC int ndpi_get_geoip_asn(struct ndpi_detection_module_struct *ndpi_str,
			 char *ip, u_int32_t *asn);
  NDPI_STATIC int ndpi_get_geoip_country_continent(struct ndpi_detection_module_struct *ndpi_str, char *ip,
				       char *country_code, u_int8_t country_code_len,
				       char *continent, u_int8_t continent_len);
#endif

  /* ******************************* */

  NDPI_STATIC char* ndpi_get_flow_name(struct ndpi_flow_struct *flow);

#ifndef __KERNEL__
  NDPI_STATIC struct ndpi_packet_struct *
  ndpi_get_packet_struct(struct ndpi_detection_module_struct *ndpi_mod);
#else
  static inline struct ndpi_packet_struct *
  ndpi_get_packet_struct(struct ndpi_detection_module_struct *ndpi_mod) {
	return &ndpi_mod->packet_struct[smp_processor_id()];
  }
#endif

  /* ******************************* */

#ifndef __KERNEL__
  NDPI_STATIC ndpi_bitmap* ndpi_bitmap_alloc(void);
  NDPI_STATIC void ndpi_bitmap_free(ndpi_bitmap* b);
  NDPI_STATIC u_int64_t ndpi_bitmap_cardinality(ndpi_bitmap* b);
  NDPI_STATIC void ndpi_bitmap_set(ndpi_bitmap* b, u_int32_t value);
  NDPI_STATIC void ndpi_bitmap_unset(ndpi_bitmap* b, u_int32_t value);
  NDPI_STATIC bool ndpi_bitmap_isset(ndpi_bitmap* b, u_int32_t value);
  NDPI_STATIC void ndpi_bitmap_clear(ndpi_bitmap* b);

  NDPI_STATIC size_t ndpi_bitmap_serialize(ndpi_bitmap* b, char **buf);
  NDPI_STATIC ndpi_bitmap* ndpi_bitmap_deserialize(char *buf);

  NDPI_STATIC void ndpi_bitmap_and(ndpi_bitmap* a, ndpi_bitmap* b_and);
  NDPI_STATIC void ndpi_bitmap_or(ndpi_bitmap* a, ndpi_bitmap* b_or);

  NDPI_STATIC ndpi_bitmap_iterator* ndpi_bitmap_iterator_alloc(ndpi_bitmap* b);
  NDPI_STATIC void ndpi_bitmap_iterator_free(ndpi_bitmap* b);
  NDPI_STATIC bool ndpi_bitmap_iterator_next(ndpi_bitmap_iterator* i, u_int32_t *value);

  /* ******************************* */

  NDPI_STATIC char* ndpi_get_flow_risk_info(struct ndpi_flow_struct *flow,
				char *out, u_int out_len,
				u_int8_t use_json);
#endif

#ifdef __cplusplus
}
#endif

#endif	/* __NDPI_API_H__ */
