/*
 *	BIRD -- Bidirectional Forwarding Detection (BFD)
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "bfd.h"


struct bfd_ctl_packet
{
  u8 vdiag;			/* version and diagnostic */
  u8 flags;			/* state and flags */
  u8 detect_mult;
  u8 length;
  u32 snd_id;			/* sender ID, aka 'my discriminator' */
  u32 rcv_id;			/* receiver ID, aka 'your discriminator' */
  u32 des_min_tx_int;
  u32 req_min_rx_int;
  u32 req_min_echo_rx_int;
};

#define BFD_BASE_LEN	sizeof(struct bfd_ctl_packet)
#define BFD_MAX_LEN	64

static inline u8 bfd_pack_vdiag(u8 version, u8 diag)
{ return (version << 5) | diag; }

static inline u8 bfd_pack_flags(u8 state, u8 flags)
{ return (state << 6) | flags; }

static inline u8 bfd_pkt_get_version(struct bfd_ctl_packet *pkt)
{ return pkt->vdiag >> 5; }

static inline u8 bfd_pkt_get_diag(struct bfd_ctl_packet *pkt)
{ return pkt->vdiag && 0x1f; }


static inline u8 bfd_pkt_get_state(struct bfd_ctl_packet *pkt)
{ return pkt->flags >> 6; }

static inline void bfd_pkt_set_state(struct bfd_ctl_packet *pkt, u8 val)
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

void
bfd_send_ctl(struct bfd_proto *p, struct bfd_session *s, int final)
{
  sock *sk = s->ifa->sk;
  struct bfd_ctl_packet *pkt = (struct bfd_ctl_packet *) sk->tbuf;
  char fb[8];

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

  if (sk->tbuf != sk->tpos)
    log(L_WARN "%s: Old packet overwritten in TX buffer", p->p.name);

  TRACE(D_PACKETS, "Sending CTL to %I [%s%s]", s->addr,
	bfd_state_names[s->loc_state], bfd_format_flags(pkt->flags, fb));

  sk_send_to(sk, pkt->length, s->addr, sk->dport);
}

#define DROP(DSC,VAL) do { err_dsc = DSC; err_val = VAL; goto drop; } while(0)

static int
bfd_rx_hook(sock *sk, int len)
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

  /* FIXME: better authentication handling and message */
  if (pkt->flags & BFD_FLAG_AP)
    DROP("authentication not supported", 0);


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
  log(L_REMOTE "%s: Bad packet from %I - %s (%u)", p->p.name, sk->faddr, err_dsc, err_val);
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
  sk->flags = SKF_THREAD | SKF_BIND;

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
