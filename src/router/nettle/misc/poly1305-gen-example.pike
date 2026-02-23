#! /usr/bin/pike

void write_little_endian(int x) {
  for (int i = 0; i < 16; i++, x >>= 8) {
    write("%02x", x & 0xff);
  }
}

int main (int argc, array(string) argv) {
  int p = (1<<130) - 5;
  int mask = 0x0ffffffc0ffffffc0ffffffc0fffffff;

  int digest = (argc > 1) ? (int) argv[1] % p : 0;
  int r = random(1<<128) & mask;
  int r2 = r*r%p;

  for (;;) {
    int m0 = random(1<<128) | (1<<128);
    /* We want m0 r^2 + m1 r = digest (mod p), or

         m1 = (digest - m0 r^2 ) r^-1 (mod p)

       To be a valid message, this must also satisfy (m1 >> 128) == 1
    */
    int m1 = r->invert(p) * (digest - m0 * r2) % p;
    if ((m1 >> 128) == 1) {
      write ("r = 0x%x\n", r);
      write ("m0 = 0x%x\n", m0);
      write ("m1 = 0x%x\n", m1);
      write ("out = 0x%x\n\n", digest);

      write ("r = le "); write_little_endian(r);
      write ("\nm0 = le "); write_little_endian(m0);
      write ("\nm1 = le "); write_little_endian(m1);
      write ("\nout = le "); write_little_endian(digest);
      write("\n");
      if ((m0*r2 + m1*r) % p != digest)
        error("inconsistent result");
      return 0;
    }
  }
}
