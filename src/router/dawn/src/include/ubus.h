#ifndef __DAWN_UBUS_H
#define __DAWN_UBUS_H

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>

#include "datastorage.h"

// 802.11 Status codes
#define WLAN_STATUS_SUCCESS 0
#define WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA 17
#define WLAN_STATUS_DENIED_NOT_HT_SUPPORT 27
#define WLAN_STATUS_DENIED_NOT_VHT_SUPPORT 104

// Disassociation Reason
#define UNSPECIFIED_REASON 0
#define NO_MORE_STAS 5

/**
 * Init ubus.
 * Setup tcp socket.
 * Start ubus timer.
 * @param ubus_socket
 * @param hostapd_dir
 * @return
 */
int dawn_init_ubus(const char *ubus_socket, const char *hostapd_dir);

/**
 * Start the umdns timer for updating the zeroconfiguration properties.
 */
void start_tcp_con_update();

/**
 * Call umdns update to update the TCP connections.
 * @return
 */
int ubus_call_umdns();

/**
 * Parse to client request.
 * @param msg
 * @param auth_req
 * @return
 */
int parse_to_client_req(struct blob_attr *msg, client_req_entry *client_req);

/**
 * Kick client from all hostapd interfaces.
 * @param client_addr - the client adress of the client to kick.
 * @param reason - the reason to kick the client.
 * @param deauth - if the client should be deauthenticated.
 * @param ban_time - the ban time the client is not allowed to connect again.
 */
void del_client_all_interfaces(const struct dawn_mac client_addr, uint32_t reason, uint8_t deauth, uint32_t ban_time);

/**
 * Update the hostapd sockets.
 * @param t
 */
void update_hostapd_sockets(struct uloop_timeout *t);

void ubus_set_nr_from_clients(struct kicking_nr* ap_list);

int ubus_send_beacon_request(client *c, ap *a, int d, int id);

void uloop_add_data_cbs();

int uci_send_via_network();

int build_network_overview(struct blob_buf* b);

int ap_get_nr(struct blob_buf* b, struct dawn_mac own_bssid_addr, const char *ssid);

int parse_add_mac_to_file(struct blob_attr* msg);

int handle_auth_req(struct blob_attr* msg);

/**
 * Send probe message via the network.
 * @param probe_entry
 * @return
 */
int ubus_send_probe_via_network(struct probe_entry_s *probe_entry, bool is_beacon);

/**
 * Add mac to a list that contains addresses of clients that can not be controlled.
 * @param buf
 * @param name
 * @param addr
 */
void blobmsg_add_macaddr(struct blob_buf *buf, const char *name, const struct dawn_mac addr);

/**
 * Send message via network.
 * @param msg
 * @param method
 * @return
 */
int send_blob_attr_via_network(struct blob_attr* msg, char* method);

/**
 * Set client timer for updating the clients.
 * @param time
 */
void add_client_update_timer(time_t time);

/**
 * Kick client from hostapd interface.
 * @param id - the ubus id.
 * @param client_addr - the client adress of the client to kick.
 * @param reason - the reason to kick the client.
 * @param deauth - if the client should be deauthenticated.
 * @param ban_time - the ban time the client is not allowed to connect again.
 */
void del_client_interface(uint32_t id, const struct dawn_mac client_addr, uint32_t reason, uint8_t deauth, uint32_t ban_time);

/**
 * Function to set the probe counter to the min probe request.
 * This allows that the client is able to connect directly without sending multiple probe requests to the Access Point.
 * @param client_addr
 * @return
 */
int send_set_probe(struct dawn_mac client_addr);

/**
 * Function to tell a client that it is about to be disconnected.
 * @param id
 * @param client_addr
 * @param dest_ap
 * @param duration
 * @return - 0 = asynchronous (client has been told to remove itself, and caller should manage arrays); 1 = synchronous (caller should assume arrays are updated)
 */
int wnm_disassoc_imminent(uint32_t id, const struct dawn_mac client_addr, struct kicking_nr* neighbor_list, int threshold, uint32_t duration);

/**
 * Function to ask a client to move to another AP, but not enforce it.
 * @param id
 * @param client_addr
 * @param dest_ap
 * @param duration
 * @return - 0 = asynchronous (client has been told to remove itself, and caller should manage arrays); 1 = synchronous (caller should assume arrays are updated)
 */
int bss_transition_request(uint32_t id, const struct dawn_mac client_addr, struct kicking_nr* neighbor_list, uint32_t duration);

/**
 * Send control message to all hosts to add the mac to a don't control list.
 * @param client_addr
 * @return
 */
int send_add_mac(struct dawn_mac client_addr);

#endif
