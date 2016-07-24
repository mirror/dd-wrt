#ifdef ENABLE_G729
# ifdef ENABLE_BCG729
#  include <bcg729/encoder.h>
#  include <bcg729/decoder.h>
#  define G729_ECTX bcg729EncoderChannelContextStruct
#  define G729_DCTX bcg729DecoderChannelContextStruct
#  define G729_EINIT initBcg729EncoderChannel
#  define G729_ECLOSE closeBcg729EncoderChannel
#  define G729_DINIT initBcg729DecoderChannel
#  define G729_ENCODE(ctx, ibuf, obuf) bcg729Encoder((ctx), (ibuf), (obuf))
#  define G729_DECODE(ctx, ibuf, isize) g279_compat_decode((ctx), (ibuf), (isize))

   int16_t *g279_compat_decode(G729_DCTX *, uint8_t *, size_t);

# else
#  include <g729_encoder.h>
#  include <g729_decoder.h>
#  define G729_ECTX G729_CTX
#  define G729_DCTX G729_CTX
#  define G729_EINIT g729_encoder_new
#  define G729_ECLOSE g729_encoder_destroy
#  define G729_DINIT g729_decoder_new
#  define G729_ENCODE(ctx, ibuf, obuf) g729_encode_frame((ctx), (ibuf), (obuf))
#  define G729_DECODE(ctx, ibuf, isize) g729_decode_frame((ctx), (ibuf), (isize))
# endif

#endif
