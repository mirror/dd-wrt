#define ST5481_DEBUG 0x0

#if ST5481_DEBUG


/*
  DEBUG flags. Set compile option  ST5481_DEBUG with the following bits set to trace
  the given subsections:

  0x01:  USB
  0x02:  D
  0x04:  B
  0x08:  PH
  0x10:  PACKET_DUMP D out
  0x20:  ISO_DUMP D out
  0x40:  PACKET_DUMP D in
  0x80:  ISO_DUMP in
  0x100: PACKET_DUMP B out
  0x200: ISO_DUMP B out
  0x400: PACKET_DUMP B in
*/

#define DBG(level, format, arg...) \
if (level &  ST5481_DEBUG) \
printk(KERN_DEBUG __FUNCTION__ ": " format "\n" , ## arg) \

static inline void 
dump_packet(const char *name,const u_char *data,int pkt_len)
{
#define DUMP_HDR_SIZE 200
#define DUMP_TLR_SIZE 8
	if (pkt_len) {
		int i,len1,len2;

		printk(KERN_DEBUG "%s: length=%d,data=",name,pkt_len);

		if (pkt_len >  DUMP_HDR_SIZE+ DUMP_TLR_SIZE) {
			len1 = DUMP_HDR_SIZE;
			len2 = DUMP_TLR_SIZE;
		} else {
			len1 = pkt_len > DUMP_HDR_SIZE ? DUMP_HDR_SIZE : pkt_len;
			len2 = 0;			
		}
		for (i = 0; i < len1; ++i) {
		 	printk ("%.2x", data[i]);
		}
		if (len2) {
		 	printk ("..");
			for (i = pkt_len-DUMP_TLR_SIZE; i < pkt_len; ++i) {
				printk ("%.2x", data[i]);
			}
		}
		printk ("\n");
	}
#undef DUMP_HDR_SIZE
#undef DUMP_TLR_SIZE
}

static inline void 
dump_iso_packet(const char *name,urb_t *urb)
{
	int i,j;
	int len,ofs;
	u_char *data;

	printk(KERN_DEBUG "%s: packets=%d,errors=%d\n",
	       name,urb->number_of_packets,urb->error_count);
	for (i = 0; i  < urb->number_of_packets; ++i) {
		if (urb->pipe & USB_DIR_IN) {
			len = urb->iso_frame_desc[i].actual_length;
		} else {
			len = urb->iso_frame_desc[i].length;
		}
		ofs = urb->iso_frame_desc[i].offset;
		printk(KERN_DEBUG "len=%.2d,ofs=%.3d ",len,ofs);
		if (len) {
			data = urb->transfer_buffer+ofs;
			for (j=0; j < len; j++) {
				printk ("%.2x", data[j]);
			}
		}
		printk("\n");
	}
}

#define DUMP_PACKET(level,data,count) \
  if (level & ST5481_DEBUG) dump_packet(__FUNCTION__,data,count)
#define DUMP_SKB(level,skb) \
  if ((level & ST5481_DEBUG) && skb) dump_packet(__FUNCTION__,skb->data,skb->len)
#define DUMP_ISO_PACKET(level,urb) \
  if (level & ST5481_DEBUG) dump_iso_packet(__FUNCTION__,urb)

#else

#define DBG(level,format, arg...) do {} while (0)
#define DUMP_PACKET(level,data,count) do {} while (0)
#define DUMP_SKB(level,skb) do {} while (0)
#define DUMP_ISO_PACKET(level,urb) do {} while (0)

#endif



