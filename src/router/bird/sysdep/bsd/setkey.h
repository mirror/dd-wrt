/*
 *	BIRD -- Manipulation the IPsec SA/SP database using setkey(8) utility
 *
 * 	(c) 2016 CZ.NIC z.s.p.o.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/pfkeyv2.h>
#include <netipsec/ipsec.h>

#include "nest/bird.h"
#include "lib/unix.h"


/*
 * Open a socket for manage the IPsec SA/SP database entries
 */
static int
setkey_open_socket(void)
{
  int s = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
  if (s < 0)
  {
    log(L_ERR "SETKEY: socket: %m");
    return -1;
  }

  return s;
}

static int
setkey_send(struct sadb_msg *msg, uint len)
{
  int s = setkey_open_socket();
  if (s < 0)
    return -1;

  if (msg->sadb_msg_type == SADB_ADD)
  {
    /* Delete possible current key in the IPsec SA/SP database */
    msg->sadb_msg_type = SADB_DELETE;
    send(s, msg, len, 0);
    msg->sadb_msg_type = SADB_ADD;
  }

  if (send(s, msg, len, 0) < 0)
  {
    log(L_ERR "SETKEY: send: %m");
    close(s);
    return -1;
  }

  close(s);
  return 0;
}

/*
 * Perform setkey(8)-like operation for set the password for TCP MD5 Signature.
 * Could be called with SABD_ADD or SADB_DELETE argument. Note that SADB_ADD
 * argument is internally processed as a pair of SADB_ADD and SADB_DELETE
 * operations to implement replace.
 */
static int
setkey_md5(sockaddr *src, sockaddr *dst, char *passwd, uint type)
{
  uint passwd_len = passwd ? strlen(passwd) : 0;

  uint total =
    sizeof(struct sadb_msg) +
    sizeof(struct sadb_key) + PFKEY_ALIGN8(passwd_len) +
    sizeof(struct sadb_sa) +
    sizeof(struct sadb_x_sa2) +
    sizeof(struct sadb_address) + PFKEY_ALIGN8(src->sa.sa_len) +
    sizeof(struct sadb_address) + PFKEY_ALIGN8(dst->sa.sa_len);

  char *buf = alloca(total);
  char *pos = buf;
  uint len;

  memset(buf, 0, total);

  struct sadb_msg *msg = (void *) pos;
  len = sizeof(struct sadb_msg);
  msg->sadb_msg_version = PF_KEY_V2;
  msg->sadb_msg_type = type;
  msg->sadb_msg_satype = SADB_X_SATYPE_TCPSIGNATURE;
  msg->sadb_msg_len = 0;	/* Fix it later */
  msg->sadb_msg_pid = getpid();
  pos += len;

  /* Set authentication algorithm and password */
  struct sadb_key *key = (void *) pos;
  len = sizeof(struct sadb_key) + PFKEY_ALIGN8(passwd_len);
  key->sadb_key_len = PFKEY_UNIT64(len);
  key->sadb_key_exttype = SADB_EXT_KEY_AUTH;
  key->sadb_key_bits = passwd_len * 8;
  memcpy(pos + sizeof(struct sadb_key), passwd, passwd_len);
  pos += len;

  struct sadb_sa *sa = (void *) pos;
  len = sizeof(struct sadb_sa);
  sa->sadb_sa_len = PFKEY_UNIT64(len);
  sa->sadb_sa_exttype = SADB_EXT_SA;
  sa->sadb_sa_spi = htonl((u32) TCP_SIG_SPI);
  sa->sadb_sa_auth = SADB_X_AALG_TCP_MD5;
  sa->sadb_sa_encrypt = SADB_EALG_NONE;
  sa->sadb_sa_flags = SADB_X_EXT_CYCSEQ;
  pos += len;

  struct sadb_x_sa2 *sa2 = (void *) pos;
  len = sizeof(struct sadb_x_sa2);
  sa2->sadb_x_sa2_len = PFKEY_UNIT64(len);
  sa2->sadb_x_sa2_exttype = SADB_X_EXT_SA2;
  sa2->sadb_x_sa2_mode = IPSEC_MODE_ANY;
  pos += len;

  /* Set source address */
  struct sadb_address *saddr = (void *) pos;
  len = sizeof(struct sadb_address) + PFKEY_ALIGN8(src->sa.sa_len);
  saddr->sadb_address_len = PFKEY_UNIT64(len);
  saddr->sadb_address_exttype = SADB_EXT_ADDRESS_SRC;
  saddr->sadb_address_proto = IPSEC_ULPROTO_ANY;
  saddr->sadb_address_prefixlen = MAX_PREFIX_LENGTH;
  memcpy(pos + sizeof(struct sadb_address), &src->sa, src->sa.sa_len);
  pos += len;

  /* Set destination address */
  struct sadb_address *daddr = (void *) pos;
  len = sizeof(struct sadb_address) + PFKEY_ALIGN8(dst->sa.sa_len);
  daddr->sadb_address_len = PFKEY_UNIT64(len);
  daddr->sadb_address_exttype = SADB_EXT_ADDRESS_DST;
  daddr->sadb_address_proto = IPSEC_ULPROTO_ANY;
  daddr->sadb_address_prefixlen = MAX_PREFIX_LENGTH;
  memcpy(pos + sizeof(struct sadb_address), &dst->sa, dst->sa.sa_len);
  pos += len;

  len = pos - buf;
  msg->sadb_msg_len = PFKEY_UNIT64(len);

  return setkey_send(msg, len);
}

/*
 * Manipulation with the IPsec SA/SP database
 */
static int
sk_set_md5_in_sasp_db(sock *s, ip_addr local, ip_addr remote, struct iface *ifa, char *passwd)
{
  sockaddr src, dst;
  sockaddr_fill(&src, s->af, local, ifa, 0);
  sockaddr_fill(&dst, s->af, remote, ifa, 0);

  if (passwd && *passwd)
  {
    int len = strlen(passwd);
    if (len > TCP_KEYLEN_MAX)
      ERR_MSG("The password for TCP MD5 Signature is too long");

    if (setkey_md5(&src, &dst, passwd, SADB_ADD) < 0)
      ERR_MSG("Cannot add TCP-MD5 password into the IPsec SA/SP database");
  }
  else
  {
    if (setkey_md5(&src, &dst, NULL, SADB_DELETE) < 0)
      ERR_MSG("Cannot delete TCP-MD5 password from the IPsec SA/SP database");
  }
  return 0;
}
