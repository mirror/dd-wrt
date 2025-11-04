/*
fixes length and checksum to fit to current image size. this is important since we sometimes need to align the image for a new size and 
factory firmware might seek for signatures after uimage defined size
*/
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "crc.c"

#define IH_NMLEN 32 /* Image Name Length		*/

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
struct legacy_img_hdr {
	unsigned int ih_magic; /* Image Header Magic Number	*/
	unsigned int ih_hcrc; /* Image Header CRC Checksum	*/
	unsigned int ih_time; /* Image Creation Timestamp	*/
	unsigned int ih_size; /* Image Data Size		*/
	unsigned int ih_load; /* Data	 Load  Address		*/
	unsigned int ih_ep; /* Entry Point Address		*/
	unsigned int ih_dcrc; /* Image Data CRC Checksum	*/
	unsigned char ih_os; /* Operating System		*/
	unsigned char ih_arch; /* CPU architecture		*/
	unsigned char ih_type; /* Image Type			*/
	unsigned char ih_comp; /* Compression Type		*/
	unsigned char ih_name[IH_NMLEN]; /* Image Name		*/
};

int main(int argc, char *argv[])
{
	FILE *in = fopen(argv[1], "r+b");
	fseek(in, 0, SEEK_END);
	size_t len = ftell(in);
	rewind(in);

	struct legacy_img_hdr *hdr = malloc(len);
	unsigned const char *buf = (unsigned char *)hdr;
	fread(hdr, 1, len, in);
	fprintf(stderr, "old size %d\n", ntohl(hdr->ih_size));
	hdr->ih_size = htonl(len - sizeof(*hdr));
	fprintf(stderr, "new size %d\n", ntohl(hdr->ih_size));
	fprintf(stderr, "old CRC %08X data\n", hdr->ih_dcrc);
	hdr->ih_dcrc = ntohl(crc32(0, buf + sizeof(*hdr), len - sizeof(*hdr)));
	fprintf(stderr, "new CRC %08X data\n", hdr->ih_dcrc);
	fprintf(stderr, "old CRC %08X header size %d\n", hdr->ih_hcrc, sizeof(struct legacy_img_hdr));
	//hdr->ih_magic = htonl(IH_MAGIC);
	hdr->ih_hcrc = htonl(0);
	unsigned int crc = crc32(0, buf, sizeof(struct legacy_img_hdr));
	hdr->ih_hcrc = htonl(crc);
	fprintf(stderr, "new CRC %08X\n", hdr->ih_hcrc);
	rewind(in);
	fwrite(hdr, 1, len, in);
	fclose(in);
}