#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#include <event2/event.h>

#include "conffile.h"
#include "reexport_backend.h"
#include "reexport.h"
#include "xcommon.h"
#include "xlog.h"

static struct event_base *evbase;
static struct reexpdb_backend_plugin *dbbackend = &sqlite_plug_ops;

/* assert_safe() always evalutes it argument, as it might have
 * a side-effect.  assert() won't if compiled with NDEBUG
 */
#define assert_safe(__sideeffect) (__sideeffect ? 0 : ({assert(0) ; 0;}))

static void client_cb(evutil_socket_t cl, short ev, void *d)
{
	struct event *me = d;
	char buf[PATH_MAX * 2];
	int n;

	(void)ev;

	n = recv(cl, buf, sizeof(buf) - 1, 0);
	if (n <= 0) {
		event_del(me);
		event_free(me);
		close(cl);
		return;
	}

	buf[n] = '\0';

	if (strncmp(buf, "get_fsidnum ", strlen("get_fsidnum ")) == 0) {
		char *req_path = buf + strlen("get_fsidnum ");
		uint32_t fsidnum;
		char *answer = NULL;
		bool found;

		assert(req_path < buf + n );

		printf("client asks for %s\n", req_path);

		if (dbbackend->fsidnum_by_path(req_path, &fsidnum, false, &found)) {
			if (found)
				assert_safe(asprintf(&answer, "+ %u", fsidnum) != -1);
			else
				assert_safe(asprintf(&answer, "+ ") != -1);
		} else {
			assert_safe(asprintf(&answer, "- %s", "Command failed") != -1);
		}

		(void)send(cl, answer, strlen(answer), 0);

		free(answer);
	} else if (strncmp(buf, "get_or_create_fsidnum ", strlen("get_or_create_fsidnum ")) == 0) {
		char *req_path = buf + strlen("get_or_create_fsidnum ");
		uint32_t fsidnum;
		char *answer = NULL;
		bool found;

		assert(req_path < buf + n );


		if (dbbackend->fsidnum_by_path(req_path, &fsidnum, true, &found)) {
			if (found) {
				assert_safe(asprintf(&answer, "+ %u", fsidnum) != -1);
			} else {
				assert_safe(asprintf(&answer, "+ ") != -1);
			}
		
		} else {
			assert_safe(asprintf(&answer, "- %s", "Command failed") != -1);
		}

		(void)send(cl, answer, strlen(answer), 0);

		free(answer);
	} else if (strncmp(buf, "get_path ", strlen("get_path ")) == 0) {
		char *req_fsidnum = buf + strlen("get_path ");
		char *path = NULL, *answer = NULL, *endp;
		bool bad_input = true;
		uint32_t fsidnum;
		bool found;

		assert(req_fsidnum < buf + n );

		errno = 0;
		fsidnum = strtoul(req_fsidnum, &endp, 10);
		if (errno == 0 && *endp == '\0') {
			bad_input = false;
		}

		if (bad_input) {
			assert_safe(asprintf(&answer, "- %s", "Command failed: Bad input") != -1);
		} else {
			if (dbbackend->path_by_fsidnum(fsidnum, &path, &found)) {
				if (found)
					assert_safe(asprintf(&answer, "+ %s", path) != -1);
				else
					assert_safe(asprintf(&answer, "+ ") != -1);
			} else {
				assert_safe(asprintf(&answer, "+ ") != -1);
			}
		}

		(void)send(cl, answer, strlen(answer), 0);

		free(path);
		free(answer);
	} else if (strcmp(buf, "version") == 0) {
		char answer[] = "+ 1";

		(void)send(cl, answer, strlen(answer), 0);
	} else {
		char *answer = NULL;

		assert_safe(asprintf(&answer, "- bad command") != -1);
		(void)send(cl, answer, strlen(answer), 0);

		free(answer);
	}
}

static void srv_cb(evutil_socket_t fd, short ev, void *d)
{
	int cl = accept4(fd, NULL, NULL, SOCK_NONBLOCK);
	struct event *client_ev;
	
	(void)ev;
	(void)d;

	client_ev = event_new(evbase, cl, EV_READ | EV_PERSIST | EV_CLOSED, client_cb, event_self_cbarg());
	event_add(client_ev, NULL);
}

int main(void)
{
	struct event *srv_ev;
	struct sockaddr_un addr;
	char *sock_file;
	int srv;

	conf_init_file(NFS_CONFFILE);

	if (!dbbackend->initdb()) {
		return 1;
	}

	sock_file = conf_get_str_with_def("reexport", "fsidd_socket", FSID_SOCKET_NAME);

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_file, sizeof(addr.sun_path) - 1);
	if (addr.sun_path[0] == '@')
		/* "abstract" socket namespace */
		addr.sun_path[0] = 0;
	else
		unlink(sock_file);

	srv = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
	if (srv == -1) {
		xlog(L_WARNING, "Unable to create AF_UNIX socket for %s: %m\n", sock_file);
		return 1;
	}

	if (bind(srv, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
		xlog(L_WARNING, "Unable to bind %s: %m\n", sock_file);
		return 1;
	}

	if (listen(srv, 5) == -1) {
		xlog(L_WARNING, "Unable to listen on %s: %m\n", sock_file);
		return 1;
	}

	evbase = event_base_new();

	srv_ev = event_new(evbase, srv, EV_READ | EV_PERSIST, srv_cb, NULL);
	event_add(srv_ev, NULL);

	event_base_dispatch(evbase);

	dbbackend->destroydb();

	return 0;
}
