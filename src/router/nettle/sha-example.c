#include <stdio.h>
#include <stdlib.h>

#include <nettle/sha2.h>

#define BUF_SIZE 1000

static void
display_hex(unsigned length, uint8_t *data)
{
  unsigned i;

  for (i = 0; i<length; i++)
    printf("%02x ", data[i]);

  printf("\n");
}

int
main(int argc, char **argv)
{
  struct sha256_ctx ctx;
  uint8_t buffer[BUF_SIZE];
  uint8_t digest[SHA256_DIGEST_SIZE];
  
  sha256_init(&ctx);
  for (;;)
  {
    int done = fread(buffer, 1, sizeof(buffer), stdin);
    sha256_update(&ctx, done, buffer);
    if (done < sizeof(buffer))
      break;
  }
  if (ferror(stdin))
    return EXIT_FAILURE;

  sha256_digest(&ctx, digest);

  display_hex(SHA256_DIGEST_SIZE, digest);
  return EXIT_SUCCESS;  
}
