/* This is a _simple_ program to lookup a hostname via. gethostbyname().
 *
 * This shows how easy it is to use custom format specifiers, to make your code
 * easier to write and maintain.
 *
 * This file is more commented than normal code, so as to make it easy to follow
 * while knowning almost nothing about Vstr or Linux IO programming.
 */
#define EX_UTILS_NO_USE_INPUT 1
#define EX_UTILS_NO_USE_OPEN 1

#include "ex_utils.h"

#include <sys/socket.h>
#include <netdb.h>


int main(int argc, char *argv[])
{
  Vstr_base *s1 = ex_init(NULL); /* init the library, and create a string */
  struct hostent *hp = NULL; /* data from the resolver library */

  /* setup the pre-written custom format specifier for IPv4 addresses,
   */
  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_sc_fmt_add_ipv4_ptr(s1->conf, "{IPv4:%p}");


  if (argc != 2) /* if a hostname isn't given output an message to stderr */
  {
    size_t pos = 0;
    size_t len = 0;

    /* add another format specifier for printing Vstr strings */
    vstr_sc_fmt_add_vstr(s1->conf, "{Vstr:%p%zu%zu%u}");

    /* find the program name ...
     * putting it at the begining of the Vstr string */
    vstr_add_cstr_ptr(s1, 0, argc ? argv[0] : "lookup_ip");
    vstr_sc_basename(s1, 1, s1->len, &pos, &len);

    /* add a format line to the Vstr string, including the program name
     * which is at the begining of this Vstr string itself */
    len = vstr_add_fmt(s1, s1->len, " %s ${Vstr:%p%zu%zu%u} %s\n",
                       "Format:",
                       s1, pos, len, 0,
                       "<hostname>");

    vstr_del(s1, 1, s1->len - len); /* delete the original program name */

    io_put_all(s1, STDERR_FILENO);

    exit (EXIT_FAILURE);
  }


  sethostent(1);
  /* call libc to lookup the hostname */
  hp = gethostbyname(argv[1]);


  /* just print the relevant data.... Note that nothing complicated needs to
   * be done to print the IPv4 address, the custom formatter takes care of
   * it */
  if (!hp)
    vstr_add_fmt(s1, 0, " Error retrieving hostname '%s': %s.\n",
                 argv[1], hstrerror(h_errno));
  else if (hp->h_addrtype == AF_INET)
    vstr_add_fmt(s1, 0, " The hostname '%s' has an "
                 "IPv4 address of \"${IPv4:%p}\".\n", hp->h_name,
                 hp->h_addr_list[0]);
  else
    vstr_add_fmt(s1, 0, " The hostname '%s' has an address type that "
                 "isn't an IPv4 address.\n",
                 hp->h_name);


  /* Cleanup... */
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, NULL));
}
