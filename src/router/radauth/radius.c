/* Really simple radius authenticator
 *
 * Copyright (c) 2004 Michael Gernoth <michael@gernoth.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "md5.h"
#include "radius.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* RFC2138 */
#define ACCESS_REQUEST	0x1
#define ACCESS_ACCEPT	0x2
#define ACCESS_REJECT	0x3

#define USER_NAME	0x1
#define USER_PW		0x2
#define NAS_IP_ADDRESS	0x4
#define NAS_PORT	0x5
#define NAS_PORT_TYPE	0x3d
#define REPLY_MESSAGE	0x12

#define MACLENGTH	13
//#define DEBUG

void
md5_calc (unsigned char *output, unsigned char *input, unsigned int inlen)
{
  md5_state_t context;

  md5_init (&context);
  md5_append (&context, input, inlen);
  md5_finish (&context, output);
}

int
resolve_host (char *name)
{
  int ret;
  struct hostent *hent;

  if (!(hent = gethostbyname (name)))
    {
      fprintf (stderr, "Can't lookup %s!\n", name);
      exit (1);
    }

  ret = (*((int *) hent->h_addr_list[0]));

  return ret;
}

int
radius (char *host, short port, char *user, char *password, char *radsecret)
{
  int sock, ret, from_len;
  struct sockaddr_in addr, to_addr, from_addr;
  struct timeval tv;
  fd_set rfds;
  int length, seclength;
  short radlength;
  unsigned char buf[1025];
  unsigned char random[16];
  unsigned char *attributes;
  unsigned char *secret;
  unsigned char tmp;
  static unsigned char id = 1;
  static unsigned char seeded = 0;
  int i;
#ifdef DEBUG
printf("checking host %s port %d user %s password %s radsecret %s\n",host,port,user,password,radsecret);
#endif
  sock = socket (PF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (1);
    }

  memset (&addr, 0, sizeof (addr));
  addr.sin_family = PF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);

  ret = bind (sock, (struct sockaddr *) &addr, sizeof (addr));
  if (ret < 0)
    {
      perror ("bind");
      exit (1);
    }

  memset (&to_addr, 0, sizeof (to_addr));
  to_addr.sin_family = PF_INET;
  to_addr.sin_addr.s_addr = resolve_host (host);
  to_addr.sin_port = htons (port);

  memset (&buf, 0, sizeof (buf));
  buf[0] = ACCESS_REQUEST;
  buf[1] = id;			/*Id */

  /* Generate random here! */
  if (!seeded)
    {
      srand (time (NULL));
      seeded = 0x1;
    }
  memset (random, 0, 16);
  for (i = 0; i <= 15; i++)
    {
      tmp = rand ();
      memcpy (random + i, &tmp, 1);
    }
  memcpy (&buf[4], random, 16);

  /* Attributes: User-Name */
  attributes = &buf[20];
  *attributes = USER_NAME;
  attributes++;
  *attributes = (unsigned char) 1 + 1 + strlen (user);
  attributes++;
  memcpy (attributes, user, strlen (user));
  attributes += strlen (user);

  /* Attributes: NAS-Port */
  *attributes = NAS_PORT;
  attributes++;
  *attributes = 6;
  attributes++;
  *attributes = 0x00;
  attributes++;
  *attributes = 0x00;
  attributes++;
  *attributes = 0x00;
  attributes++;
  *attributes = 0x01;
  attributes++;

  /* Attributes: NAS-Port-Type */
  *attributes = NAS_PORT_TYPE;
  attributes++;
  *attributes = 6;
  attributes++;
  *attributes = 0x00;
  attributes++;
  *attributes = 0x00;
  attributes++;
  *attributes = 0x00;
  attributes++;
  *attributes = 0x13;
  attributes++;

  /* The horror of radius encryption */
  seclength = (((strlen (password) - 1) / 16) + 1) * 16;
  secret = malloc (seclength);
  if (secret == NULL)
    {
      perror ("malloc");
      exit (1);
    }
  memset (secret, 0, seclength);
  memcpy (secret, password, strlen (password));
  {
    unsigned char *secpos = secret;
    unsigned char b[16], clast[16], *md5in;
    int md5length;

    /* Pre-Initialize */
    memcpy (clast, random, 16);
    md5length = (strlen (radsecret)) + sizeof (clast);
    md5in = malloc (md5length);
    if (md5in == NULL)
      {
	perror ("malloc");
	exit (1);
      }

    while (secpos - secret < seclength)
      {
	memcpy (md5in, radsecret, strlen (radsecret));
	memcpy (md5in + (strlen (radsecret)), clast, sizeof (clast));

	md5_calc (b, md5in, md5length);
	for (i = 0; i <= 15; i++)
	  {
	    secpos[i] = secpos[i] ^ b[i];
	  }
	memcpy (clast, secpos, 16);
	secpos += 16;
      }
    free (md5in);
  }

  *attributes = USER_PW;
  attributes++;
  *attributes = (unsigned char) 1 + 1 + seclength;
  attributes++;
  memcpy (attributes, secret, seclength);
  attributes += seclength;
  free (secret);

  /* Code + Identifier + Length + Authenticator + Attributes */
  radlength = 1 + 1 + 2 + 16 + (attributes - &buf[20]);
  length = radlength;
  radlength = htons (radlength);
  memcpy (&buf[2], &radlength, 2);

  ret =
    sendto (sock, &buf, length, 0, (struct sockaddr *) &to_addr,
	    sizeof (to_addr));

  FD_ZERO (&rfds);
  FD_SET (sock, &rfds);

  tv.tv_sec = 10;
  tv.tv_usec = 0;
//waiting for response
  do
    {
      #ifdef DEBUG
      printf("select for response\n");
      #endif
      ret = select (sock + 1, &rfds, NULL, NULL, &tv);
      #ifdef DEBUG
      printf("done, returns %d\n",ret);
      #endif
      if (ret < 0)
	{
	  perror ("select");
	  exit (1);
	}
      if (ret == 0)
	{
	  /* Timeout, no packet received. Reject! */
	  close(sock);
	  return -10;		//ret=2; // try again

	}
      else
	{
	  ret =
	    recvfrom (sock, &buf, sizeof (buf), 0,
		      (struct sockaddr *) &from_addr, &from_len);
//	    recvfrom (sock, &buf, sizeof (buf) - 1, 0,
//		      (struct sockaddr *) &from_addr, sizeof(from_addr));
	  if (ret < 3)
	    {
      #ifdef DEBUG
      printf("invalid packet %d\n",ret);
      #endif
	      /* Completely invalid packet */
	      ret = -1;
	      continue;
	    }

	  if (buf[1] != id)
	    {
	      fprintf (stderr,
		       "Received faked radius-reply (we: %d, he: %d)!\n", id,
		       buf[1]);
	      ret = -1;
	      continue;
	    }

	  memcpy (&radlength, &buf[2], 2);
	  radlength = ntohs (radlength);
	  if (ret != radlength)
	    {
	      fprintf (stderr,
		       "Invalid packet received. Given length (%d) != real length (%d)!\n",
		       radlength, ret);
	      ret = -1;
	      continue;
	    }

	  /* Check response authenticator */
	  {
	    unsigned char *tohash;
	    unsigned char md5out[16];
	    int md5length;

	    /* Code+ID+Length+RequestAuth+Attributes+Secret */
	    md5length = radlength + strlen (radsecret);
	    tohash = malloc (md5length);
	    if (tohash == NULL)
	      {
		perror ("malloc");
		exit (1);
	      }

	    /* Code+ID+Length */
	    memcpy (tohash, buf, 4);
	    /* RequestAuth */
	    memcpy (&tohash[4], random, 16);
	    /* Attributes */
	    memcpy (&tohash[20], &buf[20], radlength - 20);
	    /* Secret */
	    memcpy (&tohash[20 + (radlength - 20)], radsecret,
		    strlen (radsecret));

	    md5_calc (md5out, tohash, md5length);

	    free (tohash);

	    if (memcmp (md5out, &buf[4], 16) != 0)
	      {
		fprintf (stderr,
			 "Faked radius response (wrong response-authenticator!)\n");
		ret = -1;
		continue;
	      }
	  }

	  {
	    unsigned char *pos;

	    pos = &buf[20];

	    while (pos - buf < radlength)
	      {
		unsigned char type = pos[0];
		unsigned char attrlength = pos[1];

		if (radlength - (pos - buf) >= attrlength)
		  {
		    /* printf("Found attribute %d, length: %d\n",type,attrlength); */
		    if (type == REPLY_MESSAGE)
		      {
			char msg[256];
			memset (msg, 0, 256);
			memcpy (&msg, &pos[2], attrlength - 2);
			printf ("Reply-Message: %s\n", msg);
		      }
		  }
		else
		  {
		    fprintf (stderr, "Error decoding attributes...\n");
		    break;
		  }

		pos += attrlength;
	      }
	  }
      #ifdef DEBUG
      printf("auth returns %d\n",buf[0]);
      #endif

	  if (buf[0] == 0x2)
	    {
	      /* Access-Accept */
	      ret = 1;
	    }
	  else
	    {
	      /* Access-Reject */
	      ret = 0;
	    }
	}
    }
  while (ret < 0);
  close (sock);

  /* Increment ID */
  id++;
  return (ret);
}
