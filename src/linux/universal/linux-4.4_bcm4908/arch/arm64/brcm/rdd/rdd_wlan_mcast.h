/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

#ifndef _RDD_WLAN_MCAST_H_
#define _RDD_WLAN_MCAST_H_

#define RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_INDEX(_radio_index, _ssid) \
    ( (((_radio_index) & 0x3) << 4) | ((_ssid) & 0xF) )

#define RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_RADIO_INDEX(_index) \
    ( ((_index) >> 4) & 0x3 )

#define RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_SSID(_index) \
    ( (_index) & 0xF )

#define RDD_WLAN_MCAST_SSID_STATS_ENTRY_RADIO_INDEX(_index) \
    ( ((_index) >> 4) & 0x3 )

#define RDD_WLAN_MCAST_SSID_STATS_ENTRY_SSID(_index) \
    ( (_index) & 0xF )

typedef struct {
    void *virt_p;
    bdmf_phys_addr_t phys_addr;
} wlan_mcast_dhd_list_table_t;

int rdd_wlan_mcast_fwd_entry_add(RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry,
                                 RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS dhd_list_entry,
                                 uint32_t *fwd_entry_index);

int rdd_wlan_mcast_fwd_entry_delete(uint32_t xi_fwd_entry_index);

int rdd_wlan_mcast_fwd_entry_read(uint32_t fwd_entry_index,
                                  RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry,
                                  RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY_DTS *dhd_list_entry_array,
                                  uint8_t *dhd_station_count);

int rdd_wlan_mcast_fwd_entry_write(uint32_t fwd_entry_index,
                                   RDD_WLAN_MCAST_FWD_ENTRY_DTS *fwd_entry,
                                   RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS dhd_list_entry,
                                   uint32_t dhd_list_add);

int rdd_wlan_mcast_fwd_entry_dhd_station_count_get(uint32_t fwd_entry_index,
                                                   uint8_t *dhd_station_count);

int rdd_wlan_mcast_dhd_station_read(uint32_t dhd_station_index,
                                    RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry);

int rdd_wlan_mcast_dhd_station_find(RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry,
                                    uint32_t *dhd_station_index);

int rdd_wlan_mcast_dhd_station_add(RDD_WLAN_MCAST_DHD_STATION_ENTRY_DTS *dhd_station_entry,
                                   uint32_t *dhd_station_index);

int rdd_wlan_mcast_dhd_station_delete(uint32_t dhd_station_index);

int rdd_wlan_mcast_ssid_mac_address_add(uint32_t radio_index, uint32_t ssid,
                                        RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry,
                                        uint32_t *ssid_mac_address_index);

int rdd_wlan_mcast_ssid_mac_address_delete(uint32_t ssid_mac_address_index);

int rdd_wlan_mcast_ssid_mac_address_read(uint32_t ssid_mac_address_index,
                                         RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY_DTS *ssid_mac_address_entry);

int rdd_wlan_mcast_ssid_stats_read(uint32_t ssid_stats_index,
                                   RDD_WLAN_MCAST_SSID_STATS_ENTRY_DTS *ssid_stats_entry);
int rdd_wlan_mcast_init(void);

void rdd_wlan_mcast_exit(void);

#endif /* _RDD_WLAN_MCAST_H_ */
