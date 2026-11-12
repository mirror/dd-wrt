/* test harness: reads "privhex hashhex" lines, prints r||s (hex) from eccert's ec_sign() */
#define main eccert_main
#include "../eccert.c"
#undef main
static int hx(int c) { return c <= '9' ? c - '0' : (c | 32) - 'a' + 10; }
static void unhex(const char *s, u8 *o) { for (int i = 0; i < 32; i++) o[i] = hx(s[2*i]) << 4 | hx(s[2*i+1]); }
int main(void)
{
	static char line[256]; u8 priv[32], h[32], sig[64]; size_t n = 0; char c;
	ec_init();
	while (read(0, &c, 1) == 1) {
		if (c != '\n') { if (n < sizeof line - 1) line[n++] = c; continue; }
		line[n] = 0; n = 0;
		if (strlen(line) < 129) continue;
		unhex(line, priv); unhex(line + 65, h);
		if (!ec_sign(priv, h, sig)) { put(1, "FAIL\n"); continue; }
		char out[130]; for (int i = 0; i < 64; i++) { out[2*i] = "0123456789abcdef"[sig[i] >> 4]; out[2*i+1] = "0123456789abcdef"[sig[i] & 15]; }
		out[128] = '\n'; (void)!write(1, out, 129);
	}
	return 0;
}
