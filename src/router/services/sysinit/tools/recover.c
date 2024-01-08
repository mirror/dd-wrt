void start_backup(void)
{
	char drive[64];
#ifdef HAVE_OPENRISC
	sprintf(drive, "/dev/sda");
#elif HAVE_RB600
	sprintf(drive, "/dev/sda");
#else
	char *d = getdisc();
	sprintf(drive, "/dev/%s", d);
	free(d);
#endif
	//backup nvram
	fprintf(stderr, "backup nvram\n");
	int size = nvram_size();
	FILE *in = fopen("/usr/local/nvram/nvram.bin", "rb");
	if (in) {
		char *mem = malloc(size);
		fread(mem, size, 1, in);
		fclose(in);
		in = fopen(drive, "r+b");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftello(in);
		fseeko(in, mtdlen - (size + 65536), SEEK_SET);
		fwrite(mem, size, 1, in);
		fflush(in);
		fsync(fileno(in));
		fclose(in);
		free(mem);
	}
}

void start_recover(void)
{
	FILE *in;
	char dev[64];
	fprintf(stderr, "recover broken nvram\n");
#ifdef HAVE_OPENRISC
	sprintf(dev, "/dev/sda");
#elif HAVE_RB600
	sprintf(dev, "/dev/sda");
#else
	sprintf(dev, "/dev/%s", getdisc());
#endif
	int size = nvram_size();
	in = fopen(dev, "rb");
	fseeko(in, 0, SEEK_END);
	off_t mtdlen = ftello64(in);
	fseeko(in, mtdlen - (size + 65536), SEEK_SET);

	unsigned char *mem = malloc(size);
	fread(mem, size, 1, in);
	if (mem[0] != 0x46 || mem[1] != 0x4c || mem[2] != 0x53 || mem[3] != 0x48) {
		fseeko(in, mtdlen - (65536 * 2), SEEK_SET);
		fread(mem, 65536, 1, in);
	}
	fclose(in);
	if (mem[0] == 0x46 && mem[1] == 0x4c && mem[2] == 0x53 && mem[3] == 0x48) {
		fprintf(stderr, "found recovery\n");
		in = fopen("/usr/local/nvram/nvram.bin", "wb");
		if (in != NULL) {
			fwrite(mem, size, 1, in);
			fclose(in);
		}
	}
	free(mem);
}
