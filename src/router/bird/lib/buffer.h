/*
 *	BIRD Library -- Generic Buffer Structure
 *
 *	(c) 2013 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2013 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_BUFFER_H_
#define _BIRD_BUFFER_H_

#include "lib/resource.h"
#include "sysdep/config.h"

#define BUFFER(type)		struct { type *data; uint used, size; }

#define BUFFER_SIZE(v)		((v).size * sizeof(* (v).data))

#define BUFFER_INIT(v,pool,isize)					\
  ({									\
    (v).used = 0;							\
    (v).size = (isize);							\
    (v).data = mb_alloc(pool, BUFFER_SIZE(v));				\
  })

#define BUFFER_SET(v,nsize)						\
  ({									\
    (v).used = (nsize);							\
    if ((v).used > (v).size)						\
      buffer_realloc((void **) &((v).data), &((v).size), (v).used, sizeof(* (v).data)); \
  })

#define BUFFER_INC(v,step)						\
  ({									\
    uint _o = (v).used;							\
    BUFFER_SET(v, (v).used + (step));					\
    (v).data + _o;							\
  })

#define BUFFER_DEC(v,step)	({ (v).used -= (step); })

#define BUFFER_PUSH(v)		(*BUFFER_INC(v,1))

#define BUFFER_POP(v)		BUFFER_DEC(v,1)

#define BUFFER_FLUSH(v)		({ (v).used = 0; })

#endif /* _BIRD_BUFFER_H_ */
