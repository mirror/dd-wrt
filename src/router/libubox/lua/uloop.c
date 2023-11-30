/*
 * Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../uloop.h"
#include "../list.h"

struct lua_uloop_fd {
	struct uloop_fd fd;
	int r;
	int fd_r;
};

struct lua_uloop_timeout {
	struct uloop_timeout t;
	int r;
};

struct lua_uloop_process {
	struct uloop_process p;
	int r;
};

struct lua_uloop_interval {
	struct uloop_interval i;
	int r;
};

struct lua_uloop_signal {
	struct uloop_signal s;
	int r;
};

static lua_State *state;

static void *
ul_create_userdata(lua_State *L, size_t size, const luaL_Reg *reg, lua_CFunction gc)
{
	void *ret = lua_newuserdata(L, size);

	memset(ret, 0, size);
	lua_createtable(L, 0, 2);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, gc);
	lua_setfield(L, -2, "__gc");
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -3);
	lua_pushvalue(L, -2);
	luaI_openlib(L, NULL, reg, 1);
	lua_pushvalue(L, -2);

	return ret;
}

static void ul_timer_cb(struct uloop_timeout *t)
{
	struct lua_uloop_timeout *tout = container_of(t, struct lua_uloop_timeout, t);

	lua_getglobal(state, "__uloop_cb");
	lua_rawgeti(state, -1, tout->r);
	lua_remove(state, -2);

	lua_call(state, 0, 0);

}

static int ul_timer_set(lua_State *L)
{
	struct lua_uloop_timeout *tout;
	double set;

	if (!lua_isnumber(L, -1)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

	set = lua_tointeger(L, -1);
	tout = lua_touserdata(L, 1);
	uloop_timeout_set(&tout->t, set);

	return 1;
}

static int ul_timer_remaining(lua_State *L)
{
	struct lua_uloop_timeout *tout;

	tout = lua_touserdata(L, 1);
	lua_pushnumber(L, uloop_timeout_remaining64(&tout->t));
	return 1;
}

static int ul_timer_free(lua_State *L)
{
	struct lua_uloop_timeout *tout = lua_touserdata(L, 1);

	uloop_timeout_cancel(&tout->t);

	/* obj.__index.__gc = nil , make sure executing only once*/
	lua_getfield(L, -1, "__index");
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_getglobal(state, "__uloop_cb");
	luaL_unref(state, -1, tout->r);

	return 1;
}

static const luaL_Reg timer_m[] = {
	{ "set", ul_timer_set },
	{ "remaining", ul_timer_remaining },
	{ "cancel", ul_timer_free },
	{ NULL, NULL }
};

static int ul_timer(lua_State *L)
{
	struct lua_uloop_timeout *tout;
	int set = 0;
	int ref;

	if (lua_isnumber(L, -1)) {
		set = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

	if (!lua_isfunction(L, -1)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

	lua_getglobal(L, "__uloop_cb");
	lua_pushvalue(L, -2);
	ref = luaL_ref(L, -2);

	tout = ul_create_userdata(L, sizeof(*tout), timer_m, ul_timer_free);
	tout->r = ref;
	tout->t.cb = ul_timer_cb;

	if (set)
		uloop_timeout_set(&tout->t, set);

	return 1;
}

static void ul_ufd_cb(struct uloop_fd *fd, unsigned int events)
{
	struct lua_uloop_fd *ufd = container_of(fd, struct lua_uloop_fd, fd);

	lua_getglobal(state, "__uloop_cb");
	lua_rawgeti(state, -1, ufd->r);
	lua_remove(state, -2);

	/* push fd object */
	lua_getglobal(state, "__uloop_fds");
	lua_rawgeti(state, -1, ufd->fd_r);
	lua_remove(state, -2);

	/* push events */
	lua_pushinteger(state, events);
	lua_call(state, 2, 0);
}


static int get_sock_fd(lua_State* L, int idx) {
	int fd;
	if(lua_isnumber(L, idx)) {
		fd = lua_tonumber(L, idx);
	} else {
		luaL_checktype(L, idx, LUA_TUSERDATA);
		lua_getfield(L, idx, "getfd");
		if(lua_isnil(L, -1))
			return luaL_error(L, "socket type missing 'getfd' method");
		/* if we have absolute, no need to adjust for getfield() call */
		lua_pushvalue(L, idx > 0 ? idx: idx - 1);
		lua_call(L, 1, 1);
		fd = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}
	return fd;
}

static int ul_ufd_delete(lua_State *L)
{
	struct lua_uloop_fd *ufd = lua_touserdata(L, 1);

	uloop_fd_delete(&ufd->fd);

	/* obj.__index.__gc = nil , make sure executing only once*/
	lua_getfield(L, -1, "__index");
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_getglobal(state, "__uloop_cb");
	luaL_unref(state, -1, ufd->r);
	lua_remove(state, -1);

	lua_getglobal(state, "__uloop_fds");
	luaL_unref(state, -1, ufd->fd_r);
	lua_remove(state, -1);

	return 1;
}

static const luaL_Reg ufd_m[] = {
	{ "delete", ul_ufd_delete },
	{ NULL, NULL }
};

static int ul_ufd_add(lua_State *L)
{
	struct lua_uloop_fd *ufd;
	int fd = 0;
	unsigned int flags = 0;
	int ref;
	int fd_ref;

	flags = luaL_checkinteger(L, 3);
	if (!flags) {
		lua_pushstring(L, "flags cannot be zero");
		lua_error(L);
	}

	luaL_checktype(L, 2, LUA_TFUNCTION);

	fd = get_sock_fd(L, 1);

	lua_getglobal(L, "__uloop_cb");
	lua_pushvalue(L, 2);
	ref = luaL_ref(L, -2);
	lua_pop(L, 1);

	lua_getglobal(L, "__uloop_fds");
	lua_pushvalue(L, 1);
	fd_ref = luaL_ref(L, -2);
	lua_pop(L, 1);

	ufd = ul_create_userdata(L, sizeof(*ufd), ufd_m, ul_ufd_delete);
	ufd->r = ref;
	ufd->fd.fd = fd;
	ufd->fd_r = fd_ref;
	ufd->fd.cb = ul_ufd_cb;
	uloop_fd_add(&ufd->fd, flags);

	return 1;
}

static int ul_process_free(lua_State *L)
{
	struct lua_uloop_process *proc = lua_touserdata(L, 1);

	/* obj.__index.__gc = nil , make sure executing only once*/
	lua_getfield(L, -1, "__index");
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

	if (proc->r != LUA_NOREF) {
		uloop_process_delete(&proc->p);

		lua_getglobal(state, "__uloop_cb");
		luaL_unref(state, -1, proc->r);
		lua_remove(state, -1);
	}

	return 1;
}

static int ul_process_pid(lua_State *L)
{
	struct lua_uloop_process *proc = lua_touserdata(L, 1);

	if (proc->p.pid) {
		lua_pushnumber(L, proc->p.pid);
		return 1;
	}

	return 0;
}

static const luaL_Reg process_m[] = {
	{ "delete", ul_process_free },
	{ "pid", ul_process_pid },
	{ NULL, NULL }
};

static void ul_process_cb(struct uloop_process *p, int ret)
{
	struct lua_uloop_process *proc = container_of(p, struct lua_uloop_process, p);

	lua_getglobal(state, "__uloop_cb");
	lua_rawgeti(state, -1, proc->r);

	luaL_unref(state, -2, proc->r);
	proc->r = LUA_NOREF;
	lua_remove(state, -2);
	lua_pushinteger(state, ret >> 8);
	lua_call(state, 1, 0);
}

static int ul_process(lua_State *L)
{
	struct lua_uloop_process *proc;
	pid_t pid;
	int ref;

	if (!lua_isfunction(L, -1) || !lua_istable(L, -2) ||
			!lua_istable(L, -3) || !lua_isstring(L, -4)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

	pid = fork();

	if (pid == -1) {
		lua_pushstring(L, "failed to fork");
		lua_error(L);

		return 0;
	}

	if (pid == 0) {
		/* child */
		int argn = lua_objlen(L, -3);
		int envn = lua_objlen(L, -2);
		char** argp = malloc(sizeof(char*) * (argn + 2));
		char** envp = malloc(sizeof(char*) * (envn + 1));
		int i = 1;

		if (!argp || !envp)
			_exit(-1);

		argp[0] = (char*) lua_tostring(L, -4);
		for (i = 1; i <= argn; i++) {
			lua_rawgeti(L, -3, i);
			argp[i] = (char*) lua_tostring(L, -1);
			lua_pop(L, 1);
		}
		argp[i] = NULL;

		for (i = 1; i <= envn; i++) {
			lua_rawgeti(L, -2, i);
			envp[i - 1] = (char*) lua_tostring(L, -1);
			lua_pop(L, 1);
		}
		envp[i - 1] = NULL;

		execve(*argp, argp, envp);
		_exit(-1);
	}

	lua_getglobal(L, "__uloop_cb");
	lua_pushvalue(L, -2);
	ref = luaL_ref(L, -2);

	proc = ul_create_userdata(L, sizeof(*proc), process_m, ul_process_free);
	proc->r = ref;
	proc->p.pid = pid;
	proc->p.cb = ul_process_cb;
	uloop_process_add(&proc->p);

	return 1;
}

static void ul_interval_cb(struct uloop_interval *i)
{
	struct lua_uloop_interval *intv = container_of(i, struct lua_uloop_interval, i);

	lua_getglobal(state, "__uloop_cb");
	lua_rawgeti(state, -1, intv->r);
	lua_remove(state, -2);

	lua_call(state, 0, 0);
}

static int ul_interval_set(lua_State *L)
{
	struct lua_uloop_interval *intv;
	double set;

	if (!lua_isnumber(L, -1)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

	set = lua_tointeger(L, -1);
	intv = lua_touserdata(L, 1);
	uloop_interval_set(&intv->i, set);

	return 1;
}

static int ul_interval_expirations(lua_State *L)
{
	struct lua_uloop_interval *intv;

	intv = lua_touserdata(L, 1);
	lua_pushinteger(L, intv->i.expirations);
	return 1;
}

static int ul_interval_remaining(lua_State *L)
{
	struct lua_uloop_interval *intv;

	intv = lua_touserdata(L, 1);
	lua_pushnumber(L, uloop_interval_remaining(&intv->i));
	return 1;
}

static int ul_interval_free(lua_State *L)
{
	struct lua_uloop_interval *intv = lua_touserdata(L, 1);

	uloop_interval_cancel(&intv->i);

	/* obj.__index.__gc = nil , make sure executing only once*/
	lua_getfield(L, -1, "__index");
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_getglobal(state, "__uloop_cb");
	luaL_unref(state, -1, intv->r);

	return 1;
}

static const luaL_Reg interval_m[] = {
	{ "set", ul_interval_set },
	{ "expirations", ul_interval_expirations },
	{ "remaining", ul_interval_remaining },
	{ "cancel", ul_interval_free },
	{ NULL, NULL }
};

static int ul_interval(lua_State *L)
{
	struct lua_uloop_interval *intv;
	unsigned int set = 0;
	int ref;

	if (lua_isnumber(L, -1)) {
		set = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

	if (!lua_isfunction(L, -1)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

	lua_getglobal(L, "__uloop_cb");
	lua_pushvalue(L, -2);
	ref = luaL_ref(L, -2);

	intv = ul_create_userdata(L, sizeof(*intv), interval_m, ul_interval_free);
	intv->r = ref;
	intv->i.cb = ul_interval_cb;

	if (set)
		uloop_interval_set(&intv->i, set);

	return 1;
}

static void ul_signal_cb(struct uloop_signal *s)
{
	struct lua_uloop_signal *sig = container_of(s, struct lua_uloop_signal, s);

	lua_getglobal(state, "__uloop_cb");
	lua_rawgeti(state, -1, sig->r);
	lua_remove(state, -2);
	lua_pushinteger(state, sig->s.signo);

	lua_call(state, 1, 0);
}

static int ul_signal_signo(lua_State *L)
{
	struct lua_uloop_signal *sig = lua_touserdata(L, 1);

	lua_pushinteger(L, sig->s.signo);

	return 1;
}

static int ul_signal_free(lua_State *L)
{
	struct lua_uloop_signal *sig = lua_touserdata(L, 1);

	uloop_signal_delete(&sig->s);

	/* obj.__index.__gc = nil , make sure executing only once*/
	lua_getfield(L, -1, "__index");
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_getglobal(state, "__uloop_cb");
	luaL_unref(state, -1, sig->r);

	return 1;
}

static const luaL_Reg signal_m[] = {
	{ "signo", ul_signal_signo },
	{ "delete", ul_signal_free },
	{ NULL, NULL }
};

static int ul_signal(lua_State *L)
{
	struct lua_uloop_signal *sig;
	int signo = -1;
	int ref;

	if (lua_isnumber(L, -1)) {
		signo = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

	if (!lua_isfunction(L, -1) || signo <= 0 || signo > NSIG) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

	lua_getglobal(L, "__uloop_cb");
	lua_pushvalue(L, -2);
	ref = luaL_ref(L, -2);

	sig = ul_create_userdata(L, sizeof(*sig), signal_m, ul_signal_free);
	sig->r = ref;
	sig->s.cb = ul_signal_cb;
	sig->s.signo = signo;

	uloop_signal_add(&sig->s);

	return 1;
}

static int ul_init(lua_State *L)
{
	uloop_init();
	lua_pushboolean(L, 1);

	return 1;
}

static int ul_run(lua_State *L)
{
	uloop_run();
	lua_pushboolean(L, 1);

	return 1;
}

static int ul_end(lua_State *L)
{
	uloop_end();
	return 1;
}

static luaL_reg uloop_func[] = {
	{"init", ul_init},
	{"run", ul_run},
	{"timer", ul_timer},
	{"process", ul_process},
	{"fd_add", ul_ufd_add},
	{"interval", ul_interval},
	{"signal", ul_signal},
	{"cancel", ul_end},
	{NULL, NULL},
};

/* avoid warnings about missing declarations */
int luaopen_uloop(lua_State *L);
int luaclose_uloop(lua_State *L);

int luaopen_uloop(lua_State *L)
{
	state = L;

	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__uloop_cb");

	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__uloop_fds");

	luaL_openlib(L, "uloop", uloop_func, 0);
	lua_pushstring(L, "_VERSION");
	lua_pushstring(L, "1.0");
	lua_rawset(L, -3);

	lua_pushstring(L, "ULOOP_READ");
	lua_pushinteger(L, ULOOP_READ);
	lua_rawset(L, -3);

	lua_pushstring(L, "ULOOP_WRITE");
	lua_pushinteger(L, ULOOP_WRITE);
	lua_rawset(L, -3);

	lua_pushstring(L, "ULOOP_EDGE_TRIGGER");
	lua_pushinteger(L, ULOOP_EDGE_TRIGGER);
	lua_rawset(L, -3);

	lua_pushstring(L, "ULOOP_BLOCKING");
	lua_pushinteger(L, ULOOP_BLOCKING);
	lua_rawset(L, -3);

#define SIGNAME_CONST(signame) do { \
	lua_pushstring(L, #signame);    \
	lua_pushinteger(L, signame);    \
	lua_rawset(L, -3);	            \
} while(0)

#if defined(SIGINT)
	SIGNAME_CONST(SIGINT);
#endif
#if defined(SIGILL)
	SIGNAME_CONST(SIGILL);
#endif
#if defined(SIGABRT)
	SIGNAME_CONST(SIGABRT);
#endif
#if defined(SIGFPE)
	SIGNAME_CONST(SIGFPE);
#endif
#if defined(SIGSEGV)
	SIGNAME_CONST(SIGSEGV);
#endif
#if defined(SIGTERM)
	SIGNAME_CONST(SIGTERM);
#endif
#if defined(SIGHUP)
	SIGNAME_CONST(SIGHUP);
#endif
#if defined(SIGQUIT)
	SIGNAME_CONST(SIGQUIT);
#endif
#if defined(SIGTRAP)
	SIGNAME_CONST(SIGTRAP);
#endif
#if defined(SIGKILL)
	SIGNAME_CONST(SIGKILL);
#endif
#if defined(SIGPIPE)
	SIGNAME_CONST(SIGPIPE);
#endif
#if defined(SIGALRM)
	SIGNAME_CONST(SIGALRM);
#endif
#if defined(SIGSTKFLT)
	SIGNAME_CONST(SIGSTKFLT);
#endif
#if defined(SIGPWR)
	SIGNAME_CONST(SIGPWR);
#endif
#if defined(SIGBUS)
	SIGNAME_CONST(SIGBUS);
#endif
#if defined(SIGSYS)
	SIGNAME_CONST(SIGSYS);
#endif
#if defined(SIGURG)
	SIGNAME_CONST(SIGURG);
#endif
#if defined(SIGSTOP)
	SIGNAME_CONST(SIGSTOP);
#endif
#if defined(SIGTSTP)
	SIGNAME_CONST(SIGTSTP);
#endif
#if defined(SIGCONT)
	SIGNAME_CONST(SIGCONT);
#endif
#if defined(SIGCHLD)
	SIGNAME_CONST(SIGCHLD);
#endif
#if defined(SIGTTIN)
	SIGNAME_CONST(SIGTTIN);
#endif
#if defined(SIGTTOU)
	SIGNAME_CONST(SIGTTOU);
#endif
#if defined(SIGPOLL)
	SIGNAME_CONST(SIGPOLL);
#endif
#if defined(SIGXFSZ)
	SIGNAME_CONST(SIGXFSZ);
#endif
#if defined(SIGXCPU)
	SIGNAME_CONST(SIGXCPU);
#endif
#if defined(SIGVTALRM)
	SIGNAME_CONST(SIGVTALRM);
#endif
#if defined(SIGPROF)
	SIGNAME_CONST(SIGPROF);
#endif
#if defined(SIGUSR1)
	SIGNAME_CONST(SIGUSR1);
#endif
#if defined(SIGUSR2)
	SIGNAME_CONST(SIGUSR2);
#endif

	return 1;
}

int luaclose_uloop(lua_State *L)
{
	lua_pushstring(L, "Called");

	return 1;
}
