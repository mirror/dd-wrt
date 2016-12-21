/*
 *      Tool to convert ELF image to be the AP downloadable binary
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: cvimg.c,v 1.1.1.1 2005/03/09 04:52:51 rex Exp $
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cvimg.h"

#define	S_PERM				(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)
#define DEFAULT_START_ADDR	0x80300000

static unsigned short calculateChecksum(char *buf, int len);

/////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	char inFile[80]={0}, outFile[80]={0}, imageType[80]={0};
	int fh, size;
	struct stat status;
	char *buf;
	IMG_HEADER_Tp pHeader;
	unsigned int startAddr;
   unsigned int burnAddr;
	unsigned short checksum;
	int i, is_rootfs = 0;

	// parse input arguments
#if 1 //EDX wade 
	if ( argc != 5 && argc != 6 && argc != 7) {
#else
	if ( argc != 5 && argc != 6) {
#endif
		printf("Usage: cvimg linux input-filename output-filename start-addr burn-addr\n");
		printf("        \"start-addr\" is optional. If not given, it will use 0x%x as default.\n",
				DEFAULT_START_ADDR);
		exit(1);
	}
	
	sscanf(argv[1], "%s", imageType);
	sscanf(argv[2], "%s", inFile);
	sscanf(argv[3], "%s", outFile);
	if ( argc == 5 ) {
		sscanf(argv[4], "%x", &startAddr);
      }

	else
		startAddr = DEFAULT_START_ADDR;
#if 1
	if ( (argc == 6) || (argc == 7) ) {
#else
	if ( argc == 6 ) {
#endif
		sscanf(argv[5], "%x", &burnAddr);
		sscanf(argv[4], "%x", &startAddr);
        }
	else
		burnAddr = DEFAULT_START_ADDR;

	// check input file and allocate buffer
	if ( stat(inFile, &status) < 0 ) {
		printf("Can't stat file! [%s]\n", inFile );
		exit(1);
	}
	size = status.st_size + sizeof(IMG_HEADER_T) + sizeof(checksum);
	
	
	if (size%2)
		size++;	// padding	

	buf = malloc(size);
	if (buf == NULL) {
		printf("Malloc buffer failed!\n");
		exit(1);
	}
	memset(buf, '\0', size);
	pHeader = (IMG_HEADER_Tp)buf;
	buf += sizeof(IMG_HEADER_T);

	// Read data and generate header
	fh = open(inFile, O_RDONLY);
	if ( fh == -1 ) {
		printf("Open input file error!\n");
		free( pHeader );
		exit(1);
	}
	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, status.st_size) != status.st_size) {
		printf("Read file error!\n");
		close(fh);
		free(pHeader);
		exit(1);
	}
	close(fh);

	if(!strcmp( imageType, "boot"))
		memcpy(pHeader->signature, "BOOT", SIGNATURE_LEN);
//EDX shakim start
	else if(!strcmp( imageType, "anfg"))
		memcpy(pHeader->signature, "ANFG", SIGNATURE_LEN);
//EDX shakim end
	else if(!strcmp( imageType, "adsl"))
		memcpy(pHeader->signature, "ADSL", SIGNATURE_LEN);
	else if(!strcmp( imageType, "rsdk"))
		memcpy(pHeader->signature, "RSDK", SIGNATURE_LEN);
	else if (!strcmp (imageType, "root"))	//special case for 2.6 kernel
		is_rootfs = 1;
	else
		memcpy(pHeader->signature, FW_HEADER, SIGNATURE_LEN);
#if 1//EDX wade
        if ( argc == 7 )
        {
		memcpy(pHeader->modTag, argv[6], SIGNATURE_LEN);
        }
	else
		memcpy(pHeader->modTag, _WEB_HEADER_, SIGNATURE_LEN);
#else	
	memcpy(pHeader->modTag, _WEB_HEADER_, SIGNATURE_LEN);	// Erwin
#endif
	
	pHeader->len = DWORD_SWAP(size-sizeof(IMG_HEADER_T));
	//printf("%x\n",pHeader->len);
//	pHeader->len = DWORD_SWAP(0xc317);
	pHeader->startAddr = DWORD_SWAP(startAddr);
    pHeader->burnAddr = DWORD_SWAP(burnAddr);
	if (is_rootfs){
		#define SIZE_OF_SQFS_SUPER_BLOCK 640
		unsigned int fs_len;

		fs_len = DWORD_SWAP((size - sizeof(IMG_HEADER_T) - sizeof(checksum) - SIZE_OF_SQFS_SUPER_BLOCK));
		memcpy(buf + 8, &fs_len, 4);

		fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC, S_PERM);
		if (fh == -1) {
			printf("Create output file error! [%s]:%s\n", outFile, strerror(errno));
			free(pHeader);
			exit(1);
		}
		write(fh, buf, size - sizeof(IMG_HEADER_T) - sizeof(checksum));
	}else{
//EDX Rex		checksum = WORD_SWAP(calculateChecksum(buf, status.st_size));
		checksum = WORD_SWAP(calculateChecksum(buf, size-sizeof(IMG_HEADER_T)-sizeof(checksum))); //EDX Rex
		*((unsigned short *)&buf[size-sizeof(IMG_HEADER_T)-sizeof(checksum)]) = checksum;

		// Write image to output file
		fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC, S_PERM);
		if ( fh == -1 ) {
			printf("Create output file error! [%s]:%s\n", outFile, strerror(errno));
			free(pHeader);
			exit(1);
		}
		write(fh, pHeader, size);
	}
	close(fh);
	
	printf("Generate kernel image successfully, length=%d (0x%08x), checksum=0x%x\n", DWORD_SWAP(pHeader->len),DWORD_SWAP(pHeader->len),checksum);


	free(pHeader);
}

static unsigned short calculateChecksum(char *buf, int len)
{
	int i, j;
	unsigned short sum=0, tmp;

	j = (len/2)*2;

	for (i=0; i<j; i+=2) {
		tmp = *((unsigned short *)(buf + i));
		sum += WORD_SWAP(tmp);
	}

	if ( len % 2 ) {
		tmp = buf[len-1];
		sum += WORD_SWAP(tmp);
	}
	return ~sum+1;
}
