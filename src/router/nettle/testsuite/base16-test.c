#include "testutils.h"
#include "base16.h"

static void
test_base16 (size_t data_length, const uint8_t *data,
	     const char *ascii)
{
  size_t ascii_length = strlen(ascii);
  char *buffer = xalloc(1 + ascii_length);
  uint8_t *check = xalloc(1 + BASE16_DECODE_LENGTH (ascii_length));
  struct base16_decode_ctx decode;
  size_t done;

  ASSERT(ascii_length <= BASE16_ENCODE_LENGTH (data_length));
  ASSERT(data_length <= BASE16_DECODE_LENGTH (ascii_length));

  memset(buffer, 0x33, 1 + ascii_length);
  memset(check, 0x55, 1 + data_length);

  base16_encode_update (buffer, data_length, data);

  ASSERT (MEMEQ(ascii_length, buffer, ascii));
  ASSERT (0x33 == buffer[ascii_length]);

  base16_decode_init (&decode);
  done = data_length;

  ASSERT (base16_decode_update(&decode, &done, check, ascii_length, buffer));
  ASSERT (done == data_length);
  ASSERT (base16_decode_final (&decode));

  ASSERT (MEMEQ(data_length, check, data));
  ASSERT (0x55 == check[data_length]);

  free(buffer);
  free(check);
}

void
test_main(void)
{
  ASSERT(BASE16_ENCODE_LENGTH(0) == 0);
  ASSERT(BASE16_ENCODE_LENGTH(1) == 2);
  ASSERT(BASE16_ENCODE_LENGTH(2) == 4);

  ASSERT(BASE16_DECODE_LENGTH(0) == 0);
  ASSERT(BASE16_DECODE_LENGTH(1) == 1);
  ASSERT(BASE16_DECODE_LENGTH(2) == 1);
  ASSERT(BASE16_DECODE_LENGTH(3) == 2);
  ASSERT(BASE16_DECODE_LENGTH(4) == 2);

  test_base16 (LDATA(""), "");
  test_base16 (LDATA("H"), "48");
  test_base16 (LDATA("He"), "4865");
  test_base16 (LDATA("Hel"), "48656c");
  test_base16 (LDATA("Hell"), "48656c6c");
  test_base16 (LDATA("Hello"), "48656c6c6f");
  test_base16 (LDATA("Hello\0"), "48656c6c6f00");
}
