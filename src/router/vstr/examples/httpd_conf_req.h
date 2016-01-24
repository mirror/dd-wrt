#ifndef HTTPD_CONF_REQ_H
#define HTTPD_CONF_REQ_H

#include <vstr.h>

struct Con;
struct Httpd_req_data;

extern int httpd_conf_req_d0(struct Con *, struct Httpd_req_data *,
                             time_t, Conf_parse *, Conf_token *);
extern int httpd_conf_req_parse_file(Conf_parse *,
				     struct Con *, struct Httpd_req_data *);

#endif
