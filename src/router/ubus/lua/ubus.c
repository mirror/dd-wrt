/*
 * Copyright (C) 2012 Jo-Philipp Wich <jow@openwrt.org>
 * Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 * Copyright (C) 2016 Iain Fraser <iainf@netduma.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <unistd.h>
#include <libubus.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <lauxlib.h>
#include <lua.h>

#define MODNAME		"ubus"
#define METANAME	MODNAME ".meta"

static lua_State *state;

struct ubus_lua_connection {
	int timeout;
	struct blob_buf buf;
	struct ubus_context *ctx;
};

struct ubus_lua_object {
	struct ubus_object o;
	int r;
	int rsubscriber;
};

struct ubus_lua_event {
	struct ubus_event_handler e;
	int r;
};

struct ubus_lua_subscriber {
	struct ubus_subscriber s;
	int rnotify;
	int rremove;
};

static int
ubus_lua_parse_blob(lua_State *L, struct blob_attr *attr, bool table);

static int
ubus_lua_parse_blob_array(lua_State *L, struct blob_attr *attr, size_t len, bool table)
{
	int rv;
	int idx = 1;
	size_t rem = len;
	struct blob_attr *pos;

	lua_newtable(L);

	__blob_for_each_attr(pos, attr, rem)
	{
		rv = ubus_lua_parse_blob(L, pos, table);

		if (rv > 1)
			lua_rawset(L, -3);
		else if (rv > 0)
			lua_rawseti(L, -2, idx++);
	}

	return 1;
}

static int
ubus_lua_parse_blob(lua_State *L, struct blob_attr *attr, bool table)
{
	int len;
	int off = 0;
	void *data;

	if (!blobmsg_check_attr(attr, false))
		return 0;

	if (table && blobmsg_name(attr)[0])
	{
		lua_pushstring(L, blobmsg_name(attr));
		off++;
	}

	data = blobmsg_data(attr);
	len = blobmsg_data_len(attr);

	switch (blob_id(attr))
	{
	case BLOBMSG_TYPE_BOOL:
		lua_pushboolean(L, *(uint8_t *)data);
		break;

	case BLOBMSG_TYPE_INT16:
		lua_pushinteger(L, be16_to_cpu(*(uint16_t *)data));
		break;

	case BLOBMSG_TYPE_INT32:
		lua_pushinteger(L, be32_to_cpu(*(uint32_t *)data));
		break;

	case BLOBMSG_TYPE_INT64:
		lua_pushnumber(L, (double) be64_to_cpu(*(uint64_t *)data));
		break;

	case BLOBMSG_TYPE_DOUBLE:
		{
			union {
				double d;
				uint64_t u64;
			} v;
			v.u64 = be64_to_cpu(*(uint64_t *)data);
			lua_pushnumber(L, v.d);
		}
		break;

	case BLOBMSG_TYPE_STRING:
		lua_pushstring(L, data);
		break;

	case BLOBMSG_TYPE_ARRAY:
		ubus_lua_parse_blob_array(L, data, len, false);
		break;

	case BLOBMSG_TYPE_TABLE:
		ubus_lua_parse_blob_array(L, data, len, true);
		break;

	default:
		lua_pushnil(L);
		break;
	}

	return off + 1;
}


static bool
ubus_lua_format_blob_is_array(lua_State *L)
{
	lua_Integer prv = 0;
	lua_Integer cur = 0;

	/* Find out whether table is array-like */
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1))
	{
#ifdef LUA_TINT
		if (lua_type(L, -2) != LUA_TNUMBER && lua_type(L, -2) != LUA_TINT)
#else
		if (lua_type(L, -2) != LUA_TNUMBER)
#endif
		{
			lua_pop(L, 2);
			return false;
		}

		cur = lua_tointeger(L, -2);

		if ((cur - 1) != prv)
		{
			lua_pop(L, 2);
			return false;
		}

		prv = cur;
	}

	return true;
}

static int
ubus_lua_format_blob_array(lua_State *L, struct blob_buf *b, bool table);

static int
ubus_lua_format_blob(lua_State *L, struct blob_buf *b, bool table)
{
	void *c;
	bool rv = true;
	const char *key = table ? lua_tostring(L, -2) : NULL;

	switch (lua_type(L, -1))
	{
	case LUA_TBOOLEAN:
		blobmsg_add_u8(b, key, (uint8_t)lua_toboolean(L, -1));
		break;

#ifdef LUA_TINT
	case LUA_TINT:
#endif
	case LUA_TNUMBER:
		if ((uint64_t)lua_tonumber(L, -1) > INT_MAX)
			blobmsg_add_u64(b, key, (uint64_t)lua_tonumber(L, -1));
		else
			blobmsg_add_u32(b, key, (uint32_t)lua_tointeger(L, -1));
		break;

	case LUA_TSTRING:
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		blobmsg_add_string(b, key, lua_tostring(L, -1));
		break;

	case LUA_TTABLE:
		if (ubus_lua_format_blob_is_array(L))
		{
			c = blobmsg_open_array(b, key);
			rv = ubus_lua_format_blob_array(L, b, false);
			blobmsg_close_array(b, c);
		}
		else
		{
			c = blobmsg_open_table(b, key);
			rv = ubus_lua_format_blob_array(L, b, true);
			blobmsg_close_table(b, c);
		}
		break;

	default:
		rv = false;
		break;
	}

	return rv;
}

static int
ubus_lua_format_blob_array(lua_State *L, struct blob_buf *b, bool table)
{
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1))
	{
		if (!ubus_lua_format_blob(L, b, table))
		{
			lua_pop(L, 1);
			return false;
		}
	}

	return true;
}


static int
ubus_lua_connect(lua_State *L)
{
	struct ubus_lua_connection *c;
	const char *sockpath = luaL_optstring(L, 1, NULL);
	int timeout = luaL_optint(L, 2, 30);

	if ((c = lua_newuserdata(L, sizeof(*c))) != NULL &&
		(c->ctx = ubus_connect(sockpath)) != NULL)
	{
		ubus_add_uloop(c->ctx);
		c->timeout = timeout;
		memset(&c->buf, 0, sizeof(c->buf));
		luaL_getmetatable(L, METANAME);
		lua_setmetatable(L, -2);
		return 1;
	}

	/* NB: no errors from ubus_connect() yet */
	lua_pushnil(L);
	lua_pushinteger(L, UBUS_STATUS_UNKNOWN_ERROR);
	return 2;
}


static void
ubus_lua_objects_cb(struct ubus_context *c, struct ubus_object_data *o, void *p)
{
	lua_State *L = (lua_State *)p;

	lua_pushstring(L, o->path);
	lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
}

static int
ubus_lua_objects(lua_State *L)
{
	int rv;
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);
	const char *path = (lua_gettop(L) >= 2) ? luaL_checkstring(L, 2) : NULL;

	lua_newtable(L);
	rv = ubus_lookup(c->ctx, path, ubus_lua_objects_cb, L);

	if (rv != UBUS_STATUS_OK)
	{
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_pushinteger(L, rv);
		return 2;
	}

	return 1;
}

static int
ubus_method_handler(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct ubus_lua_object *o = container_of(obj, struct ubus_lua_object, o);
	int rv = 0;

	lua_getglobal(state, "__ubus_cb");
	lua_rawgeti(state, -1, o->r);
	lua_getfield(state, -1, method);
	lua_remove(state, -2);
	lua_remove(state, -2);

	if (lua_isfunction(state, -1)) {
		lua_pushlightuserdata(state, req);
		if (!msg)
			lua_pushnil(state);
		else
			ubus_lua_parse_blob_array(state, blob_data(msg), blob_len(msg), true);
		lua_call(state, 2, 1);
		if (lua_isnumber(state, -1))
			rv = lua_tonumber(state, -1);
	}

	lua_pop(state, 1);

	return rv;
}

static int lua_gettablelen(lua_State *L, int index)
{
	int cnt = 0;

	lua_pushnil(L);
	index -= 1;
	while (lua_next(L, index) != 0) {
		cnt++;
		lua_pop(L, 1);
	}

	return cnt;
}

static int ubus_lua_reply(lua_State *L)
{
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);
	struct ubus_request_data *req;

	luaL_checktype(L, 3, LUA_TTABLE);
	blob_buf_init(&c->buf, 0);

	if (!ubus_lua_format_blob_array(L, &c->buf, true))
	{
		lua_pushnil(L);
		lua_pushinteger(L, UBUS_STATUS_INVALID_ARGUMENT);
		return 2;
	}

	req = lua_touserdata(L, 2);
	ubus_send_reply(c->ctx, req, c->buf.head);

	return 0;
}

static int ubus_lua_defer_request(lua_State *L)
{
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);
	struct ubus_request_data *req = lua_touserdata(L, 2);
	struct ubus_request_data *new_req = lua_newuserdata(L, sizeof(struct ubus_request_data));
	ubus_defer_request(c->ctx, req, new_req);

	return 1;
}

static int ubus_lua_complete_deferred_request(lua_State *L)
{
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);
	struct ubus_request_data *req = lua_touserdata(L, 2);
	int ret = luaL_checkinteger(L, 3);
	ubus_complete_deferred_request(c->ctx, req, ret);

	return 0;
}

static int ubus_lua_load_methods(lua_State *L, struct ubus_method *m)
{
	struct blobmsg_policy *p;
	int plen;
	int pidx = 0;

	/* get the function pointer */
	lua_pushinteger(L, 1);
	lua_gettable(L, -2);

	/* get the policy table */
	lua_pushinteger(L, 2);
	lua_gettable(L, -3);

	/* check if the method table is valid */
	if ((lua_type(L, -2) != LUA_TFUNCTION) ||
			(lua_type(L, -1) != LUA_TTABLE) ||
			lua_objlen(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}

	/* store function pointer */
	lua_pushvalue(L, -2);
	lua_setfield(L, -6, lua_tostring(L, -5));

	m->name = lua_tostring(L, -4);
	m->handler = ubus_method_handler;

	plen = lua_gettablelen(L, -1);

	/* exit if policy table is empty */
	if (!plen) {
		lua_pop(L, 2);
		return 0;
	}

	/* setup the policy pointers */
	p = calloc(plen, sizeof(struct blobmsg_policy));
	if (!p)
		return 1;

	m->policy = p;
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		int val = lua_tointeger(L, -1);

		/* check if the policy is valid */
		if ((lua_type(L, -2) != LUA_TSTRING) ||
				(lua_type(L, -1) != LUA_TNUMBER) ||
				(val < 0) ||
				(val > BLOBMSG_TYPE_LAST)) {
			lua_pop(L, 1);
			continue;
		}
		p[pidx].name = lua_tostring(L, -2);
		p[pidx].type = val;
		lua_pop(L, 1);
		pidx++;
	}

	m->n_policy = pidx;
	lua_pop(L, 2);

	return 0;
}

static void
ubus_new_sub_cb(struct ubus_context *ctx, struct ubus_object *obj)
{
	struct ubus_lua_object *luobj;

	luobj = container_of(obj, struct ubus_lua_object, o);

	lua_getglobal(state, "__ubus_cb_publisher");
	lua_rawgeti(state, -1, luobj->rsubscriber);
	lua_remove(state, -2);

	if (lua_isfunction(state, -1)) {
		lua_pushnumber(state, luobj->o.has_subscribers );
		lua_call(state, 1, 0);
	} else {
		lua_pop(state, 1);
	}
}

static void
ubus_lua_load_newsub_cb( lua_State *L, struct ubus_lua_object *obj )
{
	/* keep ref to func */
	lua_getglobal(L, "__ubus_cb_publisher");
	lua_pushvalue(L, -2);
	obj->rsubscriber = luaL_ref(L, -2);
	lua_pop(L, 1);

	/* real callback */
	obj->o.subscribe_cb = ubus_new_sub_cb;
	return;
}

static struct ubus_object* ubus_lua_load_object(lua_State *L)
{
	struct ubus_lua_object *obj = NULL;
	int mlen = lua_gettablelen(L, -1);
	struct ubus_method *m;
	int midx = 0;

	/* setup object pointers */
	obj = calloc(1, sizeof(struct ubus_lua_object));
	if (!obj)
		return NULL;

	obj->o.name = lua_tostring(L, -2);

	/* setup method pointers */
	m = calloc(mlen, sizeof(struct ubus_method));
	obj->o.methods = m;

	/* setup type pointers */
	obj->o.type = calloc(1, sizeof(struct ubus_object_type));
	if (!obj->o.type) {
		free(obj);
		return NULL;
	}

	obj->o.type->name = lua_tostring(L, -2);
	obj->o.type->id = 0;
	obj->o.type->methods = obj->o.methods;

	/* create the callback lookup table */
	lua_createtable(L, 1, 0);
	lua_getglobal(L, "__ubus_cb");
	lua_pushvalue(L, -2);
	obj->r = luaL_ref(L, -2);
	lua_pop(L, 1);

	/* scan each method */
	lua_pushnil(L);
	while (lua_next(L, -3) != 0) {
                /* check if its the subscriber notification callback */
                if( lua_type( L, -2 ) == LUA_TSTRING &&
                                lua_type( L, -1 ) == LUA_TFUNCTION ){
                  if( !strcmp( lua_tostring( L, -2 ), "__subscriber_cb" ) )
                          ubus_lua_load_newsub_cb( L, obj );
                }

		/* check if it looks like a method */
		if ((lua_type(L, -2) != LUA_TSTRING) ||
				(lua_type(L, -1) != LUA_TTABLE) ||
				!lua_objlen(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		if (!ubus_lua_load_methods(L, &m[midx]))
			midx++;
		lua_pop(L, 1);
	}

	obj->o.type->n_methods = obj->o.n_methods = midx;

	/* pop the callback table */
	lua_pop(L, 1);

	return &obj->o;
}

static int ubus_lua_add(lua_State *L)
{
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);

	/* verify top level object */
	if (lua_istable(L, 1)) {
		lua_pushstring(L, "you need to pass a table");
		return lua_error(L);
	}

	/* scan each object */
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		struct ubus_object *obj = NULL;

		/* check if the object has a table of methods */
		if ((lua_type(L, -2) == LUA_TSTRING) && (lua_type(L, -1) == LUA_TTABLE)) {
			obj = ubus_lua_load_object(L);

			if (obj){
				ubus_add_object(c->ctx, obj);

                                /* allow future reference of ubus obj */
				lua_pushstring(state,"__ubusobj");
				lua_pushlightuserdata(state, obj);
				lua_settable(state,-3);
                        }
		}
		lua_pop(L, 1);
	}

	return 0;
}

static int
ubus_lua_notify( lua_State *L )
{
	struct ubus_lua_connection *c;
	struct ubus_object *obj;
	const char* method;

	c = luaL_checkudata(L, 1, METANAME);
	method = luaL_checkstring(L, 3);
	luaL_checktype(L, 4, LUA_TTABLE);

	if( !lua_islightuserdata( L, 2 ) ){
		lua_pushfstring( L, "Invald 2nd parameter, expected ubus obj ref" );
		return lua_error( L );
	}
	obj = lua_touserdata( L, 2 );

	/* create parameters from table */
	blob_buf_init(&c->buf, 0);
	if( !ubus_lua_format_blob_array( L, &c->buf, true ) ){
		lua_pushfstring( L, "Invalid 4th parameter, expected table of arguments" );
		return lua_error( L );
	}

	ubus_notify( c->ctx, obj, method, c->buf.head, -1 );
	return 0;
}

static void
ubus_lua_signatures_cb(struct ubus_context *c, struct ubus_object_data *o, void *p)
{
	lua_State *L = (lua_State *)p;

	if (!o->signature)
		return;

	ubus_lua_parse_blob_array(L, blob_data(o->signature), blob_len(o->signature), true);
}

static int
ubus_lua_signatures(lua_State *L)
{
	int rv;
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);
	const char *path = luaL_checkstring(L, 2);

	rv = ubus_lookup(c->ctx, path, ubus_lua_signatures_cb, L);

	if (rv != UBUS_STATUS_OK)
	{
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_pushinteger(L, rv);
		return 2;
	}

	return 1;
}


static void
ubus_lua_call_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	lua_State *L = (lua_State *)req->priv;

	if (!msg && L)
		lua_pushnil(L);

	if (msg && L)
		ubus_lua_parse_blob_array(L, blob_data(msg), blob_len(msg), true);
}

static int
ubus_lua_call(lua_State *L)
{
	int rv, top;
	uint32_t id;
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);
	const char *path = luaL_checkstring(L, 2);
	const char *func = luaL_checkstring(L, 3);

	luaL_checktype(L, 4, LUA_TTABLE);
	blob_buf_init(&c->buf, 0);

	if (!ubus_lua_format_blob_array(L, &c->buf, true))
	{
		lua_pushnil(L);
		lua_pushinteger(L, UBUS_STATUS_INVALID_ARGUMENT);
		return 2;
	}

	rv = ubus_lookup_id(c->ctx, path, &id);

	if (rv)
	{
		lua_pushnil(L);
		lua_pushinteger(L, rv);
		return 2;
	}

	top = lua_gettop(L);
	rv = ubus_invoke(c->ctx, id, func, c->buf.head, ubus_lua_call_cb, L, c->timeout * 1000);

	if (rv != UBUS_STATUS_OK)
	{
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_pushinteger(L, rv);
		return 2;
	}

	return lua_gettop(L) - top;
}

static void
ubus_event_handler(struct ubus_context *ctx, struct ubus_event_handler *ev,
			const char *type, struct blob_attr *msg)
{
	struct ubus_lua_event *listener = container_of(ev, struct ubus_lua_event, e);

	lua_getglobal(state, "__ubus_cb_event");
	lua_rawgeti(state, -1, listener->r);
	lua_remove(state, -2);

	if (lua_isfunction(state, -1)) {
		ubus_lua_parse_blob_array(state, blob_data(msg), blob_len(msg), true);
		lua_call(state, 1, 0);
	} else {
		lua_pop(state, 1);
	}
}

static struct ubus_event_handler*
ubus_lua_load_event(lua_State *L)
{
	struct ubus_lua_event* event = NULL;

	event = calloc(1, sizeof(struct ubus_lua_event));
	if (!event)
		return NULL;

	event->e.cb = ubus_event_handler;

	/* update the he callback lookup table */
	lua_getglobal(L, "__ubus_cb_event");
	lua_pushvalue(L, -2);
	event->r = luaL_ref(L, -2);
	lua_setfield(L, -1, lua_tostring(L, -3));

	return &event->e;
}

static int
ubus_lua_listen(lua_State *L) {
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);

	/* verify top level object */
	luaL_checktype(L, 2, LUA_TTABLE);

	/* scan each object */
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		struct ubus_event_handler *listener;

		/* check if the key is a string and the value is a method */
		if ((lua_type(L, -2) == LUA_TSTRING) && (lua_type(L, -1) == LUA_TFUNCTION)) {
			listener = ubus_lua_load_event(L);
			if(listener != NULL) {
				ubus_register_event_handler(c->ctx, listener, lua_tostring(L, -2));
			}
		}
		lua_pop(L, 1);
	}
	return 0;
}

static void
ubus_sub_remove_handler(struct ubus_context *ctx, struct ubus_subscriber *s,
                            uint32_t id)
{
	struct ubus_lua_subscriber *sub;

	sub = container_of(s, struct ubus_lua_subscriber, s);

	lua_getglobal(state, "__ubus_cb_subscribe");
	lua_rawgeti(state, -1, sub->rremove);
	lua_remove(state, -2);

	if (lua_isfunction(state, -1)) {
		lua_call(state, 0, 0);
	} else {
		lua_pop(state, 1);
	}
}

static int
ubus_sub_notify_handler(struct ubus_context *ctx, struct ubus_object *obj,
            struct ubus_request_data *req, const char *method,
            struct blob_attr *msg)
{
	struct ubus_subscriber *s;
	struct ubus_lua_subscriber *sub;

	s = container_of(obj, struct ubus_subscriber, obj);
	sub = container_of(s, struct ubus_lua_subscriber, s);

	lua_getglobal(state, "__ubus_cb_subscribe");
	lua_rawgeti(state, -1, sub->rnotify);
	lua_remove(state, -2);

	if (lua_isfunction(state, -1)) {
		if( msg ){
			ubus_lua_parse_blob_array(state, blob_data(msg), blob_len(msg), true);
		} else {
			lua_pushnil(state);
		}
		lua_pushstring(state, method);
		lua_call(state, 2, 0);
	} else {
		lua_pop(state, 1);
	}

	return 0;
}



static int
ubus_lua_do_subscribe( struct ubus_context *ctx, lua_State *L, const char* target,
                        int idxnotify, int idxremove )
{
	uint32_t id;
	int status;
	struct ubus_lua_subscriber *sub;

	if( ( status = ubus_lookup_id( ctx, target, &id ) ) ){
		lua_pushfstring( L, "Unable find target, status=%d", status );
		return lua_error( L );
	}

	sub = calloc( 1, sizeof( struct ubus_lua_subscriber ) );
	if( !sub ){
		lua_pushstring( L, "Out of memory" );
		return lua_error( L );
	}

	if( idxnotify ){
		lua_getglobal(L, "__ubus_cb_subscribe");
		lua_pushvalue(L, idxnotify);
		sub->rnotify = luaL_ref(L, -2);
		lua_pop(L, 1);
		sub->s.cb = ubus_sub_notify_handler;
	}

	if( idxremove ){
		lua_getglobal(L, "__ubus_cb_subscribe");
		lua_pushvalue(L, idxremove);
		sub->rremove = luaL_ref(L, -2);
		lua_pop(L, 1);
		sub->s.remove_cb = ubus_sub_remove_handler;
	}

	if( ( status = ubus_register_subscriber( ctx, &sub->s ) ) ){
		lua_pushfstring( L, "Failed to register subscriber, status=%d", status );
		return lua_error( L );
	}

	if( ( status = ubus_subscribe( ctx, &sub->s, id) ) ){
		lua_pushfstring( L, "Failed to register subscriber, status=%d", status );
		return lua_error( L );
	}

	return 0;
}

static int
ubus_lua_subscribe(lua_State *L) {
	int idxnotify, idxremove, stackstart;
	struct ubus_lua_connection *c;
	const char* target;

	idxnotify = idxremove = 0;
	stackstart = lua_gettop( L );


	c = luaL_checkudata(L, 1, METANAME);
	target = luaL_checkstring(L, 2);
	luaL_checktype(L, 3, LUA_TTABLE);


	lua_pushstring( L, "notify");
	lua_gettable( L, 3 );
	if( lua_type( L, -1 ) == LUA_TFUNCTION ){
		idxnotify = lua_gettop( L );
	} else {
		lua_pop( L, 1 );
	}

	lua_pushstring( L, "remove");
	lua_gettable( L, 3 );
	if( lua_type( L, -1 ) == LUA_TFUNCTION ){
		idxremove = lua_gettop( L );
	} else {
		lua_pop( L, 1 );
	}

	if( idxnotify )
		ubus_lua_do_subscribe( c->ctx, L, target, idxnotify, idxremove );

	if( lua_gettop( L ) > stackstart )
		lua_pop( L, lua_gettop( L ) - stackstart );

	return 0;
}

static int
ubus_lua_send(lua_State *L)
{
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);
	const char *event = luaL_checkstring(L, 2);

	if (*event == 0)
		return luaL_argerror(L, 2, "no event name");

	// Event content convert to ubus form
	luaL_checktype(L, 3, LUA_TTABLE);
	blob_buf_init(&c->buf, 0);

	if (!ubus_lua_format_blob_array(L, &c->buf, true)) {
		lua_pushnil(L);
		lua_pushinteger(L, UBUS_STATUS_INVALID_ARGUMENT);
		return 2;
	}

	// Send the event
	ubus_send_event(c->ctx, event, c->buf.head);

	return 0;
}



static int
ubus_lua__gc(lua_State *L)
{
	struct ubus_lua_connection *c = luaL_checkudata(L, 1, METANAME);

	blob_buf_free(&c->buf);
	if (c->ctx != NULL)
	{
		ubus_free(c->ctx);
		memset(c, 0, sizeof(*c));
	}

	return 0;
}

static const luaL_Reg ubus[] = {
	{ "connect", ubus_lua_connect },
	{ "objects", ubus_lua_objects },
	{ "add", ubus_lua_add },
	{ "notify", ubus_lua_notify },
	{ "reply", ubus_lua_reply },
	{ "defer_request", ubus_lua_defer_request },
	{ "complete_deferred_request", ubus_lua_complete_deferred_request },
	{ "signatures", ubus_lua_signatures },
	{ "call", ubus_lua_call },
	{ "close", ubus_lua__gc },
	{ "listen", ubus_lua_listen },
	{ "send", ubus_lua_send },
	{ "subscribe", ubus_lua_subscribe },
	{ "__gc", ubus_lua__gc },
	{ NULL, NULL },
};

/* avoid missing prototype warning */
int luaopen_ubus(lua_State *L);

int
luaopen_ubus(lua_State *L)
{
	/* create metatable */
	luaL_newmetatable(L, METANAME);

	/* metatable.__index = metatable */
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	/* fill metatable */
	luaL_register(L, NULL, ubus);
	lua_pop(L, 1);

	/* create module */
	luaL_register(L, MODNAME, ubus);

	/* set some enum defines */
	lua_pushinteger(L, BLOBMSG_TYPE_ARRAY);
	lua_setfield(L, -2, "ARRAY");
	lua_pushinteger(L, BLOBMSG_TYPE_TABLE);
	lua_setfield(L, -2, "TABLE");
	lua_pushinteger(L, BLOBMSG_TYPE_STRING);
	lua_setfield(L, -2, "STRING");
	lua_pushinteger(L, BLOBMSG_TYPE_INT64);
	lua_setfield(L, -2, "INT64");
	lua_pushinteger(L, BLOBMSG_TYPE_INT32);
	lua_setfield(L, -2, "INT32");
	lua_pushinteger(L, BLOBMSG_TYPE_INT16);
	lua_setfield(L, -2, "INT16");
	lua_pushinteger(L, BLOBMSG_TYPE_INT8);
	lua_setfield(L, -2, "INT8");
	lua_pushinteger(L, BLOBMSG_TYPE_DOUBLE);
	lua_setfield(L, -2, "DOUBLE");
	lua_pushinteger(L, BLOBMSG_TYPE_BOOL);
	lua_setfield(L, -2, "BOOLEAN");

	/* used in our callbacks */
	state = L;

	/* create the callback table */
	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__ubus_cb");

	/* create the event table */
	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__ubus_cb_event");

	/* create the subscriber table */
	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__ubus_cb_subscribe");

	/* create the publisher table - notifications of new subs */
	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__ubus_cb_publisher");
	return 0;
}
