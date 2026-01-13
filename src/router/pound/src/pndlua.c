/*
 * Lua support for pound.
 * Copyright (C) 2024-2025 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "pound.h"
#include "extern.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* Lua context, associated with a worker thread (POUND_HTTP structure). */
struct pndlua
{
  lua_State *state;           /* Lua state.  Null if global context. */
  DLIST_ENTRY (pndlua) link;  /* Links to free context list. */
};

/*
 * Array of allocated contexts.  It holds from 1 to worker_max_count + 1
 * entries.  pndlua_ctx[0] is global context.  Rest of entries are contexts
 * to use with worker threads.
 */
static struct pndlua *pndlua_ctx;
/* Number of entries allocated in pndlua_ctx. */
static int pndlua_ctx_count;
/* Global mutex serializes access to pndlua_ctx[0]. */
static pthread_mutex_t pndlua_global_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Free context list. */
static DLIST_HEAD (, pndlua) pndlua_avail =
  DLIST_HEAD_INITIALIZER (pndlua_avail);
static pthread_mutex_t pndlua_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Each started worker thread obtains a context from the available list and
 * saves it in its thread-specific storage. The key pndlua_key identifies
 * the storage and pndlua_key_once serves to initialize it.
 */
static pthread_key_t pndlua_key;
static pthread_once_t pndlua_key_once = PTHREAD_ONCE_INIT;

/*
 * Reclaim the context by returning it to pndlua_avail list.
 */
static void
pndlua_reclaim (void *ptr)
{
  struct pndlua *pndlua = ptr;
  pthread_mutex_lock (&pndlua_mutex);
  DLIST_INSERT_HEAD (&pndlua_avail, pndlua, link);
  pthread_mutex_unlock (&pndlua_mutex);
}

static void
make_pndlua_key (void)
{
  pthread_key_create (&pndlua_key, pndlua_reclaim);
}

static struct pndlua *
pndlua_get (void)
{
  struct pndlua *pndlua;
  pthread_once (&pndlua_key_once, make_pndlua_key);
  if ((pndlua = pthread_getspecific (pndlua_key)) == NULL)
    {
      pthread_mutex_lock (&pndlua_mutex);
      pndlua = DLIST_FIRST (&pndlua_avail);
      DLIST_SHIFT (&pndlua_avail, link);
      pthread_mutex_unlock (&pndlua_mutex);
      pthread_setspecific (pndlua_key, pndlua);
    }
  return pndlua;
}

static inline int
pushname (lua_State *L, char const *name)
{
  char const *p;

  p = strchr (name, '.');
  if (p)
    {
      lua_pushlstring (L, name, p - name);
      name = p + 1;
      p = lua_tostring (L, -1);
      if (lua_getglobal (L, p) == LUA_TNIL)
	{
	  lua_pop (L, 2);
	  return -1;
	}
      lua_remove (L, -2);

      while (name)
	{
	  if (!lua_istable (L, -1))
	    {
	      lua_pop (L, 1);
	      return -1;
	    }
	  p = strchr (name, '.');
	  if (p)
	    {
	      lua_pushlstring (L, name, p - name);
	      name = p + 1;
	      lua_getfield (L, -2, lua_tostring (L, -1));
	    }
	  else
	    {
	      lua_getfield (L, -1, name);
	      name = NULL;
	    }
	  lua_remove (L, -2);
	}
    }
  else
    lua_getglobal (L, name);
  return 0;
}

static inline int
function_is_defined (lua_State *L, char const *name)
{
  int f = 0;

  if (pushname (L, name) == 0)
    {
      f = lua_isfunction (L, -1);
      lua_pop (L, 1);
    }
  return f;
}


static void pndlua_set_http (lua_State *L, POUND_HTTP *http, int modresp);
static void pndlua_unset_http (lua_State *L);

static lua_State *
pndlua_lock (struct pndlua_closure const *c)
{
  lua_State *L;

  if (c->ctx == PNDLUA_CTX_GLOBAL)
    {
      pthread_mutex_lock (&pndlua_global_mutex);
      L = pndlua_ctx[0].state;
    }
  else
    {
      struct pndlua *pndlua = pndlua_get ();
      L = pndlua->state;
    }
  return L;
}

static void
pndlua_unlock (struct pndlua_closure const *c)
{
  if (c->ctx == PNDLUA_CTX_GLOBAL)
    pthread_mutex_unlock (&pndlua_global_mutex);
}

#define PLC_NONE    0
#define PLC_RETBOOL 0x1
#define PLC_MODRESP 0x2

static int
pndlua_call (POUND_HTTP *phttp, struct pndlua_closure const *clos, char **argv,
	     int flags)
{
  int i;
  int res;
  lua_State *state;

  state = pndlua_lock (clos);
  pndlua_set_http (state, phttp, !!(flags & PLC_MODRESP));

  pushname (state, clos->func);
  for (i = 0; i < clos->argc; i++)
    lua_pushstring (state, argv[i]);

  res = lua_pcall (state, clos->argc, !!(flags & PLC_RETBOOL), 0);
  if (res)
    {
      conf_error_at_locus_range (&clos->locus,
				 "(%"PRItid") error calling Lua function %s: %s",
				 POUND_TID (), clos->func,
				 lua_tostring (state, -1));
      res = -1;
    }
  else if (flags & PLC_RETBOOL)
    {
      res = lua_toboolean (state, -1);
      lua_pop (state, 1);
    }

  pndlua_unset_http (state);
  pndlua_unlock (clos);

  return res;
}

int
pndlua_match (POUND_HTTP *phttp, struct pndlua_closure const *clos,
	      char **argv, void *data)
{
  return pndlua_call (phttp, clos, argv, PLC_RETBOOL);
}

int
pndlua_modify (POUND_HTTP *phttp, struct pndlua_closure const *clos,
	       char **argv, void *data)
{
  int target = *(int*)data;
  return pndlua_call (phttp, clos, argv,
		      target == REWRITE_RESPONSE ? PLC_MODRESP : 0);
}

int
pndlua_backend (POUND_HTTP *phttp, struct pndlua_closure const *clos,
		char **argv, void *data)
{
  return pndlua_call (phttp, clos, argv, PLC_MODRESP);
}


/*
 * Define auxiliary Lua functions.
 */
static void
check_args (lua_State *L, char *fname, int nargs)
{
  if (lua_gettop (L) == nargs)
    return;
  luaL_error (L, "'%s' requires %d arguments", fname, nargs);
}

/*
 * pound.log(prio, text)
 *  Log text using priority prio.
 */
static int
pndlua_pound_log (lua_State *L)
{
  char const *msg;
  int prio;

  check_args (L, "log", 2);
  prio = luaL_checkinteger (L, 1);
  msg = luaL_checkstring (L, 2);
  logmsg (prio, "%s", msg);
  return 0;
}

static int
pndlua_pound_tid (lua_State *L)
{
  lua_pushinteger (L, (lua_Integer) pthread_self ());
  return 1;
}

static char pndlua_pound_dump[] = "return function (o)\n\
   if type(o) == 'nil' or\n\
      type(o) == 'number' or\n\
      type(o) == 'boolean' then\n\
      return tostring(o), nil\n\
   elseif type(o) == 'string' then\n\
      return string.format(\"%q\", o)\n\
   elseif type(o) == 'table' then\n\
      local s = '{'\n\
      for k,v in pairs(o) do\n\
	 i, e = dump(k)\n\
	 if e ~= nil then\n\
	    return '', e\n\
	 end\n\
	 s = s .. '['.. i ..'] = ' .. dump(v) .. ','\n\
      end\n\
      return s .. '}', nil\n\
   else\n\
      return '', \"cannot serialize \"..type(o)\n\
   end\n\
end";

/*
 * Copy N stack elements starting from index S in state SRC to state DST.
 * Return 0 on success.
 * On error, return -1 and leave error message in tos of SRC.
 */
static int
pndlua_stkcopy (lua_State *dst, lua_State *src, int n, int s)
{
  int i;

  pushname (src, "pound.dump");
  if (s < 0)
    s -= 2;
  for (i = 0; i < n; i++)
    {
      char const *str;

      switch (lua_type (src, s + i))
	{
	case LUA_TNIL:
	  lua_pushnil (dst);
	  break;

	case LUA_TBOOLEAN:
	  lua_pushboolean (dst, lua_toboolean (src, s + i));
	  break;

	case LUA_TNUMBER:
	  lua_pushnumber (dst, lua_tonumber (src, s + i));
	  break;

	case LUA_TSTRING:
	  lua_pushstring (dst, lua_tostring (src, s + i));
	  break;

	case LUA_TTABLE:
	  /* Get next value from the local stack. */
	  lua_pushvalue (src, -1);
	  lua_pushvalue (src, s + i);
	  if (lua_pcall (src, 1, 2, 0) != LUA_OK)
	    {
	      lua_pushfstring (src, "error converting argument %d: %s",
			       i, lua_tostring (src, -1));
	      lua_remove (src, -2);
	      lua_remove (src, -2);
	      if (i)
		lua_pop (dst, i);
	      return -1;
	    }

	  if (lua_type (src, -1) != LUA_TNIL)
	    {
	      lua_pushfstring (src, "error converting argument %d: %s",
			       i, lua_tostring (src, -1));
	      lua_remove (src, -2);
	      lua_remove (src, -2);
	      if (i)
		lua_pop (dst, i);
	      return -1;
	    }

	  lua_pop (src, 1);

	  lua_pushstring (src, "return ");
	  lua_pushvalue (src, -2);
	  lua_concat (src, 2);
	  str = lua_tostring (src, -1);

	  /* Load it to the global stack. */
	  if (luaL_loadstring (dst, str) != LUA_OK)
	    {
	      lua_pushfstring (src, "error passing argument %d: %s",
			       i, lua_tostring (dst, -1));
	      lua_remove (src, -2);
	      lua_remove (src, -2);
	      lua_pop (dst, i + 1);
	      return -1;
	    }
	  if (lua_pcall (dst, 0, 1, 0) != LUA_OK)
	    {
	      lua_pushfstring (src, "error passing argument %d: %s",
			       i, lua_tostring (dst, -1));
	      lua_remove (src, -2);
	      lua_remove (src, -2);
	      lua_pop (dst, i + 1);
	      return -1;
	    }

	  lua_pop (src, 2);
	  break;

	default:
	  lua_pushfstring (src,
			   "error passing argument %d: unsupported data type %s",
			   i, lua_typename (src, lua_type (src, s + i)));
	  lua_pop (dst, i + 1);
	  return -1;
	}
    }
  lua_pop (src, 1);
  return LUA_OK;
}

static int
pndlua_pound_gcall (lua_State *L)
{
  int nargs = lua_gettop (L);
  char const *fname = lua_tostring (L, 1);
  lua_State *GL;
  int nret;
  int res = LUA_OK, r;

  if (L == pndlua_ctx[0].state)
    {
      /* Already in global state. */
      int i;
      nret = lua_gettop (L);
      if (lua_getglobal (L, fname) != LUA_TFUNCTION)
	luaL_error (L, "%s: not a function", fname);
      for (i = 1; i <= nargs; i++)
	lua_pushvalue (L, i);
      lua_call (L, nargs, LUA_MULTRET);
      return lua_gettop (L) - nret;
    }

  pthread_mutex_lock (&pndlua_global_mutex);

  GL = pndlua_ctx[0].state;
  nret = lua_gettop (GL);
  if (lua_getglobal (GL, fname) != LUA_TFUNCTION)
    {
      lua_pop (GL, 1);
      lua_pushfstring (L, "%s: not a function", fname);
      res = LUA_ERRRUN;
      goto err;
    }

  --nargs;

  /* Pass arguments. */
  res = pndlua_stkcopy (GL, L, nargs, 2);
  if (res != LUA_OK)
    {
      lua_pop (GL, 1);
      goto err;
    }

  /* Call the function. */
  res = lua_pcall (GL, nargs, LUA_MULTRET, 0);
  if (res != LUA_OK)
    {
      if (pndlua_stkcopy (L, GL, 1, -1) != LUA_OK)
	lua_pop (GL, 1);
      goto err;
    }

  nret = lua_gettop (GL) - nret;
  if ((r = pndlua_stkcopy (L, GL, nret, - nret)) != LUA_OK)
    res = r;
  lua_pop (GL, nret);

 err:
  pthread_mutex_unlock (&pndlua_global_mutex);
  if (res != LUA_OK)
    luaL_error (L, "%s", lua_tostring (L, -1));

  return nret;
}


/*
 * Additional functions for defining table elements.
 */

/* In a table at -1, define a method "name". */
static void
pndlua_dcl_function (lua_State *L, char const *name, lua_CFunction func)
{
  lua_pushstring (L, name);
  lua_pushcfunction (L, func);
  lua_rawset (L, -3);
}

static void
pndlua_dcl_function_chunk (lua_State *L, char const *name, char const *chunk)
{
  lua_pushstring (L, name);
  luaL_loadstring (L, chunk);
  lua_call (L, 0, 1);
  lua_rawset (L, -3);
}

/* In a table at -1, define integer variable "name" with the given value. */
static void
pndlua_dcl_integer (lua_State *L, char const *name, int value)
{
  lua_pushstring (L, name);
  lua_pushinteger (L, value);
  lua_rawset (L, -3);
}

/* Create a metatable and store it in the registry under the given name. */
static void
pndlua_new_metatable (lua_State *L, char const *name)
{
  lua_newtable (L);
  /* Leave the value on stack upon exit. */
  lua_pushvalue (L, -1);

  /* Create __name field. */
  lua_pushstring (L, name);
  lua_setfield (L, -2, "__name");  /* metatable.__name = tname */

  /* Register the table. */
  lua_setfield (L, LUA_REGISTRYINDEX, name);
}

/*
 * Lookup a name in a poor man's hash table.  The table consists of an
 * array of luaL_Reg reg and index string letidx.  Entries in reg are
 * ordered by name.  Elements in letidx are related to those in reg as
 * follows:
 *
 *                letidx[i] == reg[i].name[0]
 *
 * Thus, number of elements in reg equals strlen(letidx).
 *
 * On success, the function returns a pointer to the function stored under
 * the given name.  On failure, it returns NULL.
 */
static lua_CFunction
pndlua_reg_locate (char const *letidx, luaL_Reg *reg, char const *name)
{
  int i;
  char *p;
  size_t reg_size = strlen (letidx);

  if ((p = strchr (letidx, name[0])) != NULL)
    for (i = p - letidx; i < reg_size && reg[i].name[0] == name[0]; i++)
      if (strcmp (name, reg[i].name) == 0)
	return reg[i].func;
  return NULL;
}

/*
 * Obtain userdata from an object at stack index idx.  The object must
 * be a table such that the userdata is stored in obj[0].  Raise an error
 * if this is not the case or if the obtained pointer is NULL.
 */
static void *
pndlua_get_userdata (lua_State *L, int idx)
{
  void *p;

  if (!lua_istable (L, idx))
    luaL_argerror (L, idx, NULL);
  lua_rawgeti (L, idx, 0);
  p = lua_touserdata (L, -1);
  if (!p)
    luaL_argerror (L, idx, NULL);
  lua_pop (L, 1);
  return p;
}

static int
ro_newindex (lua_State *L)
{
  luaL_error (L, "attempt to modify read-only data");
  return 0;
}

static int
pndlua_memerr (lua_State *L)
{
  return luaL_error (L, "out of memory");
}

/* HTTP accessors.
 *
 * http.req   - returns HTTP request
 * http.resp  - returns HTTP response
 *
 * req.line    - full request line
 * req.method  - request method (string)
 * req.headers - headers (table)
 * req.version - HTTP version (table)
 *   tostring(req.version) - string
 *   req.version.major     - major number
 *   req.version.minor     - minor number
 * req.url
 * req.path
 * req.query   - query (table)
 *   tostring(req.query)   - full query as a string.
 *   req.query[k]          - value of query parameter k.
 */

struct req_ud
{
  struct http_request *req;
};

static int
req_line (lua_State *L)
{
  struct req_ud *ud  = pndlua_get_userdata (L, 1);
  lua_pushstring (L, ud->req->request);
  return 1;
}

static int
req_method (lua_State *L)
{
  struct req_ud *ud  = pndlua_get_userdata (L, 1);
  lua_pushstring (L, method_name (ud->req->method));
  return 1;
}

static int
req_headers_str (lua_State *L)
{
  struct req_ud *ud  = pndlua_get_userdata (L, 1);
  struct http_header *hdr;
  luaL_Buffer b;

  luaL_buffinit (L, &b);
  DLIST_FOREACH (hdr, &ud->req->headers, link)
    {
      luaL_addstring (&b, hdr->header);
      luaL_addstring (&b, "\n");
    }
  luaL_pushresult (&b);

  return 1;
}

static int
req_headers_index (lua_State *L)
{
  struct req_ud *ud  = pndlua_get_userdata (L, 1);
  char const *field = lua_tostring (L, 2);
  char const *val;
  struct http_header *hdr;

  hdr = http_header_list_locate_name (&ud->req->headers, field,
				      strlen (field));
  if (hdr == NULL)
    lua_pushnil (L);
  else
    {
      if ((val = http_header_get_value (hdr)) == NULL)
	pndlua_memerr (L);
      lua_pushstring (L, val);
      if ((hdr = http_header_list_next (hdr)) != NULL)
	{
	  int n = 1;

	  /* Return multiple values as a table. */
	  lua_newtable (L);
	  lua_rotate (L, -2, 1);
	  lua_rawseti (L, -2, 0);

	  do
	    {
	      if ((val = http_header_get_value (hdr)) == NULL)
		pndlua_memerr (L);
	      lua_pushstring (L, val);
	      lua_rawseti (L, -2, n);
	      n++;
	    }
	  while ((hdr = http_header_list_next (hdr)) != NULL);
	}
    }
  return 1;
}

/*
 * Get string value from index IDX and append it to the headers list, using
 * the indicated mode.  Bail out on error.
 */
static void
pndlua_header_list_append (lua_State *L, HTTP_HEADER_LIST *head, int idx, int mode)
{
  switch (http_header_list_append (head, lua_tostring (L, idx), mode))
    {
    case -1:
      pndlua_memerr (L);
      break;
    case 1:
      luaL_error (L, "internal error");
    }
}

/*
 * Append header or headers to the header list.  Field name is at stack
 * index 2, and new value is at index 3.  The value can be one of:
 *
 *   nil        - all headers with that field name are removed from the
 *                list;
 *   string     - "field: value" is appended to the list;
 *   table      - all headers with that field name are removed from the
 *                list, and the values from the table are added instead.
 */
static int
headers_set (lua_State *L, HTTP_HEADER_LIST *head)
{
  char const *field = lua_tostring (L, 2);

  if (lua_isnil (L, 3))
    {
      http_header_list_remove_field (head, field);
    }
  else if (lua_istable (L, 3))
    {
      http_header_list_remove_field (head, field);

      lua_pushnil (L);  /* first key */
      while (lua_next (L, 3) != 0)
	{
	  lua_pushstring (L, field);
	  lua_pushstring (L, ": ");
	  lua_rotate (L, -3, -1);
	  lua_concat (L, 3);
	  pndlua_header_list_append (L, head, -1, H_APPEND);
	  lua_pop (L, 1);
	}
    }
  else if (lua_isstring (L, 3))
    {
      lua_pushvalue (L, 2);
      lua_pushstring (L, ": ");
      lua_pushvalue (L, 3);
      lua_concat (L, 3);
      pndlua_header_list_append (L, head, -1, H_REPLACE);
      lua_pop (L, 1);
    }

  return 0;
}

static int
req_headers_newindex (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  return headers_set (L, &ud->req->headers);
}

static int
req_headers_len (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  struct http_header *hdr;
  int n = 0;
  DLIST_FOREACH (hdr, &ud->req->headers, link)
    n++;
  lua_pushinteger (L, n);
  return 1;
}

/*
 * Create and push on stack a new http.req (or http.resp) object.
 *
 * Argument n gives the stack index at which the userdata with struct
 * req_ud is located.
 *
 */
static int
req_new_headers (lua_State *L, int n)
{
  /* Create the object */
  lua_newtable (L);
  /* Account for new element on stack. */
  if (n < 0)
    n--;
  /* t[0] = ud */
  lua_rawgeti (L, n, 0);
  lua_rawseti (L, -2, 0);

  /* Prepare metatable */
  lua_newtable (L);

  lua_pushcfunction (L, req_headers_str);
  lua_setfield (L, -2, "__tostring");

  lua_pushcfunction (L, req_headers_index);
  lua_setfield (L, -2, "__index");

  lua_pushcfunction (L, req_headers_newindex);
  lua_setfield (L, -2, "__newindex");

  lua_pushcfunction (L, req_headers_len);
  lua_setfield (L, -2, "__len");

  /* Set metatable. */
  lua_setmetatable (L, -2);

  return 1;
}

static int
req_headers (lua_State *L)
{
  return req_new_headers (L, 1);
}

static int
req_version_str (lua_State *L)
{
  lua_pushvalue (L, lua_upvalueindex (1));
  return 1;
}

/* Implementation of http.req.version table. */
static int
req_version (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  char *p;

  /* Create the object */
  lua_newtable (L);

  /* Prepare metatable */
  lua_newtable (L);
  /* Create __name field. */
  p = strrchr (ud->req->request, '/');
  if (!p)
    luaL_error (L, "malformed request");
  p++;
  lua_pushstring (L, p);
  lua_pushvalue (L, -1);
  lua_setfield (L, -3, "__name");  /* metatable.__name = tname */

  lua_pushcclosure (L, req_version_str, 1);
  lua_setfield (L, -2, "__tostring");

  /* Set metatable. */
  lua_setmetatable (L, -2);

  lua_pushinteger (L, 1);
  lua_setfield (L, -2, "major");
  lua_pushinteger (L, ud->req->version);
  lua_setfield (L, -2, "minor");

  return 1;
}

static int
req_url (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  char const *v;
  http_request_get_url (ud->req, &v);
  lua_pushstring (L, v);
  return 1;
}

static int
req_path (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  char const *v;
  if (http_request_get_path (ud->req, &v))
    pndlua_memerr (L);
  lua_pushstring (L, v);
  return 1;
}

static int
req_query_str (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  char const *v;
  if (http_request_get_query (ud->req, &v))
    pndlua_memerr (L);
  lua_pushstring (L, v ? v : "");
  return 1;
}

static int
req_query_len (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  lua_pushinteger (L, http_request_count_query_param (ud->req));
  return 1;
}

static int
req_query_index (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  char const *field = lua_tostring (L, 2);
  char const *val;
  switch (http_request_get_query_param_value (ud->req, field, &val))
    {
    case RETRIEVE_ERROR:
      pndlua_memerr (L);
      break;
    case RETRIEVE_NOT_FOUND:
      lua_pushnil (L);
      break;
    case RETRIEVE_OK:
      lua_pushstring (L, val);
    }
  return 1;
}

static int
req_query_newindex (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  char const *field = lua_tostring (L, 2);
  if (lua_isnil (L, 3))
    {
      http_request_set_query_param (ud->req, field, NULL);
    }
  else if (lua_istable (L, 3))
    {
      luaL_argerror (L, 3, NULL);
    }
  else
    {
      http_request_set_query_param (ud->req, field, luaL_tolstring (L, 3, NULL));
      lua_pop (L, 1);
    }
  return 0;
}

static int
req_query (lua_State *L)
{
  /* Create the object */
  lua_newtable (L);
  /* t[0] = ud */
  lua_rawgeti (L, 1, 0);
  lua_rawseti (L, -2, 0);

  /* Prepare metatable */
  lua_newtable (L);

  lua_pushcfunction (L, req_query_str);
  lua_setfield (L, -2, "__tostring");

  lua_pushcfunction (L, req_query_index);
  lua_setfield (L, -2, "__index");

  lua_pushcfunction (L, req_query_newindex);
  lua_setfield (L, -2, "__newindex");

  lua_pushcfunction (L, req_query_len);
  lua_setfield (L, -2, "__len");

  /* Set metatable. */
  lua_setmetatable (L, -2);

  return 1;
}

static int
pndlua_req_index (lua_State *L)
{
  char const *field;
  lua_CFunction fun;

  static char letidx[] = "hlmpquv";
  static struct luaL_Reg reg[] = {
    { "headers", req_headers },
    { "line", req_line },
    { "method", req_method },
    { "path", req_path },
    { "query", req_query },
    { "url", req_url },
    { "version", req_version },
  };

  field = lua_tostring (L, 2);
  if ((fun = pndlua_reg_locate (letidx, reg, field)) == NULL)
    luaL_error (L, "%s: no such field", field);
  return fun (L);
}

static int
set_headers (lua_State *L, HTTP_HEADER_LIST *head)
{
  http_header_list_free (head);
  if (lua_istable (L, 3))
    {
      lua_pushnil (L);
      while (lua_next (L, 3) != 0)
	{
	  lua_pushvalue (L, -2);
	  lua_pushstring (L, ": ");
	  lua_rotate (L, -3, -1);
	  lua_concat (L, 3);
	  pndlua_header_list_append (L, head, -1, H_APPEND);
	  lua_pop (L, 1);
	}
    }
  else if (!lua_isnil (L, 3))
    luaL_argerror (L, 3, NULL);
  return 0;
}

static int
req_set_headers (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  return set_headers (L, &ud->req->headers);
}

static int
req_set_path (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  if (http_request_set_path (ud->req, lua_tostring (L, 3)))
    pndlua_memerr (L);
  return 0;
}

static int
req_set_url (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  if (http_request_set_url (ud->req, lua_tostring (L, 3)))
    pndlua_memerr (L);
  return 0;
}

static int
req_set_query (lua_State *L)
{
  struct req_ud *ud = pndlua_get_userdata (L, 1);
  if (http_request_set_query (ud->req, lua_tostring (L, 3)))
    pndlua_memerr (L);
  return 0;
}

static int
pndlua_req_newindex (lua_State *L)
{
  char const *field;
  lua_CFunction fun;

  static char letidx[] = "hlmpquv";
  static struct luaL_Reg reg[] = {
    { "headers", req_set_headers },
    { "line", ro_newindex },
    { "method", ro_newindex },
    { "path", req_set_path },
    { "query", req_set_query },
    { "url", req_set_url },
    { "version", ro_newindex },
  };

  field = lua_tostring (L, 2);
  if ((fun = pndlua_reg_locate (letidx, reg, field)) == NULL)
    luaL_error (L, "%s: no such field", field);
  return fun (L);
}

static char const pndlua_req_class[] = "req";

static void
pndlua_dcl_req (lua_State *L)
{
  pndlua_new_metatable (L, pndlua_req_class);
  /* Prepare the __index entry. */
  pndlua_dcl_function (L, "__index", pndlua_req_index);
  pndlua_dcl_function (L, "__newindex", pndlua_req_newindex);
  lua_pop (L, 1);
}

/*
 * Implementation of the http global.
 */
struct http_ud
{
  POUND_HTTP *phttp;
  int modresp;
};

static int
resp_get_body (lua_State *L)
{
  struct http_ud *ud = pndlua_get_userdata (L, 1);
  struct stringbuf *body = ud->phttp->response.body;
  if (body)
    lua_pushlstring (L, stringbuf_value (body), stringbuf_len (body));
  else
    lua_pushlstring (L, "", 0);
  return 1;
}

static int
resp_get_code (lua_State *L)
{
  struct http_ud *ud = pndlua_get_userdata (L, 1);
  lua_pushinteger (L, ud->phttp->response_code);
  return 1;
}

static int
resp_get_headers (lua_State *L)
{
  struct http_ud *http = pndlua_get_userdata (L, 1);
  struct req_ud *rud;
  int rc;

  /* Create request userdata. */
  lua_newtable (L);
  rud = lua_newuserdata (L, sizeof (*rud));
  rud->req = &http->phttp->response;
  lua_rawseti (L, -2, 0);
  /* Create the headers table. */
  rc = req_new_headers (L, -1);
  /* Get rid of the original userdata. */
  lua_remove (L, -2);
  return rc;
}

static int
resp_get_reason (lua_State *L)
{
  struct http_ud *ud = pndlua_get_userdata (L, 1);
  lua_pushstring (L, ud->phttp->response.request);
  return 1;
}

static int
pndlua_resp_index (lua_State *L)
{
  char const *field;
  lua_CFunction fun;

  static char letidx[] = "bchr";
  static struct luaL_Reg reg[] = {
    { "body",  resp_get_body },
    { "code",  resp_get_code },
    { "headers", resp_get_headers },
    { "reason", resp_get_reason }
  };

  field = lua_tostring (L, 2);
  if ((fun = pndlua_reg_locate (letidx, reg, field)) == NULL)
    luaL_error (L, "%s: no such field", field);
  return fun (L);
}

static int
resp_set_body (lua_State *L)
{
  struct http_ud *ud = pndlua_get_userdata (L, 1);
  struct stringbuf *body = ud->phttp->response.body;

  if (!body)
    {
      if ((body = malloc (sizeof (*body))) == NULL)
	return pndlua_memerr (L);
      stringbuf_init_log (body);
      ud->phttp->response.body = body;
    }

  stringbuf_reset (body);
  if (!lua_isnil (L, 3))
    {
      char const *val = lua_tostring (L, 3);
      if (stringbuf_add_string (body, val))
	luaL_error (L, "out if memory");
    }
  return 0;
}

static int
resp_set_code (lua_State *L)
{
  struct http_ud *ud = pndlua_get_userdata (L, 1);
  int code = lua_tointeger (L, 3);
  if (code >= 100 && code < 600)
    {
      ud->phttp->response_code = code;
    }
  else
    luaL_argerror (L, 3, "argument out of allowed range");
  return 0;
}

static int
resp_set_headers (lua_State *L)
{
  struct http_ud *ud = pndlua_get_userdata (L, 1);
  return set_headers (L, &ud->phttp->response.headers);
}

static int
resp_set_reason (lua_State *L)
{
  struct http_ud *ud = pndlua_get_userdata (L, 1);

  free (ud->phttp->response.request);
  if (lua_isnil (L, 3))
    {
      ud->phttp->response.request = NULL;
    }
  else
    {
      ud->phttp->response.request = strdup (lua_tostring (L, 3));
      if (!ud->phttp->response.request)
	return pndlua_memerr (L);
    }
  return 0;
}

static int
pndlua_resp_newindex (lua_State *L)
{
  char const *field;
  lua_CFunction fun;

  static char letidx[] = "bchr";
  static struct luaL_Reg reg[] = {
    { "body",  resp_set_body },
    { "code",  resp_set_code },
    { "headers", resp_set_headers },
    { "reason", resp_set_reason }
  };

  field = lua_tostring (L, 2);
  if ((fun = pndlua_reg_locate (letidx, reg, field)) == NULL)
    luaL_error (L, "%s: no such field", field);
  return fun (L);
}

static char const pndlua_resp_class[] = "resp";

static void
pndlua_dcl_resp (lua_State *L)
{
  pndlua_new_metatable (L, pndlua_resp_class);
  pndlua_dcl_function (L, "__index", pndlua_resp_index);
  pndlua_dcl_function (L, "__newindex", pndlua_resp_newindex);
  lua_pop (L, 1);
}

static int
http_req (lua_State *L)
{
  struct http_ud *http = pndlua_get_userdata (L, 1);
  struct req_ud *rud;

  lua_newtable (L);
  rud = lua_newuserdata (L, sizeof (*rud));
  lua_rawseti (L, -2, 0);

  rud->req = &http->phttp->request;

  lua_getfield (L, LUA_REGISTRYINDEX, pndlua_req_class);
  lua_setmetatable (L, -2);

  return 1;
}

static int
http_resp (lua_State *L)
{
  /* Create the object */
  lua_newtable (L);
  /* t[0] = ud */
  lua_rawgeti (L, 1, 0);
  lua_rawseti (L, -2, 0);

  lua_getfield (L, LUA_REGISTRYINDEX, pndlua_resp_class);
  lua_setmetatable (L, -2);

  return 1;
}

static int
pndlua_http_index (lua_State *L)
{
  struct http_ud *http = pndlua_get_userdata (L, 1);
  char const *field;
  lua_CFunction fun;

  int rw = http->modresp != 0;

  static char *letidx[] = { "r", "rr" };
  static struct luaL_Reg reg[] = {
    { "req", http_req },
    { "resp", http_resp }
  };

  field = lua_tostring (L, 2);
  if ((fun = pndlua_reg_locate (letidx[rw], reg, field)) == NULL)
    luaL_error (L, "%s: no such field", field);
  return fun (L);
}

/*
 * Prepare and set the "http" global in the Lua state.  If modresp is not 0,
 * the http.resp element will be available and assignments to http.resp.body
 * will be allowed.
 */
static void
pndlua_set_http (lua_State *L, POUND_HTTP *phttp, int modresp)
{
  struct http_ud *ud;

  /* Create table with userdata at t[0] */
  lua_newtable (L);
  ud = lua_newuserdata (L, sizeof (*ud));
  lua_rawseti (L, -2, 0);
  ud->phttp = phttp;
  ud->modresp = modresp;

  pndlua_new_metatable (L, "http");
  /* Create __index entry. */
  pndlua_dcl_function (L, "__index", pndlua_http_index);
  /* Create __newindex entry. */
  pndlua_dcl_function (L, "__newindex", ro_newindex);

  /* Set metatable. */
  lua_setmetatable (L, -2);

  lua_setglobal (L, "http");
}

/*
 * Unset the "http" global.
 */
static void
pndlua_unset_http (lua_State *L)
{
  lua_pushnil (L);
  lua_setglobal (L, "http");
}

static int
pndlua_stash_init (lua_State *L)
{
    int ctx = L == pndlua_ctx[0].state ? PNDLUA_CTX_GLOBAL : PNDLUA_CTX_THREAD;
    struct http_ud *ud;
    int result;

    lua_getglobal (L, "http");
    ud = pndlua_get_userdata (L, -1);
    lua_pop (L, 1);
    result = ud->phttp->stash_init[ctx];
    ud->phttp->stash_init[ctx] = 1;
    return result;
}

/*
 * Stash implementation.
 *
 * Global variable stash can be used to share data between various
 * LuaMatch and LuaBackend invocations, run in the same thread while
 * processing one HTTP request.
 *
 * When accessed for the first time, the variable is initialized to
 * an empty table.  Any values stored in this table will remain there
 * until processing of the HTTP request is finished.
 */
static char stash_name[] = "stash";

static void
stash_assert (lua_State *L)
{
  if (!pndlua_stash_init (L))
    {
      lua_pushinteger (L, (lua_Integer) pthread_self ());
      lua_newtable (L);
      lua_rawset (L, 1);   /* stash = stash[thread_id] */
    }
  lua_pushinteger (L, (lua_Integer) pthread_self ());
  lua_rawget (L, 1);
}

static int
stash_index (lua_State *L)
{
  stash_assert (L);
  lua_pushvalue (L, 2);  /* key */
  lua_rawget (L, -2);    /* stash[key] */
  lua_remove (L, -2);
  return 1;
}

static int
stash_newindex (lua_State *L)
{
  stash_assert (L);

  lua_pushvalue (L, 2); /* key */
  lua_pushvalue (L, 3); /* value */
  lua_rawset (L, -3);   /* stash[key] = value */

  lua_pop (L, 1);
  return 0;
}

static void
pndlua_mkstash (lua_State *L)
{
  /* Create the table. */
  lua_newtable (L);

  /* Prepare metatable */
  lua_newtable (L);

  lua_pushcfunction (L, stash_index);
  lua_setfield (L, -2, "__index");

  lua_pushcfunction (L, stash_newindex);
  lua_setfield (L, -2, "__newindex");

  /* Set metatable. */
  lua_setmetatable (L, -2);

  lua_setglobal (L, stash_name);
}

/*
 * Lua path manipulation.
 *
 * Two configuration statements are provided for the purpose: "Path" modifies
 * package.path and "CPath" modifies package.cpath. In both cases, the
 * argument gets prepended to the corresponding Lua variable.
 */

/* This structure represents a single path element. */
struct path_dir
{
  SLIST_ENTRY (path_dir) next; /* Link to the next element. */
  char name[1];                /* Element name (allocated after the struct). */
};
typedef SLIST_HEAD(,path_dir) PATH_HEAD;

enum
  {
    PNDLUA_PATH,
    PNDLUA_CPATH
  };

/*
 * Arguments to each path configuration statement are split on ';' and
 * the resulting elements are appended to the corresponding path_head
 * list.  The elements are concatenated again right before prepending
 * them to the corresponding 'package.*' variable. This allows for multiple
 * "Path" ("CPath") statement and ensures their relative ordering is preserved
 * in the resulting path.
 *
 * Both lists are freed after Lua subsystem is initialized.
 */
static char *path_name[] = { "path", "cpath" };
static PATH_HEAD path_head[2] = {
  SLIST_HEAD_INITIALIZER (path_head[0]),
  SLIST_HEAD_INITIALIZER (path_head[1])
};

/* Adds a single name to the path list. */
static void
path_list_add (PATH_HEAD *head, char const *name)
{
  struct path_dir *dir = xmalloc (sizeof (*dir) + strlen (name));
  strcpy (dir->name, name);
  SLIST_INSERT_TAIL (head, dir, next);
}

static void
path_list_free (PATH_HEAD *head)
{
  while (!SLIST_EMPTY (head))
    {
      struct path_dir *dir = SLIST_FIRST (head);
      SLIST_REMOVE_HEAD (head, next);
      free (dir);
    }
}

/*
 * Re-assemble path list path_head[type] and prepend it to the right
 * 'package.*' global in the Lua state.
 */
static void
pndlua_add_path (lua_State *L, int type)
{
  struct path_dir *dir;
  int n;

  if (SLIST_EMPTY (&path_head[type]))
    return;

  lua_getglobal (L, "package");
  n = 1;
  SLIST_FOREACH (dir, &path_head[type], next)
    {
      lua_pushstring (L, dir->name);
      lua_pushstring (L, ";");
      n += 2;
    }

  /* Concatenate. */
  lua_getfield (L, -n, path_name[type]);
  lua_concat (L, n);

  /* Store new path and clean up stack. */
  lua_setfield (L, -2, path_name[type]);
  lua_pop (L, 1);
}

static struct kwtab severity_table[] = {
  { "EMERG",   LOG_EMERG },
  { "ALERT",   LOG_ALERT },
  { "CRIT",    LOG_CRIT },
  { "ERR",     LOG_ERR },
  { "WARNING", LOG_WARNING },
  { "NOTICE",  LOG_NOTICE },
  { "INFO",    LOG_INFO },
  { "DEBUG",   LOG_DEBUG },
  { NULL }
};

/*
 * Create and initialize new pound Lua state.
 */
static lua_State *
pndlua_new_state (void)
{
  int i;
  lua_State *state;

  state = luaL_newstate ();
  luaL_openlibs (state);

  lua_newtable (state);

  for (i = 0; severity_table[i].name; i++)
    pndlua_dcl_integer (state, severity_table[i].name, severity_table[i].tok);

  pndlua_dcl_function (state, "log", pndlua_pound_log);
  pndlua_dcl_function (state, "tid", pndlua_pound_tid);
  pndlua_dcl_function_chunk (state, "dump", pndlua_pound_dump);
  pndlua_dcl_function (state, "gcall", pndlua_pound_gcall);
  lua_setglobal (state, "pound");

  pndlua_mkstash (state);

  for (i = 0; i < sizeof (path_head) / sizeof (path_head[0]); i++)
    pndlua_add_path (state, i);

  pndlua_dcl_req (state);
  pndlua_dcl_resp (state);

  return state;
}

/*
 * This structure identifies a single Lua source along with the location
 * of the place in configuration file that requires it to be loaded.
 */
struct pndlua_source
{
  char *filename;                   /* Full file name. */
  char *modname;                    /* Module name. */
  struct locus_range locus;         /* Config location. */
  SLIST_ENTRY (pndlua_source) next; /* Link to the next source. */
};

/*
 * Lua sources are stored in two single-linked lists:
 *  global_sources - for sources required using the "LoadGlobal" directive,
 *  thread_sources - for those required with the "Load" directive.
 */
static SLIST_HEAD (pndlua_source_head,pndlua_source)
  global_sources = SLIST_HEAD_INITIALIZER (global_sources),
  thread_sources = SLIST_HEAD_INITIALIZER (thread_sources);

static int
source_load (lua_State *L, struct pndlua_source *source, int nresults)
{
  int res;

  if (luaL_loadfile (L, source->filename) != LUA_OK)
    {
      conf_error_at_locus_range (&source->locus,
				 "error loading Lua file %s: %s",
				 source->filename,
				 lua_tostring (L, -1));
      lua_pop (L, 1);
      return -1;
    }

  res = lua_pcall (L, 0, nresults, 0);
  switch (res)
    {
    case LUA_OK:
      break;

    case LUA_ERRRUN:
      conf_error_at_locus_range (&source->locus,
				 "Lua runtime error: %s",
				 lua_tostring (L, -1));
      lua_pop (L, 1);
      return -1;

    case LUA_ERRMEM:
      conf_error_at_locus_range (&source->locus,
				 "out of memory running Lua code in %s",
				 source->filename);
      return -1;

    case LUA_ERRERR:
      conf_error_at_locus_range (&source->locus,
				 "Lua message handler error in %s: %s",
				 source->filename, lua_tostring (L, -1));
      lua_pop (L, 1);
      return -1;

#ifdef LUA_ERRGCMM
    case LUA_ERRGCMM:
      conf_error_at_locus_range (&source->locus,
				 "Lua garbage collector error in %s: %s",
				 source->filename, lua_tostring (L, -1));
      lua_pop (L, 1);
      return -1;
#endif

    default:
      conf_error_at_locus_range (&source->locus, "unhandled Lua error %d",
				 res);
      return -1;
    }

  return 0;
}

static inline int
c_is_lua_var_beg (int c)
{
  return c_isalpha (c) || c == '_';
}

static inline int
c_is_lua_var (int c)
{
  return c_isalnum (c) || c == '_';
}

static inline int
is_lua_var_name (char const *s)
{
  if (!c_is_lua_var_beg (*s))
    return 0;
  while (*++s)
    if (!c_is_lua_var (*s))
      return 0;
  return 1;
}

static int
source_load_module (lua_State *L, struct pndlua_source *source)
{
  if (source->modname == NULL)
    {
      static char const suf[] = ".lua";
      static size_t suflen = sizeof (suf) - 1;
      WORKDIR *wd;
      char const *basename;
      size_t baselen, i;

      if ((basename = filename_split_wd (source->filename, &wd)) == NULL)
	{
	  conf_error_at_locus_range (&source->locus,
				     "can't split filename: %s",
				     strerror (errno));
	  return -1;
	}
      baselen = strlen (basename);
      if (baselen > suflen && strcmp (basename + baselen - suflen, suf) == 0)
	baselen -= suflen;
      if (c_is_lua_var_beg (basename[0]))
	source->modname = xstrndup (basename, baselen);
      else
	{
	  source->modname = xmalloc (baselen + 2);
	  source->modname[0] = '_';
	  memcpy (source->modname, basename, baselen);
	  baselen++;
	  source->modname[baselen] = 0;
	}
      for (i = 0; i < baselen; i++)
	if (!c_is_lua_var (source->modname[i]))
	  source->modname[i] = '_';
    }

  luaL_getsubtable (L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield (L, -1, source->modname);  /* LOADED[modname] */
  if (!lua_toboolean (L, -1))
    {
      lua_pop (L, 1);
      if (source_load (L, source, 1))
	return -1;
      if (!lua_istable (L, -1))
	{
	  lua_pop (L, 2);
	  return 0;
	}
      lua_pushvalue (L, -1);  /* make copy of module (call result) */
      lua_setfield (L, -2, source->modname);  /* LOADED[modname] = module */
    }
  lua_remove(L, -2);  /* remove LOADED table */
  lua_setglobal (L, source->modname);  /* _G[modname] = module */
  return 0;
}

/*
 * Load each Lua source into the Lua state of the ith context. When
 * loading, initialize the pound.loadctx global with the value i.
 *
 * Bail out at the first error.  Return 0 on success, -1 on error (an
 * error message has already been issued in that case).
 */
static int
source_list_load (int i, struct pndlua_source_head *head)
{
  lua_State *L = pndlua_ctx[i].state;
  struct pndlua_source *source;
  int rc = 0;

  lua_getglobal (L, "pound");
  lua_pushstring (L, "loadctx");
  lua_pushinteger (L, i);
  lua_rawset (L, -3);

  SLIST_FOREACH (source, head, next)
    {
      if ((rc = source_load_module (L, source)) != 0)
	break;
    }

  lua_pushstring (L, "loadctx");
  lua_pushnil (L);
  lua_rawset (L, -3);

  lua_pop (L, 1);

  return rc;
}

static void
source_list_free (struct pndlua_source_head *head)
{
  while (!SLIST_EMPTY (head))
    {
      struct pndlua_source *source = SLIST_FIRST (head);
      SLIST_REMOVE_HEAD (head, next);
      locus_range_unref (&source->locus);
      free (source->filename);
      free (source->modname);
      free (source);
    }
}

/*
 * Lua closure list maintenance.
 *
 * Each LuaMatch and LuaBackend statement results in creation of a new struct
 * pndlua_closure, and saving a pointer to in the closure list closure_head.
 *
 * When all Lua sources have been loaded, the list is traversed in order to
 * determine the context for each its element: thread, if the function func
 * is found in thread-specific Lua state, and global, if it is found in the
 * global state. If the function is not found in either one, an error is
 * reported.
 *
 * The list is freed when the Lua subsystem have been initialized.
 *
 * List elements have the following structure:
 */
struct closure_entry
{
  struct pndlua_closure *closure;
  SLIST_ENTRY (closure_entry) next;
};

/* Closure list. */
static SLIST_HEAD (,closure_entry) closure_head =
  SLIST_HEAD_INITIALIZER (closure_head);

/* Add new closure to the list. */
static void
closure_head_add (struct pndlua_closure *closure)
{
    struct closure_entry *ent;
    XZALLOC (ent);
    ent->closure = closure;
    SLIST_INSERT_TAIL (&closure_head, ent, next);
}

/* Free the closure list. */
static void
closure_head_free (void)
{
  while (!SLIST_EMPTY (&closure_head))
    {
      struct closure_entry *ent = SLIST_FIRST (&closure_head);
      SLIST_REMOVE_HEAD (&closure_head, next);
      free (ent);
    }
}

/*
 * Resolve a single closure: i.e. determine whether its function is
 * in thread-specific or in the global state. Return 0 if it has been
 * resolved, otherwise report an error and return -1.
 */
static int
closure_resolve (struct pndlua_closure *closure)
{
  if (pndlua_ctx_count > 1 &&
      function_is_defined (pndlua_ctx[1].state, closure->func))
    closure->ctx = PNDLUA_CTX_THREAD;
  else if (function_is_defined (pndlua_ctx[0].state, closure->func))
    closure->ctx = PNDLUA_CTX_GLOBAL;
  else
    {
      conf_error_at_locus_range (&closure->locus,
				 "Lua function %s not defined",
				 closure->func);
      return -1;
    }
  return 0;
}

/*
 * Resolve each element in the list.
 */
static int
closure_head_resolve (void)
{
  struct closure_entry *ent;
  int err = 0;
  SLIST_FOREACH (ent, &closure_head, next)
    if (closure_resolve (ent->closure))
      ++err;
  return err;
}

/*
 * Initialize Lua subsystem.
 */
int
pndlua_init (void)
{
  int i;

  /*
   * Count the contexts. Global context is always available. Per-thread
   * contexts are created only if one or more "Load" statements were given.
   */
  pndlua_ctx_count = 1;
  if (!SLIST_EMPTY (&thread_sources))
    pndlua_ctx_count += worker_max_count;

  /*
   * Initialize contexts and store them in pndlua_avail list.
   */
  pndlua_ctx = xcalloc (pndlua_ctx_count, sizeof (*pndlua_ctx));
  pndlua_ctx[0].state = pndlua_new_state ();
  for (i = 1; i < pndlua_ctx_count; i++)
    {
      pndlua_ctx[i].state = pndlua_new_state ();
      DLIST_INSERT_TAIL (&pndlua_avail, &pndlua_ctx[i], link);
    }

  /* Load global sources. */
  if (source_list_load (0, &global_sources))
    return -1;

  /* Load per-thread sources. */
  for (i = 1; i < pndlua_ctx_count; i++)
    if (source_list_load (i, &thread_sources))
      return -1;

  /* Resolve invocation contexts. */
  if (closure_head_resolve ())
    return -1;

  /* Free unneeded memory. */
  source_list_free (&global_sources);
  source_list_free (&thread_sources);
  path_list_free (&path_head[PNDLUA_PATH]);
  path_list_free (&path_head[PNDLUA_CPATH]);
  closure_head_free ();

  return 0;
}

/*
 * Configuration parser.
 */

static int
parse_lua_path (int n)
{
  char *path, *s, *p;
  int rc = cfg_assign_string (&path, NULL);
  if (rc != CFGPARSER_OK)
    return rc;
  for (s = strtok_r (path, ";", &p); s; s = strtok_r (NULL, ";", &p))
    {
      if (!strchr (s, '?'))
	{
	  conf_error ("%s: this doesn't look like a Lua path component", s);
	  rc = CFGPARSER_FAIL;
	}
      else
	path_list_add (&path_head[n], s);
    }
  return rc;
}

static int
pndlua_parse_lua_path (void *call_data, void *section_data)
{
  return parse_lua_path (PNDLUA_PATH);
}

static int
pndlua_parse_lua_cpath (void *call_data, void *section_data)
{
  return parse_lua_path (PNDLUA_CPATH);
}

static int
parse_lua_load (struct pndlua_source_head *head)
{
  struct token *tok;
  char *filename;
  struct pndlua_source *src;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  if ((filename = filename_resolve (tok->str)) == NULL)
    return CFGPARSER_FAIL;

  XZALLOC (src);
  src->filename = filename;
  locus_range_init (&src->locus);
  locus_range_copy (&src->locus, &tok->locus);
  SLIST_INSERT_TAIL (head, src, next);

  if ((tok = gettkn_any ()) == NULL)
    return CFGPARSER_FAIL;

  if (tok->type == T_STRING)
    {
      if (!is_lua_var_name (tok->str))
	{
	  conf_error ("%s", "not a valid Lua variable name");
	  return CFGPARSER_FAIL;
	}

      src->modname = xstrdup (tok->str);
    }
  else if (tok->type != '\n')
    return CFGPARSER_FAIL;

  return CFGPARSER_OK_NONL;
}

static int
pndlua_parse_lua_load (void *call_data, void *section_data)
{
  return parse_lua_load (&thread_sources);
}

static int
pndlua_parse_lua_load_global (void *call_data, void *section_data)
{
  return parse_lua_load (&global_sources);
}

int
pndlua_parse_closure (struct pndlua_closure *cls)
{
  struct token *tok;
  size_t argmax = 0;

  if ((tok = gettkn_expect (T_STRING)) == NULL)
    return CFGPARSER_FAIL;

  locus_range_init (&cls->locus);
  locus_range_copy (&cls->locus, &tok->locus);

  cls->func = xstrdup (tok->str);
  while (1)
    {
      if ((tok = gettkn_any ()) == NULL)
	return CFGPARSER_FAIL;
      if (tok->type == T_STRING)
	{
	  if (cls->argc == argmax)
	    {
	      cls->argv = x2nrealloc (cls->argv, &argmax,
				       sizeof cls->argv[0]);
	    }
	  cls->argv[cls->argc++] = xstrdup (tok->str);
	}
      else if (tok->type == '\n')
	break;
      else
	{
	  conf_error ("expected string or newline, but found %s",
		      token_type_str (tok->type));
	  return CFGPARSER_FAIL;
	}
    }
  closure_head_add (cls);
  return CFGPARSER_OK_NONL;
}

static CFGPARSER_TABLE lua_parsetab[] = {
  {
    .name = "End",
    .parser = cfg_parse_end
  },
  {
    .name = "Path",
    .parser = pndlua_parse_lua_path
  },
  {
    .name = "CPath",
    .parser = pndlua_parse_lua_cpath
  },
  {
    .name = "Load",
    .parser = pndlua_parse_lua_load
  },
  {
    .name = "LoadGlobal",
    .parser = pndlua_parse_lua_load_global
  },
  { NULL }
};

int
pndlua_parse_config (void *call_data, void *section_data)
{
  return parser_loop (lua_parsetab, NULL, NULL, NULL);
}
