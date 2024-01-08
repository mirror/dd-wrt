#include <stdio.h>

static void dumprom(int dev, char *target)
{
	char addr[64];
	char value[64];
	sprintf(addr, "/sys/kernel/debug/ieee80211/phy%d/ath10k/fwmem_addr",
		dev);
	sprintf(value, "/sys/kernel/debug/ieee80211/phy%d/ath10k/fwmem_value",
		dev);
	int i;
	FILE *out = fopen(target, "wb");
	for (i = 0; i < 0x40000; i += 4) {
		FILE *in = fopen(addr, "wb");
		if (!in)
			return;
		fprintf(in, "0x%x", i);
		fprintf(stdout, "read %X\n", i);
		fclose(in);
		in = fopen(value, "rb");
		unsigned int a, v;
		fscanf(in, "0x%08x:0x%08x", &a, &v);
		//              fwrite(
		//              putc(v >> 24,out);
		//              putc((v >> 16) & 0xff,out);
		//              putc((v >> 8) & 0xff,out);
		//              putc(v &0xff,out);
		fwrite(&v, 4, 1, out);
		fclose(in);
	}
	fclose(out);
}

void start_dumprom(void)
{
	dumprom(0, "/tmp/rom0.bin");
	dumprom(1, "/tmp/rom1.bin");
}
