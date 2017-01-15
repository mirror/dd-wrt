/*
 *	BIRD -- Bidirectional Forwarding Detection (BFD)
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "bfd.h"
#include "lib/mac.h"


struct bfd_ctl_packet
{
  u8 vdiag;				/* Version and diagnostic */
  u8 flags;				/* State and flags */
  u8 detect_mult;
  u8 length;				/* Whole packet length */
  u32 snd_id;				/* Sender ID, aka 'my discriminator' */
  u32 rcv_id;				/* Receiver ID, aka 'your discriminator' */
  u32 des_min_tx_int;
  u32 req_min_rx_int;
  u32 req_min_echo_rx_int;
};

struct bfd_auth
{
  u8 type;				/* Authentication type (BFD_AUTH_*) */
  u8 length;				/* Authentication section length */
};

struct bfd_simple_auth
{
  u8 type;				/* BFD_AUTH_SIMPLE */
  u8 length;				/* Length of bfd_simple_auth + pasword length */
  u8 key_id;				/* Key ID */
  byte password[0];			/* Password itself, variable length */
};

#define BFD_MAX_PASSWORD_LENGTH 16

struct bfd_crypto_auth
{
  u8 type;				/* BFD_AUTH_*_MD5 or BFD_AUTH_*_SHA1 */
  u8 length;				/* Length of bfd_crypto_auth + hash length */
  u8 key_id;				/* Key ID */
  u8 zero;				/* Reserved, zero on transmit */
  u32 csn;				/* Cryptographic sequence number */
  byte data[0];				/* Authentication key/hash, length 16 or 20 */
};

#define BFD_BASE_LEN	sizeof(struct bfd_ctl_packet)
#define BFD_MAX_LEN	64

#define DROP(DSC,VAL) do { err_dsc = DSC; err_val = VAL; goto drop; } while(0)

#define LOG_PKT(msg, args...) \
  log(L_REMOTE "%s: " msg, p->p.name, args)

#define LOG_PKT_AUTH(msg, args...) \
  log(L_AUTH "%s: " msg, p->p.name, args)


static inline u8 bfd_pack_vdiag(u8 version, u8 diag)
{ return (version << 5) | diag; }

static inline u8 bfd_pack_flags(u8 state, u8 flags)
{ return (state << 6) | flags; }

static inline u8 bfd_pkt_get_version(struct bfd_ctl_packet *pkt)
{ return pkt->vdiag >> 5; }

static inline u8 bfd_pkt_get_diag(struct bfd_ctl_packet *pkt)
{ return pkt->vdiag & 0x1f; }


static inline u8 bfd_pkt_get_state(struct bfd_ctl_packet *pkt)
{ return pkt->flags >> 6; }

static inline void UNUSED bfd_pkt_set_state(struct bfd_ctl_packet *pkt, u8 val)
{ pkt->flags = val << 6; }


char *
bfd_format_flags(u8 flags, char *buf)
{
  char *bp = buf;
  if (flags & BFD_FLAGS)	*bp++ = ' ';
  if (flags & BFD_FLAG_POLL)	*bp++ = 'P';
  if (flags & BFD_FLAG_FINAL)	*bp++ = 'F';
  if (flags & BFD_FLAG_CPI)	*bp++ = 'C';
  if (flags & BFD_FLAG_AP)	*bp++ = 'A';
  if (flags & BFD_FLAG_DEMAND)	*bp++ = 'D';
  if (flags & BFD_FLAG_MULTIPOINT) *bp++ = 'M';
  *bp = 0;

  return buf;
}

const u8 bfd_auth_type_to_hash_alg[] = {
    [BFD_AUTH_NONE] = 			ALG_UNDEFINED,
    [BFD_AUTH_SIMPLE] = 		ALG_UNDEFINED,
    [BFD_AUTH_KEYED_MD5] = 		ALG_MD5,
    [BFD_AUTH_METICULOUS_KEYED_MD5] = 	ALG_MD5,
    [BFD_AUTH_KEYED_SHA1] = 		ALG_SHA1,
    [BFD_AUTH_METICULOUS_KEYED_SHA1] = 	ALG_SHA1,
};


/* Fill authentication section and modifies final length in control section packet */
static void
bfd_fill_authentication(struct bfd_proto *p, struct bfd_session *s, struct bfd_ctl_packet *pkt)
{
  struct bfd_iface_config *cf = s->ifa->cf;
  struct password_item *pass = password_find(cf->passwords, 0);
  uint meticulous = 0;

  if (!pass)
  {
    /* FIXME: This should not happen */
    log(L_ERR "%s: No suitable password found for authentication", p->p.name);
    return;
  }

  switch (cf->auth_type)
  {
  case BFD_AUTH_SIMPLE:
  {
    struct bfd_simple_auth *auth = (void *) (pkt + 1);
    uint pass_len = MIN(pass->length, BFD_MAX_PASSWORD_LENGTH);

    auth->type = BFD_AUTH_SIMPLE;
    auth->length = sizeof(struct bfd_simple_auth) + pass_len;
    auth->key_id = pass->id;

    pkt->flags |= BFD_FLAG_AP;
    pkt->length += auth->length;

    memcpy(auth->password, pass->password, pass_len);
    return;
  }

  case BFD_AUTH_METICULOUS_KEYED_MD5:
  case BFD_AUTH_METICULOUS_KEYED_SHA1:
    meticulous = 1;

  case BFD_AUTH_KEYED_MD5:
  case BFD_AUTH_KEYED_SHA1:
  {
    struct bfd_crypto_auth *auth = (void *) (pkt + 1);
    uint hash_alg = bfd_auth_type_to_hash_alg[cf->auth_type];
    uint hash_len = mac_type_length(pass->alg);

    /* Increase CSN about one time per second */
    u32  new_time = (u64) current_time() >> 20;
    if ((new_time != s->tx_csn_time) || meticulous)
    {
      s->tx_csn++;
      s->tx_csn_time = new_time;
    }

    DBG("[%I] CSN: %u\n", s->addr, s->last_tx_csn);

    auth->type = cf->auth_type;
    auth->length = sizeof(struct bfd_crypto_auth) + hash_len;
    auth->key_id = pass->id;
    auth->zero = 0;
    auth->csn = htonl(s->tx_csn);

    pkt->flags |= BFD_FLAG_AP;
    pkt->length += auth->length;

    strncpy(auth->data, pass->password, hash_len);
    mac_fill(hash_alg, NULL, 0, (byte *) pkt, pkt->length, auth->data);
    return;
  }
  }
}

static int
bfd_check_authentication(struct bfd_proto *p, struct bfd_session *s, struct bfd_ctl_packet *pkt)
{
  struct bfd_iface_config *cf = s->ifa->cf;
  const char *err_dsc = NULL;
  uint err_val = 0;
  uint auth_type = 0;
  uint meticulous = 0;

  if (pkt->flags & BFD_FLAG_AP)
  {
    struct bfd_auth *auth = (void *) (pkt + 1);

    if ((pkt->length < (BFD_BASE_LEN + sizeof(struct bfd_auth))) ||
	(pkt->length < (BFD_BASE_LEN + auth->length)))
      DROP("packet length mismatch", pkt->length);

    /* Zero is reserved, we use it as BFD_AUTH_NONE internally */
    if (auth->type == 0)
      DROP("reserved authentication type", 0);

    auth_type = auth->type;
  }

  if (auth_type != cf->auth_type)
    DROP("authentication method mismatch", auth_type);

  switch (auth_type)
  {
  case BFD_AUTH_NONE:
    return 1;

  case BFD_AUTH_SIMPLE:
  {
    struct bfd_simple_auth *auth = (void *) (pkt + 1);

    if (auth->length < sizeof(struct bfd_simple_auth))
      DROP("wrong authentication length", auth->length);

    struct password_item *pass = password_find_by_id(cf->passwords, auth->key_id);
    if (!pass)
      DROP("no suitable password found", auth->key_id);

    uint pass_len = MIN(pass->length, BFD_MAX_PASSWORD_LENGTH);
    uint auth_len = sizeof(struct bfd_simple_auth) + pass_len;

    if ((auth->length != auth_len) || memcmp(auth->password, pass->password, pass_len))
      DROP("wrong password", pass->id);

    return 1;
  }

  case BFD_AUTH_METICULOUS_KEYED_MD5:
  case BFD_AUTH_METICULOUS_KEYED_SHA1:
    meticulous = 1;

  case BFD_AUTH_KEYED_MD5:
  case BFD_AUTH_KEYED_SHA1:
  {
    struct bfd_crypto_auth *auth = (void *) (pkt + 1);
    uint hash_alg = bfd_auth_type_to_hash_alg[cf->auth_type];
    uint hash_len = mac_type_length(hash_alg);

    if (auth->length != (sizeof(struct bfd_crypto_auth) + hash_len))
      DROP("wrong authentication length", auth->length);

    struct password_item *pass = password_find_by_id(cf->passwords, auth->key_id);
    if (!pass)
      DROP("no suitable password found", auth->key_id);

    /* BFD CSNs are in 32-bit circular number space */
    u32 csn = ntohl(auth->csn);
    if (s->rx_csn_known &&
	(((csn - s->rx_csn) > (3 * s->detect_mult)) ||
	 (meticulous && (csn == s->rx_csn))))
    {
      /* We want to report both new and old CSN */
      LOG_PKT_AUTH("Authentication failed for %I - "
		   "wrong sequence number (rcv %u, old %u)",
		   s->addr, csn, s->rx_csn);
      return 0;
    }

    byte *auth_data = alloca(hash_len);
    memcpy(auth_data, auth->data, hash_len);
    strncpy(auth->data, pass->password, hash_len);

    if (!mac_verify(hash_alg, NULL, 0, (byte *) pkt, pkt->length, auth_data))
      DROP("wrong authentication code", pass->id);

    s->rx_csn = csn;
    s->rx_csn_known = 1;

    return 1;
  }
  }

drop:
  LOG_PKT_AUTH("Authentication failed for %I - %s (%u)",
	       s->addr, err_dsc, err_val);
  return 0;
}

void
bfd_send_ctl(struct bfd_proto *p, struct bfd_session *s, int final)
{
  sock *sk = s->ifa->sk;
  struct bfd_ctl_packet *pkt;
  char fb[8];

  if (!sk)
    return;

  pkt = (struct bfd_ctl_packet *) sk->tbuf;
  pkt->vdiag = bfd_pack_vdiag(1, s->loc_diag);
  pkt->flags = bfd_pack_flags(s->loc_state, 0);
  pkt->detect_mult = s->detect_mult;
  pkt->length = BFD_BASE_LEN;
  pkt->snd_id = htonl(s->loc_id);
  pkt->rcv_id = htonl(s->rem_id);
  pkt->des_min_tx_int = htonl(s->des_min_tx_new);
  pkt->req_min_rx_int = htonl(s->req_min_rx_new);
  pkt->req_min_echo_rx_int = 0;

  if (final)
    pkt->flags |= BFD_FLAG_FINAL;
  else if (s->poll_active)
    pkt->flags |= BFD_FLAG_POLL;

  if (s->ifa->cf->auth_type)
    bfd_fill_authentication(p, s, pkt);

  if (sk->tbuf != sk->tpos)
    log(L_WARN "%s: Old packet overwritten in TX buffer", p->p.name);

  TRACE(D_PACKETS, "Sending CTL to %I [%s%s]", s->addr,
	bfd_state_names[s->loc_state], bfd_format_flags(pkt->flags, fb));

  sk_send_to(sk, pkt->length, s->addr, sk->dport);
}

static int
bfd_rx_hook(sock *sk, uint len)
{
  struct bfd_proto *p =  sk->data;
  struct bfd_ctl_packet *pkt = (struct bfd_ctl_packet *) sk->rbuf;
  const char *err_dsc = NULL;
  uint err_val = 0;
  char fb[8];

  if ((sk->sport == BFD_CONTROL_PORT) && (sk->rcv_ttl < 255))
    DROP("wrong TTL", sk->rcv_ttl);

  if (len < BFD_BASE_LEN)
    DROP("too short", len);

  u8 version = bfd_pkt_get_version(pkt);
  if (version != 1)
    DROP("version mismatch", version);

  if ((pkt->length < BFD_BASE_LEN) || (pkt->length > len))
    DROP("length mismatch", pkt->length);

  if (pkt->detect_mult == 0)
    DROP("invalid detect mult", 0);

  if ((pkt->flags & BFD_FLAG_MULTIPOINT) ||
      ((pkt->flags & BFD_FLAG_POLL) && (pkt->flags & BFD_FLAG_FINAL)))
    DROP("invalid flags", pkt->flags);

  if (pkt->snd_id == 0)
    DROP("invalid my discriminator", 0);

  struct bfd_session *s;
  u32 id = ntohl(pkt->rcv_id);

  if (id)
  {
    s = bfd_find_session_by_id(p, id);

    if (!s)
      DROP("unknown session id", id);
  }
  else
  {
    u8 ps = bfd_pkt_get_state(pkt);
    if (ps > BFD_STATE_DOWN)
      DROP("invalid init state", ps);

    s = bfd_find_session_by_addr(p, sk->faddr);

    /* FIXME: better session matching and message */
    if (!s)
      return 1;
  }

  /* bfd_check_authentication() has its own error logging */
  if (!bfd_check_authentication(p, s, pkt))
    return 1;

  u32 old_tx_int = s->des_min_tx_int;
  u32 old_rx_int = s->rem_min_rx_int;

  s->rem_id= ntohl(pkt->snd_id);
  s->rem_state = bfd_pkt_get_state(pkt);
  s->rem_diag = bfd_pkt_get_diag(pkt);
  s->rem_demand_mode = pkt->flags & BFD_FLAG_DEMAND;
  s->rem_min_tx_int = ntohl(pkt->des_min_tx_int);
  s->rem_min_rx_int = ntohl(pkt->req_min_rx_int);
  s->rem_detect_mult = pkt->detect_mult;

  TRACE(D_PACKETS, "CTL received from %I [%s%s]", sk->faddr,
	bfd_state_names[s->rem_state], bfd_format_flags(pkt->flags, fb));

  bfd_session_process_ctl(s, pkt->flags, old_tx_int, old_rx_int);
  return 1;

drop:
  LOG_PKT("Bad packet from %I - %s (%u)", sk->faddr, err_dsc, err_val);
  return 1;
}

static void
bfd_err_hook(sock *sk, int err)
{
  struct bfd_proto *p = sk->data;
  log(L_ERR "%s: Socket error: %m", p->p.name, err);
}

sock *
bfd_open_rx_sk(struct bfd_proto *p, int multihop)
{
  sock *sk = sk_new(p->tpool);
  sk->type = SK_UDP;
  sk->sport = !multihop ? BFD_CONTROL_PORT : BFD_MULTI_CTL_PORT;
  sk->data = p;

  sk->rbsize = BFD_MAX_LEN;
  sk->rx_hook = bfd_rx_hook;
  sk->err_hook = bfd_err_hook;

  /* TODO: configurable ToS and priority */
  sk->tos = IP_PREC_INTERNET_CONTROL;
  sk->priority = sk_priority_control;
  sk->flags = SKF_THREAD | SKF_LADDR_RX | (!multihop ? SKF_TTL_RX : 0);

#ifdef IPV6
  sk->flags |= SKF_V6ONLY;
#endif

  if (sk_open(sk) < 0)
    goto err;

  sk_start(sk);
  return sk;

 err:
  sk_log_error(sk, p->p.name);
  rfree(sk);
  return NULL;
}

sock *
bfd_open_tx_sk(struct bfd_proto *p, ip_addr local, struct iface *ifa)
{
  sock *sk = sk_new(p->tpool);
  sk->type = SK_UDP;
  sk->saddr = local;
  sk->dport = ifa ? BFD_CONTROL_PORT : BFD_MULTI_CTL_PORT;
  sk->iface = ifa;
  sk->data = p;

  sk->tbsize = BFD_MAX_LEN;
  sk->err_hook = bfd_err_hook;

  /* TODO: configurable ToS, priority and TTL security */
  sk->tos = IP_PREC_INTERNET_CONTROL;
  sk->priority = sk_priority_control;
  sk->ttl = ifa ? 255 : -1;
  sk->flags = SKF_THREAD | SKF_BIND | SKF_HIGH_PORT;

#ifdef IPV6
  sk->flags |= SKF_V6ONLY;
#endif

  if (sk_open(sk) < 0)
    goto err;

  sk_start(sk);
  return sk;

 err:
  sk_log_error(sk, p->p.name);
  rfree(sk);
  return NULL;
}
