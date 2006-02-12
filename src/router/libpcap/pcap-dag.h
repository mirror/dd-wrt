/*
 * pcap-dag.c: Packet capture interface for Endace DAG card.
 *
 * The functionality of this code attempts to mimic that of pcap-linux as much
 * as possible.  This code is only needed when compiling in the DAG card code
 * at the same time as another type of device.
 *
 * Author: Richard Littin, Sean Irvine ({richard,sean}@reeltwo.com)
 *
 * @(#) $Header: /repository/WRT54G/src/router/libpcap/Attic/pcap-dag.h,v 1.1.2.1 2004/06/11 11:06:32 nikki Exp $ (LBL)
 */

pcap_t *dag_open_live(const char *device, int snaplen, int promisc, int to_ms, char *ebuf);

