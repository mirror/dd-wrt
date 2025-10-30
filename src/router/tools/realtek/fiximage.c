#include <stdio.h>
#include <stdlib.h>
#include "crc.c"

#define IH_NMLEN		32	/* Image Name Length		*/

/* Reused from common.h */
#define ROUND(a, b)		(((a) + (b) - 1) & ~((b) - 1))

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
struct legacy_img_hdr {
	unsigned int	ih_magic;	/* Image Header Magic Number	*/
	unsigned int	ih_hcrc;	/* Image Header CRC Checksum	*/
	unsigned int	ih_time;	/* Image Creation Timestamp	*/
	unsigned int	ih_size;	/* Image Data Size		*/
	unsigned int	ih_load;	/* Data	 Load  Address		*/
	unsigned int	ih_ep;		/* Entry Point Address		*/
	unsigned int	ih_dcrc;	/* Image Data CRC Checksum	*/
	unsigned char		ih_os;		/* Operating System		*/
	unsigned char		ih_arch;	/* CPU architecture		*/
	unsigned char		ih_type;	/* Image Type			*/
	unsigned char		ih_comp;	/* Compression Type		*/
	unsigned char		ih_name[IH_NMLEN];	/* Image Name		*/
};


int main(int argc, char *argv[]) {
FILE *in=fopen(argv[1],"r+b");
fseek(in,0,SEEK_END);
size_t len = ftell(in);
rewind(in);

struct legacy_img_hdr *hdr = malloc(len);
unsigned char *buf = (unsigned char*)hdr;
fread(hdr, len, 1, in);
fprintf(stderr, "old size %d\n", ntohl(hdr->ih_size));
hdr->ih_size = ntohl(len - sizeof(*hdr));
fprintf(stderr, "new size %d\n", ntohl(hdr->ih_size));
hdr->ih_dcrc = ntohl(crc32(buf + sizeof(*hdr), len - sizeof(*hdr), 0));
hdr->ih_hcrc = 0;
hdr->ih_hcrc = ntohl(crc32(hdr, sizeof(*hdr), 0));
rewind(in);
fwrite(hdr, len, 1, in);
fclose(in);
}