void start_backup(void)
{
	char drive[64];
#ifdef HAVE_RB600
	sprintf(drive, "/dev/sda");
#else
	sprintf(drive, "/dev/discs/disc%d/disc", getdiscindex());
#endif
	//backup nvram
	fprintf(stderr, "backup nvram\n");
	FILE *in = fopen64("/usr/local/nvram/nvram.bin", "rb");
	if (in) {
		char *mem = malloc(65536);
		fread(mem, 65536, 1, in);
		fclose(in);
		in = fopen(drive, "r+b");
		fseeko64(in, 0, SEEK_END);
		__off64_t mtdlen = ftell(in);
		fseeko64(in, mtdlen-(65536*2), SEEK_SET);
		fwrite(mem, 65536, 1, in);
		fclose(in);
		eval("sync");
		fprintf(stderr, "reread for sync disc\n");
		in = fopen64(drive, "rb");
		fseeko64(in, mtdlen-(65536*2), SEEK_SET);
		fread(mem, 65536, 1, in);
		fprintf(stderr, "%X%X%X%X\n", mem[0] & 0xff, mem[1] & 0xff,
			mem[2] & 0xff, mem[3] & 0xff);
		fclose(in);
		free(mem);
	}

}

void start_recover(void)
{
	FILE *in;
	char dev[64];
	fprintf(stderr, "recover broken nvram\n");
#ifdef HAVE_RB600
	sprintf(dev, "/dev/sda");
#else
	sprintf(dev, "/dev/discs/disc%d/disc", getdiscindex());
#endif
	in = fopen(dev, "rb");
	fseeko64(in, 0, SEEK_END);
	__off64_t mtdlen = ftello64(in);
	fseeko64(in, mtdlen-(65536*2), SEEK_SET);

	unsigned char *mem = malloc(65536);
	fread(mem, 65536, 1, in);
	fclose(in);
	if (mem[0] == 0x46 && mem[1] == 0x4c && mem[2] == 0x53
	    && mem[3] == 0x48) {
		fprintf(stderr, "found recovery\n");
		in = fopen64("/usr/local/nvram/nvram.bin", "wb");
		if (in != NULL) {
			fwrite(mem, 65536, 1, in);
			fclose(in);
		}
	}
	free(mem);

}

