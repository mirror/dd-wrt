/* Golang-style template engine for pound
 * Copyright (C) 2023-2024 Sergey Poznyakoff
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

/*
 * This file implements a template engine complying to the specifications
 * in https://pkg.go.dev/text/template (as of version 1.19.4).
 *
 * All values, including dot are struct json_value *.
 * Variables are implemented only in the "range" action and are global.
 * Only the '=' assignment operator is supported.
 */

#include "pound.h"
#include <assert.h>
#include "json.h"

static void
write_string (void *data, char const *str, size_t len)
{
  FILE *fp = data;
  fwrite (str, len, 1, fp);
}

static void
print_json (struct json_value *val, FILE *fp)
{
  struct json_format format = {
    .indent = 0,
    .precision = 0,
    .write = write_string,
    .data = fp
  };
  json_value_format (val, &format, 0);
}

void
json_error (struct json_value *val, char const *fmt, ...)
{
  va_list ap;

  fprintf (stderr, "%s: ", progname);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fprintf (stderr, " in: ");
  print_json (val, stderr);
  fputc ('\n', stderr);
}

void
errormsg (int ex, int ec, char const *fmt, ...)
{
  va_list ap;

  fprintf (stderr, "%s: ", progname);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  if (ec)
    fprintf (stderr, ": %s", strerror (ec));
  fputc ('\n', stderr);
  if (ex)
    exit (ex);
}

struct tmpl_env
{
  struct json_value *dot;
  int dotfree;
  struct json_value *vars;
  FILE *outfile;
};

struct tmpl_env *
tmpl_env_new (struct json_value *dot, int dotfree, struct tmpl_env *up)
{
  struct tmpl_env *env;

  XZALLOC (env);
  env->dot = dot;
  env->dotfree = dotfree;
  if (up)
    {
      json_value_copy (up->vars, &env->vars);
      env->outfile = up->outfile;
    }
  else
    {
      env->vars = json_new_object ();
      env->outfile = stdout;
    }
  if (!env->vars)
    xnomem ();
  return env;
}

static void
tmpl_env_free (struct tmpl_env *env)
{
  fflush (env->outfile);
  json_value_free (env->vars);
  if (env->dotfree)
    json_value_free (env->dot);
  free (env);
}

typedef SLIST_HEAD (,tmpl_command) TMPL_PIPELINE;

struct tmpl_formal_arg
{
  TMPL_PIPELINE *pipeline;
  SLIST_ENTRY (tmpl_formal_arg) next;
};
typedef SLIST_HEAD (,tmpl_formal_arg) FORMAL_ARG_HEAD;

struct tmpl_actual_arg
{
  struct json_value *val;
  SLIST_ENTRY (tmpl_actual_arg) next;
};
typedef SLIST_HEAD (,tmpl_actual_arg) ACTUAL_ARG_HEAD;

typedef struct json_value *(*TMPL_FUNC) (ACTUAL_ARG_HEAD const *);

struct tmpl_funcall
{
  TMPL_FUNC func;
  FORMAL_ARG_HEAD args;
};

enum tmpl_command_type
  {
    TMPL_COMMAND_ARG,
    TMPL_COMMAND_FUNCALL,
    TMPL_COMMAND_DOT,
    TMPL_COMMAND_ATTR,
    TMPL_COMMAND_VAR,
    TMPL_COMMAND_PIPELINE,
    TMPL_COMMAND_WITHDOT,
  };

struct tmpl_command
{
  SLIST_ENTRY (tmpl_command) next;
  enum tmpl_command_type type;
  union
  {
    struct json_value *arg;
    struct tmpl_funcall call;
    char *varname;
    TMPL_PIPELINE pipeline;
    struct
    {
      TMPL_PIPELINE dot;
      TMPL_PIPELINE body;
    } withdot;
  } v;
};

static void
actual_arg_head_free (ACTUAL_ARG_HEAD *arghead)
{
  while (!SLIST_EMPTY (arghead))
    {
      struct tmpl_actual_arg *arg = SLIST_FIRST (arghead);
      SLIST_SHIFT (arghead, next);
      json_value_free (arg->val);
      free (arg);
    }
}

static void tmpl_pipeline_free (TMPL_PIPELINE *head);

static void
formal_arg_head_free (FORMAL_ARG_HEAD *arghead)
{
  while (!SLIST_EMPTY (arghead))
    {
      struct tmpl_formal_arg *arg = SLIST_FIRST (arghead);
      SLIST_SHIFT (arghead, next);
      tmpl_pipeline_free (arg->pipeline);
      free (arg->pipeline);
    }
}

static void
tmpl_command_free (struct tmpl_command *com)
{
  switch (com->type)
    {
    case TMPL_COMMAND_ARG:
      json_value_free (com->v.arg);
      break;

    case TMPL_COMMAND_FUNCALL:
      formal_arg_head_free (&com->v.call.args);
      break;

    case TMPL_COMMAND_DOT:
      break;

    case TMPL_COMMAND_ATTR:
    case TMPL_COMMAND_VAR:
      free (com->v.varname);
      break;

    case TMPL_COMMAND_PIPELINE:
      tmpl_pipeline_free (&com->v.pipeline);
      break;

    case TMPL_COMMAND_WITHDOT:
      tmpl_pipeline_free (&com->v.withdot.dot);
      tmpl_pipeline_free (&com->v.withdot.body);
      break;
    }
  free (com);
}

static void
tmpl_pipeline_free (TMPL_PIPELINE *head)
{
  while (!SLIST_EMPTY (head))
    {
      struct tmpl_command *com = SLIST_FIRST (head);
      SLIST_SHIFT (head, next);
      tmpl_command_free (com);
    }
}

struct json_value *
tmpl_pipeline_eval (struct tmpl_env *env, TMPL_PIPELINE *head)
{
  struct tmpl_command *com;
  struct json_value *arg, *res, *tmp;

  res = json_new_null ();
  SLIST_FOREACH (com, head, next)
    {
      arg = res;
      res = NULL;
      switch (com->type)
	{
	case TMPL_COMMAND_ARG:
	  json_value_copy (com->v.arg, &res);
	  break;

	case TMPL_COMMAND_FUNCALL:
	  {
	    struct tmpl_formal_arg *farg;
	    struct tmpl_actual_arg *aarg;
	    ACTUAL_ARG_HEAD arghead;

	    SLIST_INIT (&arghead);
	    SLIST_FOREACH (farg, &com->v.call.args, next)
	      {
		XZALLOC (aarg);
		aarg->val = tmpl_pipeline_eval (env, farg->pipeline);
		SLIST_PUSH (&arghead, aarg, next);
	      }
	    if (com != SLIST_FIRST (head))
	      {
		XZALLOC (aarg);
		json_value_copy (arg, &aarg->val);
		SLIST_PUSH (&arghead, aarg, next);
	      }
	    res = com->v.call.func (&arghead);
	    actual_arg_head_free (&arghead);
	  }
	  break;

	case TMPL_COMMAND_DOT:
	  json_value_copy (env->dot, &res);
	  break;

	case TMPL_COMMAND_ATTR:
	  if (json_object_get (env->dot, com->v.varname, &tmp) == 0)
	    json_value_copy (tmp, &res);
	  else
	    res = json_new_null ();
	  break;

	case TMPL_COMMAND_VAR:
	  if (json_object_get (env->vars, com->v.varname, &tmp))
	    res = json_new_null ();
	  else
	    json_value_copy (tmp, &res);
	  break;

	case TMPL_COMMAND_PIPELINE:
	  {
	    struct tmpl_env *sub_env;

	    sub_env = tmpl_env_new (env->dot, 0, env);
	    res = tmpl_pipeline_eval (sub_env, &com->v.pipeline);
	    tmpl_env_free (sub_env);
	  }
	  break;

	case TMPL_COMMAND_WITHDOT:
	  {
	    struct tmpl_env *sub_env;
	    struct json_value *jv;

	    sub_env = tmpl_env_new (env->dot, 0, env);
	    jv = tmpl_pipeline_eval (sub_env, &com->v.withdot.dot);
	    tmpl_env_free (sub_env);

	    sub_env = tmpl_env_new (jv, 1, env);
	    res = tmpl_pipeline_eval (sub_env, &com->v.withdot.body);
	    tmpl_env_free (sub_env);
	  }
	  break;

	default:
	  abort ();
	}
      json_value_free (arg);
    }
  return res;
}

/*
 * Functions
 */

static int
is_empty (struct json_value *val)
{
  switch (val->type)
    {
    case json_null:
      return 1;
    case json_bool:
      return val->v.b == 0;
    case json_number:
      return -1e-3 <= val->v.n && val->v.n <= 1e-3;
    case json_integer:
      return (long) val->v.n == 0;
    case json_string:
      return strlen (val->v.s) == 0;
    case json_array:
      return val->v.a->oc == 0;
    case json_object:
      return val->v.o->pair_count == 0;
    }
  return 0;
}

static inline int
is_true (struct json_value *val)
{
  return !is_empty (val);
}

static void
assert_args (ACTUAL_ARG_HEAD const *head, char const *funcname)
{
  if (SLIST_EMPTY (head))
    errormsg (1, 0, "not enough arguments for %s", funcname);
}

static struct tmpl_actual_arg *
single_arg (ACTUAL_ARG_HEAD const *head, char const *funcname)
{
  struct tmpl_actual_arg *arg;

  if (SLIST_EMPTY (head))
    errormsg (1, 0, "not enough arguments for %s", funcname);

  arg = SLIST_FIRST (head);
  if (SLIST_NEXT (arg, next) != NULL)
    errormsg (1, 0, "too many arguments for %s", funcname);

  return arg;
}

static void
two_args (ACTUAL_ARG_HEAD const *head, char const *name,
	  struct json_value **pa, struct json_value **pb)
{
  struct tmpl_actual_arg *arg;
  if (SLIST_EMPTY (head) || (arg = SLIST_NEXT (SLIST_FIRST (head), next)) == NULL)
    errormsg (1, 0, "not enough arguments for %s", name);

  if (SLIST_NEXT (arg, next))
    errormsg (1, 0, "too many arguments for %s", name);

  *pa = SLIST_FIRST (head)->val;
  *pb = SLIST_NEXT (SLIST_FIRST (head), next)->val;
}

static struct json_value *
func_and (ACTUAL_ARG_HEAD const *head)
{
  struct tmpl_actual_arg *arg, *res;
  struct json_value *jv;
  SLIST_FOREACH (arg, head, next)
    {
      res = arg;
      if (!is_true (res->val))
	break;
    }
  json_value_copy (res->val, &jv);
  return jv;
}

static struct json_value *
func_or (ACTUAL_ARG_HEAD const *head)
{
  struct tmpl_actual_arg *arg, *res;
  struct json_value *jv;
  SLIST_FOREACH (arg, head, next)
    {
      res = arg;
      if (is_true (res->val))
	break;
    }
  json_value_copy (res->val, &jv);
  return jv;
}

struct json_value *
json_cast_to_integer (struct json_value *val)
{
  double n;
  switch (val->type)
    {
    case json_null:
      n = 0;
      break;

    case json_bool:
      n = val->v.b;
      break;

    case json_number:
    case json_integer:
      n = val->v.n;
      break;

    case json_string:
      {
	char *p;
	errno = 0;
	n = strtol (val->v.s, &p, 10);
	if (errno || *p)
	  n = 0;
      }
      break;

    case json_array:
      n = val->v.a->oc;
      break;

    case json_object:
      n = val->v.o->pair_count;
    }
  return json_new_integer (n);
}

struct json_value *
json_cast_to_string (struct json_value *val)
{
  struct json_value *result;
  struct stringbuf sb;
  char *s;

  xstringbuf_init (&sb);
  switch (val->type)
    {
    case json_null:
      s = "";
      break;

    case json_bool:
      s = val->v.b ? "true" : "";
      break;

    case json_number:
      stringbuf_printf (&sb, "%g", val->v.n);
      s = stringbuf_finish (&sb);
      break;

    case json_integer:
      stringbuf_printf (&sb, "%ld", (long)val->v.n);
      s = stringbuf_finish (&sb);
      break;

    case json_string:
      s = val->v.s;
      break;

    case json_array:
      stringbuf_printf (&sb, "%lu", (unsigned long)val->v.n);
      s = stringbuf_finish (&sb);
      break;

    case json_object:
      stringbuf_printf (&sb, "%lu", (unsigned long)val->v.o->pair_count);
      s = stringbuf_finish (&sb);
      break;
    }
  result = json_new_string (s);
  stringbuf_free (&sb);
  return result;
}

static struct json_value *
func_index (ACTUAL_ARG_HEAD const *head)
{
  struct tmpl_actual_arg *arg;
  struct json_value *v, *tmp;

  arg = SLIST_FIRST (head);
  v = arg->val;

  while ((arg = SLIST_NEXT (arg, next)) != NULL)
    {
      switch (v->type)
	{
	case json_array:
	  tmp = json_cast_to_integer (arg->val);
	  json_array_get (v, (size_t) tmp->v.n, &v);
	  json_value_free (tmp);
	  break;

	case json_object:
	  tmp = json_cast_to_string (arg->val);
	  if (json_object_get (v, tmp->v.s, &v))
	    {
	      json_error (v, "no such attribute: %s", tmp->v.s);
	      exit (1);
	    }
	  json_value_free (tmp);
	  break;

	default:
	  json_error (SLIST_FIRST (head)->val, "can't be indexed");
	  exit (1);
	}
    }
  json_value_copy (v, &tmp);
  return tmp;
}

static struct json_value *
func_len (ACTUAL_ARG_HEAD const *head)
{
  struct json_value *val, *tmp;
  size_t len;

  val = single_arg (head, "len")->val;
  switch (val->type)
    {
    case json_null:
      len = 0;
      break;

    case json_bool:
    case json_number:
    case json_integer:
      tmp = json_cast_to_string (val);
      len = strlen (tmp->v.s);
      json_value_free (tmp);
      break;

    case json_string:
      len = strlen (val->v.s);
      break;

    case json_array:
      len = val->v.a->oc;
      break;

    case json_object:
      len = val->v.o->pair_count;
      break;
    }

  return json_new_integer (len);
}

static struct json_value *
func_not (ACTUAL_ARG_HEAD const *head)
{
  return json_new_bool (!is_true (single_arg (head, "not")->val));
}

static int
json_cmp_eq (struct json_value *a, struct json_value *b)
{
  return 0;
}

static int
json_number_cmp (struct json_value *a, struct json_value *b)
{
  if (a->v.n < b->v.n)
    return -1;
  if (a->v.n > b->v.n)
    return 1;
  return 0;
}

static int
json_integer_cmp (struct json_value *a, struct json_value *b)
{
  long ai = (long) a->v.n;
  long bi = (long) b->v.n;
  if (ai < bi)
    return -1;
  if (ai > bi)
    return 1;
  return 0;
}

static int
json_string_cmp (struct json_value *a, struct json_value *b)
{
  return strcmp (a->v.s, b->v.s);
}

#define NTYPES (json_object+1)
static int (*json_cmp_tab[NTYPES][NTYPES]) (struct json_value *, struct json_value *) = {
  [json_null] = {
    [json_null] = json_cmp_eq,
  },
  [json_bool] = {
    [json_bool] = json_cmp_eq,
  },
  [json_number] = {
    [json_number] = json_number_cmp,
    [json_integer] = json_integer_cmp
  },
  [json_integer] = {
    [json_number] = json_integer_cmp,
    [json_integer] = json_integer_cmp
  },
  [json_string] = {
    [json_string] = json_string_cmp
  }
};

static char const *json_type_str[] = {
  "null",
  "bool",
  "number",
  "integer",
  "string",
  "array",
  "object"
};

static int
json_cmp (ACTUAL_ARG_HEAD const *head, char const *funcname)
{
  struct json_value *a, *b;
  int (*cmp) (struct json_value *, struct json_value *);

  two_args (head, funcname, &a, &b);

  cmp = json_cmp_tab[a->type][b->type];

  if (!cmp)
    errormsg (1, 0, "%s and %s are not comparable",
	      json_type_str[a->type],
	      json_type_str[b->type]);

  return cmp (a, b);
}

static struct json_value *
func_eq (ACTUAL_ARG_HEAD const *head)
{
  return json_new_bool (json_cmp (head, "eq") == 0);
}

static struct json_value *
func_ne (ACTUAL_ARG_HEAD const *head)
{
  return json_new_bool (json_cmp (head, "ne") != 0);
}

static struct json_value *
func_lt (ACTUAL_ARG_HEAD const *head)
{
  return json_new_bool (json_cmp (head, "lt") < 0);
}

static struct json_value *
func_le (ACTUAL_ARG_HEAD const *head)
{
  return json_new_bool (json_cmp (head, "le") <= 0);
}

static struct json_value *
func_gt (ACTUAL_ARG_HEAD const *head)
{
  return json_new_bool (json_cmp (head, "gt") > 0);
}

static struct json_value *
func_ge (ACTUAL_ARG_HEAD const *head)
{
  return json_new_bool (json_cmp (head, "ge") >= 0);
}

static struct json_value *
func_even (ACTUAL_ARG_HEAD const *head)
{
  struct tmpl_actual_arg *arg = single_arg (head, "even");
  struct json_value *tmp = json_cast_to_integer (arg->val);
  struct json_value *result = json_new_bool (((long)tmp->v.n) % 2 == 0);
  json_value_free (tmp);
  return result;
}

static struct json_value *
func_typeof (ACTUAL_ARG_HEAD const *head)
{
  struct tmpl_actual_arg *arg = single_arg (head, "typeof");
  static char const *json_typestr[] = {
    [json_null] = "null",
    [json_bool] = "bool",
    [json_number] = "number",
    [json_integer] = "integer",
    [json_string] = "string",
    [json_array] = "array",
    [json_object] = "object"
  };
  assert (arg->val->type >= 0 && arg->val->type < sizeof (json_typestr) / sizeof (json_typestr[0]));
  return json_new_string (json_typestr[arg->val->type]);
}

static struct json_value *
func_exists (ACTUAL_ARG_HEAD const *head)
{
  struct json_value *a, *b, *jv;
  int rc;
  two_args (head, "exists", &a, &b);
  if (a->type != json_object)
    {
      json_error (a, "argument 1 to exists has wrong type");
      rc = 0;
    }
  else if (b->type != json_string)
    {
      json_error (a, "argument 2 to exists has wrong type");
      rc = 0;
    }
  else
    rc = json_object_get (a, b->v.s, &jv) == 0;
  return json_new_bool (rc);
}

static void
assert_numeric_value (struct json_value *val, char const *func, int n)
{
  if (val->type != json_number && val->type != json_integer)
    errormsg (1, 0, "bad type of argument %d to %s", n, func);
}

static struct json_value *
func_add (ACTUAL_ARG_HEAD const *head)
{
  double n;
  int i;
  struct tmpl_actual_arg *arg;

  assert_args (head, "add");
  arg = SLIST_FIRST (head);
  assert_numeric_value (arg->val, "add", 1);
  n = arg->val->v.n;
  i = 2;
  while ((arg = SLIST_NEXT (arg, next)) != NULL)
    {
      assert_numeric_value (arg->val, "add", i++);
      n += arg->val->v.n;
    }
  return json_new_number (n);
}

static struct json_value *
func_sub (ACTUAL_ARG_HEAD const *head)
{
  double n;
  int i;
  struct tmpl_actual_arg *arg;

  assert_args (head, "sub");
  arg = SLIST_FIRST (head);
  assert_numeric_value (arg->val, "sub", 1);
  n = arg->val->v.n;
  i = 2;
  while ((arg = SLIST_NEXT (arg, next)) != NULL)
    {
      assert_numeric_value (arg->val, "sub", i++);
      n -= arg->val->v.n;
    }
  return json_new_number (n);
}

static struct json_value *
func_mul (ACTUAL_ARG_HEAD const *head)
{
  double n;
  int i;
  struct tmpl_actual_arg *arg;

  assert_args (head, "mul");
  arg = SLIST_FIRST (head);
  assert_numeric_value (arg->val, "mul", 1);
  n = arg->val->v.n;
  i = 2;
  while ((arg = SLIST_NEXT (arg, next)) != NULL)
    {
      assert_numeric_value (arg->val, "mul", i++);
      n *= arg->val->v.n;
    }
  return json_new_number (n);
}

static struct json_value *
func_div (ACTUAL_ARG_HEAD const *head)
{
  struct json_value *a, *b;

  two_args (head, "div", &a, &b);
  assert_numeric_value (a, "div", 1);
  assert_numeric_value (b, "div", 2);
  return json_new_number (a->v.n / b->v.n);
}

/*
 * Implementation of the printf function, written along the lines of
 * sprintf built-in function in mailfromd
 * (https://git.gnu.org.ua/mailfromd.git/tree/src/builtin/sprintf.bi)
 */
#define FMT_ALTPOS         0x01
#define FMT_ALTERNATE      0x02
#define FMT_PADZERO        0x04
#define FMT_ADJUST_LEFT    0x08
#define FMT_SPACEPFX       0x10
#define FMT_SIGNPFX        0x20

typedef enum
  {
    fmts_copy,      /* Copy char as is */
    fmts_pos,       /* Expect argument position -- %_2$ */
    fmts_flags,     /* Expect flags -- %2$_# */
    fmts_width,     /* Expect width -- %2$#_8 or %2$#_* */
    fmts_width_arg, /* Expect width argument position -- %2$#*_1$ */
    fmts_prec,      /* Expect precision */
    fmts_prec_arg,  /* Expect precision argument position */
    fmts_conv       /* Expect conversion specifier */
  }
  printf_format_state;

static int
get_num (const char *p, int i, unsigned *pn)
{
  unsigned n = 0;

  for (; p[i] && isdigit (p[i]); i++)
    n = n * 10 + p[i] - '0';
  *pn = n;
  return i;
}

static void
tmpl_va_arg_n (struct tmpl_actual_arg *arg, unsigned n, int type, void *retval)
{
  assert (n > 0);
  while (--n)
    {
      if ((arg = SLIST_NEXT (arg, next)) == NULL)
	errormsg (1, 0, "not enough arguments in call to %s: argument %d requested by format string",
		  "printf", n);

    }
  switch (type)
    {
    case json_integer:
      if (arg->val->type == type || arg->val->type == json_number)
	{
	  *(long*)retval = arg->val->v.n;
	  return;
	}
      break;

    case json_number:
      if (arg->val->type == type || arg->val->type == json_integer)
	{
	  *(double*)retval = arg->val->v.n;
	  return;
	}
      break;

    case json_string:
      {
	struct json_value *jv = json_cast_to_string (arg->val);
	*(char**)retval = xstrdup (jv->v.s);
	json_value_free (jv);
	return;
      }
      break;

    case json_object:
      *(struct json_value**)retval = arg->val;
      return;

    default:
      errormsg (0, 0, "%s:%d: INTERNAL ERROR: incorrect type supplied; please, report", __FILE__, __LINE__);
      abort ();
    }

  errormsg (1, 0, "argument type mismatch");
}

static void
sb_write_string (void *data, char const *str, size_t len)
{
  struct stringbuf *sb = data;
  stringbuf_add (sb, str, len);
}

void
stringbuf_print_json (struct stringbuf *sb, struct json_value *val)
{
  struct json_format format = {
    .indent = 0,
    .precision = 0,
    .write = sb_write_string,
    .data = sb
  };
  json_value_format (val, &format, 0);
}

static struct json_value *
func_printf (ACTUAL_ARG_HEAD const *head)
{
  struct tmpl_actual_arg *argstart;
  int i = 1;
  int cur = 0;
  int start;
  printf_format_state state = fmts_copy;
  int flags = 0;
  unsigned width = 0;
  unsigned prec = 0;
  int has_prec = 0;
  unsigned argnum;
  char const *format;
  struct stringbuf sb, tmpbuf;
  struct json_value *jv;

  if (SLIST_EMPTY (head))
    errormsg (1, 0, "not enough arguments for %s", "printf");

  argstart = SLIST_FIRST (head);

  if (argstart->val->type != json_string)
    errormsg (1, 0, "printf format is not string");

  format = argstart->val->v.s;

  xstringbuf_init (&sb);
  xstringbuf_init (&tmpbuf);

  argstart = SLIST_NEXT (argstart, next);

  while (format[cur])
    {
      unsigned n;
      char *str;
      long num;
      double d;
      int negative;
      char fmtbuf[] = { '%', 'x', 0 };

      switch (state)
	{
	case fmts_copy:
	  /* Expect `%', and copy all the rest verbatim */
	  if (format[cur] == '%')
	    {
	      start = cur;
	      state = fmts_pos;
	      flags = 0;
	      width = 0;
	      prec = 0;
	      has_prec = 0;
	    }
	  else
	    stringbuf_add_char (&sb, format[cur]);
	  cur++;
	  break;

	case fmts_pos:
	  /* Expect '%' or an argument position -- %_% or %_2$ */
	  if (format[cur] == '%')
	    {
	      stringbuf_add_char (&sb, '%');
	      cur++;
	      state = fmts_copy;
	      break;
	    }
	  if (isdigit (format[cur]))
	    {
	      int pos = get_num (format, cur, &n);
	      if (format[pos] == '$')
		{
		  argnum = n - 1;
		  flags |= FMT_ALTPOS;
		  cur = pos + 1;
		}
	    }
	  state = fmts_flags;
	  break;

	case fmts_flags:
	  /* Expect flags -- %2$_# */
	  switch (format[cur])
	    {
	    case '#':
	      flags |= FMT_ALTERNATE;
	      cur++;
	      break;

	    case '0':
	      flags |= FMT_PADZERO;
	      cur++;
	      break;

	    case '-':
	      flags |= FMT_ADJUST_LEFT;
	      cur++;
	      break;

	    case ' ':
	      flags |= FMT_SPACEPFX;
	      cur++;
	      break;

	    case '+':
	      flags |= FMT_SIGNPFX;
	      cur++;
	      break;

	    default:
	      state = fmts_width;
	    }
	  break;

	case fmts_width:
	  /* Expect width -- %2$#_8 or %2$#_* */
	  if (isdigit (format[cur]))
	    {
	      cur = get_num (format, cur, &width);
	      state = fmts_prec;
	    }
	  else if (format[cur] == '*')
	    {
	      cur++;
	      state = fmts_width_arg;
	    }
	  else
	    state = fmts_prec;
	  break;

	case fmts_width_arg:
	  /* Expect width argument position -- %2$#*_1$ */
	  state = fmts_prec;
	  if (isdigit(format[cur]))
	    {
	      int pos = get_num (format, cur, &n);
	      if (format[pos] == '$')
		{
		  tmpl_va_arg_n (argstart, n, json_integer, &num);
		  cur = pos + 1;
		  if (num < 0)
		    {
		      flags |= FMT_SPACEPFX;
		      num = - num;
		    }
		  width = (unsigned) num;
		  break;
		}
	    }

	  tmpl_va_arg_n (argstart, i, json_integer, &num);
	  i++;
	  if (num < 0)
	    {
	      /*
	       * A negative field width is taken
	       * as a `-' flag followed by a positive field width.
	       */
	      flags |= FMT_SPACEPFX;
	      num = - num;
	    }
	  width = (unsigned) num;
	  break;

	case fmts_prec:
	  /* Expect precision -- %2$#*1$_. */
	  state = fmts_conv;
	  if (format[cur] == '.')
	    {
	      cur++;
	      if (isdigit (format[cur]))
		{
		  cur = get_num (format, cur, &prec);
		}
	      else if (format[cur] == '*')
		{
		  cur++;
		  state = fmts_prec_arg;
		}
	      has_prec = 1;
	    }
	  break;

	case fmts_prec_arg:
	  /* Expect precision argument position -- %2$#*1$.*_3$ */
	  state = fmts_conv;
	  if (isdigit (format[cur]))
	    {
	      int pos = get_num (format, cur, &n);
	      if (format[pos] == '$')
		{
		  tmpl_va_arg_n (argstart, n, json_integer, &num);
		  if (num > 0)
		    prec = (unsigned) num;
		  cur = pos + 1;
		  break;
		}
	    }
	  tmpl_va_arg_n (argstart, i, json_integer, &num);
	  if (num > 0)
	    prec = (unsigned) num;
	  break;

	case fmts_conv:       /* Expect conversion specifier */
	  if (!(flags & FMT_ALTPOS))
	    argnum = i++;
	  switch (format[cur])
	    {
	    case 's':
	      tmpl_va_arg_n (argstart, argnum, json_string, &str);
	      n = strlen (str);
	      if (prec && prec < n)
		n = prec;
	      if (width)
		{
		  char *q, *s;
		  if (n > width)
		    width = n;
		  q = s = xmalloc (width + 1);
		  q[width] = 0;
		  memset (q, ' ', width);
		  if (!(flags & FMT_ADJUST_LEFT) && n < width)
		    {
		      s = q + width - n;
		    }
		  memcpy (s, str, n);
		  free (str);
		  str = q;
		  n = width;
		}
	      stringbuf_add (&sb, str, n);
	      free (str);
	      break;

	    case 'i':
	    case 'd':
	      tmpl_va_arg_n (argstart, argnum, json_integer, &num);
	      if (num < 0)
		{
		  negative = 1;
		  num = - num;
		}
	      else
		negative = 0;
	      /*
	       * If a precision is given with a numeric conversion, the 0
	       * flag is ignored.
	       *
	       * A - overrides a 0 if both are given.
	       */
	      if (prec || (flags & FMT_ADJUST_LEFT))
		flags &= ~FMT_PADZERO;
	      /* A + overrides a ' ' if both are given. */
	      if (flags & FMT_SIGNPFX)
		flags &= ~FMT_SPACEPFX;

	      /* Reset temp buffer */
	      stringbuf_reset (&tmpbuf);
	      /* Alloc an extra slot for sign */
	      stringbuf_set (&tmpbuf, ' ', 1);
	      /* Format the value */
	      stringbuf_printf (&tmpbuf, "%ld", num);
	      /* Point str to the start of the formatted value */
	      str = stringbuf_finish (&tmpbuf) + 1;
	      n = strlen (str);
	      if (prec && prec > n)
		{
		  memmove (str + prec - n, str, n + 1);
		  memset (str, '0', prec - n);
		}

	      /* Provide sign if requested */
	      if (flags & FMT_SIGNPFX)
		{
		  *--str = negative ? '-' : '+';
		}
	      else if (flags & FMT_SPACEPFX)
		{
		  *--str = negative ? '-' : ' ';
		}
	      else if (negative)
		{
		  *--str = '-';
		}

	      n = strlen (str);

	      if (width && width > n)
		{
		  char *q = stringbuf_set (&sb,
					   (flags & FMT_PADZERO) ? '0' : ' ',
					   width);
		  if (flags & FMT_ADJUST_LEFT)
		    memcpy (q, str, n);
		  else
		    {
		      if ((flags & FMT_PADZERO) && str == stringbuf_value (&tmpbuf))
			{
			  q[0] = *str++;
			  n--;
			}
		      memcpy (q + width - n, str, n);
		    }
		}
	      else
		stringbuf_add (&sb, str, n);
	      break;

	    case 'u':
	      tmpl_va_arg_n (argstart, argnum, json_integer, &num);
	      /*
	       * If a precision is given with a numeric conversion, the 0
	       * flag is ignored.
	       *
	       * A - overrides a 0 if both are given.
	       */
	      if (prec || (flags & FMT_ADJUST_LEFT))
		flags &= ~FMT_PADZERO;
	      /* A + overrides a ' ' if both are given. */
	      if (flags & FMT_SIGNPFX)
		flags &= ~FMT_SPACEPFX;

	      stringbuf_reset (&tmpbuf);
	      stringbuf_printf (&tmpbuf, "%lu", num);
	      str = stringbuf_finish (&tmpbuf);
	      n = strlen (str);
	      if (prec && prec > n)
		{
		  memmove (str + prec - n, str, n + 1);
		  memset (str, '0', prec - n);
		  n = prec;
		}

	      if (width && width > n)
		{
		  char *q = stringbuf_set (&sb,
					   (flags & FMT_PADZERO) ? '0' : ' ',
					   width);
		  if (flags & FMT_ADJUST_LEFT)
		    memcpy (q, str, n);
		  else
		    memcpy (q + width - n, str, n);
		}
	      else
		stringbuf_add (&sb, str, n);
	      break;

	    case 'x':
	    case 'X':
	      tmpl_va_arg_n (argstart, argnum, json_integer, &num);
	      /*
	       * If a precision is given with a numeric conversion, the 0
	       * flag is ignored.
	       *
	       * A - overrides a 0 if both are given.
	       */
	      if (prec || (flags & FMT_ADJUST_LEFT))
		flags &= ~FMT_PADZERO;
	      /* A + overrides a ' ' if both are given. */
	      if (flags & FMT_SIGNPFX)
		flags &= ~FMT_SPACEPFX;

	      fmtbuf[1] = format[cur];
	      /* Reset temp buffer */
	      stringbuf_reset (&tmpbuf);
	      /* Provide two extra slots */
	      stringbuf_set (&tmpbuf, ' ', 2);
	      /* Format the value */
	      stringbuf_printf (&tmpbuf, fmtbuf, num);
	      /* Point str to the start of the formatted value */
	      str = stringbuf_finish (&tmpbuf) + 2;
	      n = strlen (str);
	      if (prec && prec > n)
		{
		  memmove (str + prec - n, str, n + 1);
		  memset (str, '0', prec - n);
		  n = prec;
		}

	      if (flags & FMT_ALTERNATE)
		{
		  *--str = format[cur];
		  *--str = '0';
		  n += 2;
		}

	      if (width && width > n)
		{
		  char *q = stringbuf_set (&sb,
					   (flags & FMT_PADZERO) ? '0' : ' ',
					   width);
		  if (flags & FMT_ADJUST_LEFT)
		    memcpy (q, str, n);
		  else
		    {
		      if ((flags & FMT_ALTERNATE) && (flags & FMT_PADZERO))
			{
			  q[0] = *str++;
			  q[1] = *str++;
			  n -= 2;
			}
		      memcpy (q + width - n, str, n);
		    }
		}
	      else
		stringbuf_add (&sb, str, n);
	      break;

	    case 'o':
	      tmpl_va_arg_n (argstart, argnum, json_integer, &num);
	      /*
	       * If a precision is given with a numeric conversion, the 0
	       * flag is ignored.
	       *
	       * A - overrides a 0 if both are given.
	       */
	      if (prec || (flags & FMT_ADJUST_LEFT))
		flags &= ~FMT_PADZERO;
	      /* A + overrides a ' ' if both are given. */
	      if (flags & FMT_SIGNPFX)
		flags &= ~FMT_SPACEPFX;

	      stringbuf_reset (&tmpbuf);
	      stringbuf_set (&tmpbuf, ' ', 1);
	      stringbuf_printf (&tmpbuf, "%lo", num);
	      str = stringbuf_finish (&tmpbuf) + 1;
	      n = strlen (str);
	      if (prec && prec > n)
		{
		  memmove (str + prec - n, str, n + 1);
		  memset (str, '0', prec - n);
		}

	      if ((flags & FMT_ALTERNATE) && *str != '0')
		{
		  *--str = '0';
		  n++;
		}

	      if (width && width > n)
		{
		  char *q = stringbuf_set (&sb,
					   (flags & FMT_PADZERO) ? '0' : ' ',
					   width);
		  if (flags & FMT_ADJUST_LEFT)
		    memcpy (q, str, n);
		  else
		    memcpy (q + width - n, str, n);
		}
	      else
		stringbuf_add (&sb, str, n);
	      break;

	    case 'e':
	    case 'E':
	    case 'f':
	    case 'F':
	    case 'g':
	    case 'G':
	      {
		char ffmt[sizeof("%#0-+ *.*f")];
		int i = 0;

		tmpl_va_arg_n (argstart, argnum, json_number, &d);

		/* Prepare the format */

		if (prec || (flags & FMT_ADJUST_LEFT))
		  flags &= ~FMT_PADZERO;
		/* A + overrides a ' ' if both are given. */
		if (flags & FMT_SIGNPFX)
		  flags &= ~FMT_SPACEPFX;

		ffmt[i++] = '%';
		if (flags & FMT_ALTERNATE)
		  ffmt[i++] = '#';
		if (flags & FMT_PADZERO)
		  ffmt[i++] = '0';
		if (flags & FMT_ADJUST_LEFT)
		  ffmt[i++] = '-';
		if (flags & FMT_SPACEPFX)
		  ffmt[i++] = ' ';
		if (flags & FMT_SIGNPFX)
		  ffmt[i++] = '+';

		if (width)
		  ffmt[i++] = '*';
		if (has_prec)
		  {
		    ffmt[i++] = '.';
		    ffmt[i++] = '*';
		  }
		ffmt[i++] = format[cur];
		ffmt[i] = 0;

		/* Reset temp buffer */
		stringbuf_reset (&tmpbuf);

		/* Format the value */
		if (width)
		  {
		    if (has_prec)
		      stringbuf_printf (&tmpbuf, ffmt, width, prec, d);
		    else
		      stringbuf_printf (&tmpbuf, ffmt, prec, d);
		  }
		else
		  {
		    if (has_prec)
		      stringbuf_printf (&tmpbuf, ffmt, prec, d);
		    else
		      stringbuf_printf (&tmpbuf, ffmt, d);
		  }
	      }
	      str = stringbuf_finish (&tmpbuf);
	      stringbuf_add_string (&sb, str);
	      break;

	    case 'v':
	      tmpl_va_arg_n (argstart, argnum, json_object, &jv);
	      // FIXME: flags?
	      stringbuf_print_json (&sb, jv);
	      break;

	    default:
	      stringbuf_add (&sb, &format[start], cur - start + 1);
	    }

	  cur++;
	  state = fmts_copy;
	}
    }

  jv = json_new_string (stringbuf_finish (&sb));
  stringbuf_free (&sb);
  stringbuf_free (&tmpbuf);
  return jv;
}

/*
 * Function table
 */
struct func_def
{
  char *name;
  TMPL_FUNC func;
};

static struct func_def funtab[] = {
  { "and", func_and },
  { "or", func_or },
  { "index", func_index },
  { "len", func_len },
  { "not", func_not },
  { "eq", func_eq },
  { "ne", func_ne },
  { "lt", func_lt },
  { "le", func_le },
  { "gt", func_gt },
  { "ge", func_ge },
  { "even", func_even },
  { "printf", func_printf },
  { "typeof", func_typeof },
  { "exists", func_exists },
  { "add", func_add },
  { "sub", func_sub },
  { "mul", func_mul },
  { "div", func_div },
  { NULL }
};

static TMPL_FUNC
find_func (char *name)
{
  struct func_def *def;

  for (def = funtab; def->name; def++)
    {
      if (strcmp (def->name, name) == 0)
	return def->func;
    }
  return NULL;
}

/* Token types */
enum
  {
    TMPL_TOK_EOF,        /* End of file */
    TMPL_TOK_TEXT = 256, /* Arbitrary text */
    TMPL_TOK_STR,        /* String value */
    TMPL_TOK_NUM,        /* Numeric value */
    TMPL_TOK_DOT,        /* . */
    TMPL_TOK_ATTR,       /* .Attr */
    TMPL_TOK_VAR,        /* $VAR */
    TMPL_TOK_IDENT,      /* Identifier (function name) */
    TMPL_TOK_BEG,        /* Opening {{ */
    TMPL_TOK_END,        /* Terminating }} */
    TMPL_TOK_ERR,        /* Error */
  };

/* Errors */
static char const *tmpl_error_text[] = {
  "No error",
  "Unexpected end of file",
  "Number out of range",
  "No such function",
  "Unexpected token",
  "No such template defined"
};

char const *
template_strerror (int ec)
{
  if (ec >= 0 && ec < sizeof (tmpl_error_text) / sizeof (tmpl_error_text[0]))
    return tmpl_error_text[ec];
  return "Unknown error";
}

struct tmpl_tok
{
  int type;
  union
  {
    char *str;
    long num;
  } v;
};

struct tmpl_input
{
  char *text;
  size_t off;
  size_t start;
  int error;
  struct tmpl_tok tok;  /* Current token */
  int ready;            /* Token already parsed and put back */
};

static void
tmpl_input_init (struct tmpl_input *inp, char *text)
{
  memset (inp, 0, sizeof (*inp));
  inp->text = text;
}

static inline struct tmpl_tok *
tmpl_input_error (struct tmpl_input *inp, int err)
{
  inp->error = err;
  inp->tok.type = TMPL_TOK_ERR;
  return &inp->tok;
}

static void
tmpl_input_mark (struct tmpl_input *inp)
{
  inp->start = inp->off;
}

static inline int
tmpl_getchar (struct tmpl_input *inp)
{
  int c;
  if ((c = inp->text[inp->off]) != 0)
    inp->off++;
  return c;
}

static inline int
tmpl_lookahead (struct tmpl_input *inp)
{
  return inp->text[inp->off];
}

static inline void
tmpl_input_less (struct tmpl_input *inp, size_t len)
{
  assert (len <= inp->off);
  inp->off -= len;
}

static inline void
tmpl_input_putback (struct tmpl_input *inp)
{
  assert (inp->ready == 0);
  inp->ready = 1;
}

static void
tmpl_tok_free (struct tmpl_tok *tok)
{
  switch (tok->type)
    {
    case TMPL_TOK_TEXT:
    case TMPL_TOK_ATTR:
    case TMPL_TOK_VAR:
    case TMPL_TOK_IDENT:
    case TMPL_TOK_STR:
      free (tok->v.str);
      break;

    default:
      break;
    }
}

static struct tmpl_tok *
tmpl_input_token (struct tmpl_input *inp, int type)
{
  struct tmpl_tok *tok = &inp->tok;

  tmpl_tok_free (tok);

  if (inp->start == inp->off)
    tok->type = TMPL_TOK_EOF;
  else
    tok->type = type;
  switch (tok->type)
    {
    case TMPL_TOK_EOF:
    case TMPL_TOK_DOT:
    case TMPL_TOK_END:
    case TMPL_TOK_ERR:
    case TMPL_TOK_BEG:
      break;

    case TMPL_TOK_TEXT://FIXME: see comment to tmpl_gettext
    case TMPL_TOK_ATTR:
    case TMPL_TOK_VAR:
    case TMPL_TOK_IDENT:
      {
	size_t len = inp->off - inp->start;
	tok->v.str = xmalloc (len + 1);
	memcpy (tok->v.str, inp->text + inp->start, len);
	tok->v.str[len] = 0;
      }
      break;

    case TMPL_TOK_STR:
      {
	size_t len = inp->off - inp->start;
	char *start = inp->text + inp->start;
	char *end = inp->text + inp->off - 1;
	char *p;

	tok->v.str = xmalloc (len);
	for (p = tok->v.str; start < end; start++)
	  {
	    if (*start == '\\')
	      {
		switch (*++start)
		  {
		  case '\\':
		  case '\"':
		    *p++ = *start;
		    break;

		  case 'a':
		    *p++ = '\a';
		    break;

		  case 'b':
		    *p++ = '\b';
		    break;

		  case 'f':
		    *p++ = '\f';
		    break;

		  case 'n':
		    *p++ = '\n';
		    break;

		  case 'r':
		    *p++ = '\r';
		    break;

		  case 't':
		    *p++ = '\t';
		    break;

		  case 'v':
		    *p++ = '\v';
		    break;

		  default:
		    *p++ = '\\';
		    *p++ = *start;
		  }
	      }
	    else
	      *p++ = *start;
	  }
	*p = 0;
      }
      break;

    case TMPL_TOK_NUM:
      /* should not happen */
      abort();

    default:
      // FIXME
      if (tok->type > 128)
	abort ();
    }
  inp->start = inp->off;
  return tok;
}

static struct tmpl_tok *
tmpl_gettok (struct tmpl_input *inp)
{
  int c;

  if (inp->ready)
    {
      if (inp->tok.type != TMPL_TOK_ERR)
	inp->ready = 0;
      return &inp->tok;
    }

 again:
  do
    {
      if ((c = tmpl_getchar (inp)) == 0)
	return tmpl_input_token (inp, TMPL_TOK_EOF);
    }
  while (isspace (c));

  tmpl_input_less (inp, 1);
  tmpl_input_mark (inp);
  tmpl_getchar (inp);

  if (isalpha (c))
    {
      while ((c = tmpl_getchar (inp)) != 0 && (isalnum (c) || c == '_'))
	;
      tmpl_input_less (inp, 1);
      return tmpl_input_token (inp, TMPL_TOK_IDENT);
    }

  if (isdigit (c))
    {
      long n = 0;
      //FIXME: move to tmpl_input_token
      do
	{
	  c -= '0';
	  if (LONG_MAX - n < c)
	    return tmpl_input_error (inp, TMPL_ERR_RANGE);
	  n = n * 10 + c;
	}
      while ((c = tmpl_getchar (inp)) != 0 && isdigit (c));
      tmpl_input_less (inp, 1);
      inp->tok.type = TMPL_TOK_NUM;
      inp->tok.v.num = n;
      return &inp->tok;
    }

  if (c == '"')
    {
      tmpl_input_mark (inp);
      do
	{
	  if ((c = tmpl_getchar (inp)) == 0)
	    return tmpl_input_error (inp, TMPL_ERR_EOF);
	  if (c == '\\' && (c = tmpl_getchar (inp)) == 0)
	    return tmpl_input_error (inp, TMPL_ERR_EOF);
	}
      while (c != '"');
      return tmpl_input_token (inp, TMPL_TOK_STR);
    }
  else if (c == '.')
    {
      if (isalpha (tmpl_lookahead (inp)))
	{
	  tmpl_input_mark (inp);
	  while ((c = tmpl_getchar (inp)) != 0 &&
		 (isalnum (c) || c == '_' || c == '-'))
	    ;
	  tmpl_input_less (inp, 1);
	  return tmpl_input_token (inp, TMPL_TOK_ATTR);
	}
      return tmpl_input_token (inp, TMPL_TOK_DOT);
    }
  else if (c == '-' && isdigit (tmpl_lookahead (inp)))
    {
      long n = 0;
      while ((c = tmpl_getchar (inp)) != 0 && isdigit (c))
	{
	  c -= '0';
	  if (-(LONG_MIN+1) - n < c)
	    return tmpl_input_error (inp, TMPL_ERR_RANGE);
	  n = n * 10 + c;
	}
      tmpl_input_less (inp, 1);
      inp->tok.type = TMPL_TOK_NUM;
      inp->tok.v.num = - n;
      return &inp->tok;
    }
  else if (c == '$' && isalpha (tmpl_lookahead (inp)))
    {
      tmpl_input_mark (inp);
      while ((c = tmpl_getchar (inp)) != 0 && (isalnum (c) || c == '_'))
	;
      tmpl_input_less (inp, 1);
      return tmpl_input_token (inp, TMPL_TOK_VAR);
    }
  else if (c == '/' && tmpl_lookahead (inp) == '*')
    {
      tmpl_getchar (inp);
      while ((c = tmpl_getchar (inp)) != 0)
	{
	  if (c == '*')
	    {
	      if ((c = tmpl_getchar (inp)) == '/')
		goto again;
	      tmpl_input_less (inp, 1);
	    }
	}
      return tmpl_input_error (inp, TMPL_ERR_EOF);
    }
  else if (c == '{' && tmpl_lookahead (inp) == '{')
    {
      tmpl_getchar (inp);
      if (tmpl_lookahead (inp) == '-' && isspace (inp->text[inp->off+1]))
	inp->off += 2;
      return tmpl_input_token (inp, TMPL_TOK_BEG);
    }
  else if (c == '}' && tmpl_lookahead (inp) == '}')
    {
      tmpl_getchar (inp);
      return tmpl_input_token (inp, TMPL_TOK_END);
    }
  else if (c == '-' && inp->off > 2 && isspace (inp->text[inp->off-2]) &&
	   inp->text[inp->off] == '}' && inp->text[inp->off+1] == '}')
    {
      tmpl_input_token (inp, TMPL_TOK_END);
      inp->off += 2;
      /* Skip leading whitespace */
      while (inp->text[inp->off] && isspace (inp->text[inp->off]))
	inp->off++;
      return &inp->tok;
    }

  /* Punctuation or other character */
  return tmpl_input_token (inp, c);
}

static struct tmpl_tok *
tmpl_gettext (struct tmpl_input *inp)
{
  int c;
  int trim_right = 0;
  size_t end, len;
  struct tmpl_tok *tok;

  tmpl_input_mark (inp);
  while ((c = tmpl_getchar (inp)) != 0)
    {
      if (c == '{' && tmpl_lookahead (inp) == c)
	{
	  if (inp->text[inp->off+1] == '-' && isspace (inp->text[inp->off+2]))
	    {
	      trim_right = 1;
	    }
	  if (inp->start + 1 == inp->off)
	    {
	      inp->off++;
	      if (trim_right)
		inp->off += 2;
	      return tmpl_input_token (inp, TMPL_TOK_BEG);
	    }
	  tmpl_input_less (inp, 1);
	  break;
	}
    }

  end = inp->off;

  if (trim_right)
    {
      while (end > inp->start && isspace (inp->text[end-1]))
	end--;
    }

  if (end == inp->start)
    return tmpl_gettok (inp);

  //FIXME: move this to tmpl_input_token
  tok = &inp->tok;
  tmpl_tok_free (tok);
  len = end - inp->start;
  tok->v.str = xmalloc (len + 1);
  memcpy (tok->v.str, inp->text + inp->start, len);
  tok->v.str[len] = 0;

  tok->type = TMPL_TOK_TEXT;

  return tok;
}

/*
 * Pipeline parser
 */
static struct tmpl_command *
tmpl_command_new (TMPL_PIPELINE *head, int type)
{
  struct tmpl_command *com;

  XZALLOC (com);
  com->type = type;
  SLIST_PUSH (head, com, next);
  return com;
}

static int scan_pipeline (struct tmpl_input *inp, TMPL_PIPELINE *head);

static int
scan_arg (struct tmpl_input *inp, TMPL_PIPELINE *head)
{
  struct tmpl_tok *tok;
  struct tmpl_command *com;
  int rc;
  int apply_attr = 0;
  struct tmpl_command *tail = NULL;

  tok = tmpl_gettok (inp);
  switch (tok->type)
    {
    case TMPL_TOK_DOT:
      com = tmpl_command_new (head, TMPL_COMMAND_DOT);
      break;

    case TMPL_TOK_ATTR:
      apply_attr = 1;
      tail = SLIST_LAST (head);
      com = tmpl_command_new (head, TMPL_COMMAND_ATTR);
      com->v.varname = tok->v.str;
      tok->v.str = NULL;
      break;

    case '(':
      apply_attr = 1;
      tail = SLIST_LAST (head);
      com = tmpl_command_new (head, TMPL_COMMAND_PIPELINE);
      if ((rc = scan_pipeline (inp, &com->v.pipeline)) != ')')
	return 1;
      break;

    case TMPL_TOK_NUM:
      com = tmpl_command_new (head, TMPL_COMMAND_ARG);
      com->v.arg = json_new_integer (tok->v.num);
      break;

    case TMPL_TOK_STR:
      com = tmpl_command_new (head, TMPL_COMMAND_ARG);
      com->v.arg = json_new_string (tok->v.str);
      break;

    case TMPL_TOK_IDENT:
      if (strcmp (tok->v.str, "true") == 0)
	{
	  com = tmpl_command_new (head, TMPL_COMMAND_ARG);
	  com->v.arg = json_new_bool (1);
	}
      else if (strcmp (tok->v.str, "false") == 0)
	{
	  com = tmpl_command_new (head, TMPL_COMMAND_ARG);
	  com->v.arg = json_new_bool (0);
	}
      else
	{
	  com = tmpl_command_new (head, TMPL_COMMAND_FUNCALL);
	  com->v.call.func = find_func (tok->v.str);
	  if (!com->v.call.func)
	    {
	      tmpl_input_token (inp, TMPL_ERR_NOFUNC);
	      return 1;
	    }

	  SLIST_INIT (&com->v.call.args);
	  for (;;)
	    {
	      struct tmpl_formal_arg *arg;
	      XZALLOC (arg);
	      XZALLOC (arg->pipeline);
	      if (scan_arg (inp, arg->pipeline))
		{
		  free (arg->pipeline);
		  free (arg);
		  break;
		}
	      SLIST_PUSH (&com->v.call.args, arg, next);
	    }
	}
      break;

    case TMPL_TOK_VAR:
      apply_attr = 1;
      tail = SLIST_LAST (head);
      com = tmpl_command_new (head, TMPL_COMMAND_VAR);
      com->v.varname = tok->v.str;
      tok->v.str = NULL;
      break;

    default:
      tmpl_input_putback (inp);
      return 1;
    }

 again:
  if (apply_attr)
    {
      if (!isspace (tmpl_lookahead (inp)))
	{
	  if ((tok = tmpl_gettok (inp))->type == TMPL_TOK_ATTR)
	    {
	      /* Save off last element */
	      struct tmpl_command *dot = SLIST_LAST (head);

	      /* Remove it from the list */
	      if (tail)
		{
		  tail->next = NULL;
		  SLIST_LAST (head) = tail;
		}
	      else
		SLIST_INIT (head);

	      /* Create new WITHDOT command */
	      com = tmpl_command_new (head, TMPL_COMMAND_WITHDOT);
	      SLIST_INIT (&com->v.withdot.dot);

	      /* Set saved dot to it. */
	      SLIST_PUSH (&com->v.withdot.dot, dot, next);

	      /* Attach a single ATTR command to its pipeline. */
	      SLIST_INIT (&com->v.withdot.body);
	      com = tmpl_command_new (&com->v.withdot.body, TMPL_COMMAND_ATTR);
	      com->v.varname = tok->v.str;

	      /* Prevent attribute name from being freed. */
	      tok->v.str = NULL;
	      goto again;
	    }
	  else
	    tmpl_input_putback (inp);
	}
    }

  return 0;
}

int
scan_pipeline (struct tmpl_input *inp, TMPL_PIPELINE *head)
{
  do
    {
      if (scan_arg (inp, head))
	break;
    }
  while (tmpl_gettok (inp)->type == '|');

  return inp->tok.type;
}

enum tmpl_action_type
  {
    TMPL_ACT_TEXT,
    TMPL_ACT_PIPELINE,
    TMPL_ACT_COND,
    TMPL_ACT_WITH,
    TMPL_ACT_RANGE,
    TMPL_ACT_BREAK,
    TMPL_ACT_CONTINUE,
    TMPL_ACT_BLOCK,
  };

typedef SLIST_HEAD (,tmpl_action) TMPL_ACTION_LIST;

struct tmpl_conditional
{
  TMPL_PIPELINE cond;
  TMPL_ACTION_LIST branches[2];
};

struct tmpl_range
{
  TMPL_PIPELINE cond;
  TMPL_ACTION_LIST branches[2];
  char *vars[2];
};

struct tmpl_with
{
  TMPL_PIPELINE cond;
  TMPL_ACTION_LIST branches[2];
  char *var;
};

struct tmpl_block
{
  TMPL_PIPELINE arg;
  TEMPLATE tmpl;
};

struct tmpl_action
{
  enum tmpl_action_type type;
  SLIST_ENTRY (tmpl_action) next;
  union
  {
    char *text;
    TMPL_PIPELINE pipeline;
    struct tmpl_conditional cond;
    struct tmpl_range range;
    struct tmpl_block block;
    struct tmpl_with with;
  } v;
};

static void
tmpl_env_write (struct tmpl_env *env, char const *str, size_t len)
{
  fwrite (str, 1, len, env->outfile);
}

static void
env_write_string (void *data, char const *str, size_t len)
{
  struct tmpl_env *env = data;
  tmpl_env_write (env, str, len);
}

static void
tmpl_env_print_json (struct tmpl_env *env, struct json_value *val)
{
  switch (val->type)
    {
    case json_null:
      return;

    case json_string:
      tmpl_env_write (env, val->v.s, strlen (val->v.s));
      break;

    default:
      {
	struct json_format format = {
	  .indent = 0,
	  .precision = 0,
	  .write = env_write_string,
	  .data = env
	};
	json_value_format (val, &format, 0);
      }
    }
}

static void tmpl_action_list_free (TMPL_ACTION_LIST *head);

static void
tmpl_action_free (struct tmpl_action *act)
{
  switch (act->type)
    {
    case TMPL_ACT_TEXT:
      free (act->v.text);
      break;

    case TMPL_ACT_PIPELINE:
      tmpl_pipeline_free (&act->v.pipeline);
      break;

    case TMPL_ACT_COND:
      tmpl_pipeline_free (&act->v.cond.cond);
      tmpl_action_list_free (&act->v.cond.branches[0]);
      tmpl_action_list_free (&act->v.cond.branches[1]);
      break;

    case TMPL_ACT_WITH:
      tmpl_pipeline_free (&act->v.with.cond);
      tmpl_action_list_free (&act->v.with.branches[0]);
      tmpl_action_list_free (&act->v.with.branches[1]);
      free (act->v.with.var);
      break;

    case TMPL_ACT_RANGE:
      tmpl_pipeline_free (&act->v.range.cond);
      tmpl_action_list_free (&act->v.range.branches[0]);
      tmpl_action_list_free (&act->v.range.branches[1]);
      free (act->v.range.vars[0]);
      free (act->v.range.vars[1]);
      break;

    case TMPL_ACT_BLOCK:
    case TMPL_ACT_CONTINUE:
    case TMPL_ACT_BREAK:
      break;
    }
  free (act);
}

static void
tmpl_action_list_free (TMPL_ACTION_LIST *head)
{
  while (!SLIST_EMPTY (head))
    {
      struct tmpl_action *act = SLIST_FIRST (head);
      SLIST_SHIFT (head, next);
      tmpl_action_free (act);
    }
}

static void tmpl_run_in_env (TEMPLATE tmpl, struct json_value *val, struct tmpl_env *top);

int
tmpl_action_eval (struct tmpl_env *env, TMPL_ACTION_LIST *head)
{
  struct tmpl_action *act;
  int res = 0;

  SLIST_FOREACH (act, head, next)
    {
      if (res)
	break;

      switch (act->type)
	{
	case TMPL_ACT_TEXT:
	  tmpl_env_write (env, act->v.text, strlen (act->v.text));
	  break;

	case TMPL_ACT_PIPELINE:
	  {
	    struct json_value *jv;
	    jv = tmpl_pipeline_eval (env, &act->v.pipeline);
	    if (jv->type != json_null)
	      tmpl_env_print_json (env, jv);
	    json_value_free (jv);
	  }
	  break;

	case TMPL_ACT_COND:
	  {
	    struct json_value *jv;
	    int bn;

	    jv = tmpl_pipeline_eval (env, &act->v.cond.cond);
	    bn = is_true (jv);
	    json_value_free (jv);

	    res = tmpl_action_eval (env, &act->v.cond.branches[bn]);
	  }
	  break;

	case TMPL_ACT_WITH:
	  {
	    struct json_value *jv;

	    jv = tmpl_pipeline_eval (env, &act->v.with.cond);
	    if (!is_empty (jv))
	      {
		struct tmpl_env *sub_env;

		if (act->v.with.var)
		  {
		    sub_env = tmpl_env_new (env->dot, 0, env);
		    json_object_set (sub_env->vars, act->v.with.var, jv);
		  }
		else
		  {
		    sub_env = tmpl_env_new (jv, 1, env);
		  }
		res = tmpl_action_eval (sub_env, &act->v.with.branches[1]);
		tmpl_env_free (sub_env);
	      }
	    else
	      res = tmpl_action_eval (env, &act->v.with.branches[0]);
	  }
	  break;

	case TMPL_ACT_RANGE:
	  {
	    struct json_value *jv;
	    size_t len;

	    jv = tmpl_pipeline_eval (env, &act->v.range.cond);
	    switch (jv->type)
	      {
	      case json_array:
		if ((len = json_array_length (jv)) > 0)
		  {
		    size_t i;
		    struct tmpl_env *sub_env;

		    sub_env = tmpl_env_new (env->dot, 0, env);

		    for (i = 0; i < len; i++)
		      {
			if (act->v.range.vars[0] || act->v.range.vars[1])
			  {
			    if (act->v.range.vars[0])
			      json_object_set (sub_env->vars,
					       act->v.range.vars[0],
					       json_new_integer (i));
			    if (act->v.range.vars[1])
			      {
				struct json_value *v, *t;
				json_array_get (jv, i, &v);
				json_value_copy (v, &t);
				json_object_set (sub_env->vars,
						 act->v.range.vars[1], t);
			      }
			  }
			else
			  json_array_get (jv, i, &sub_env->dot);
			if (tmpl_action_eval (sub_env, &act->v.range.branches[1]))
			  break;
		      }
		    tmpl_env_free (sub_env);
		  }
		else
		  res = tmpl_action_eval (env, &act->v.range.branches[0]);
		break;

	      case json_object:
		if (!SLIST_EMPTY (&jv->v.o->pair_head))
		  {
		    struct json_pair *pair;
		    struct tmpl_env *sub_env;

		    sub_env = tmpl_env_new (env->dot, 0, env);

		    SLIST_FOREACH (pair, &jv->v.o->pair_head, next)
		      {
			if (act->v.range.vars[0] || act->v.range.vars[1])
			  {
			    if (act->v.range.vars[0])
			      json_object_set (sub_env->vars,
					       act->v.range.vars[0],
					       json_new_string (pair->k));
			    if (act->v.range.vars[1])
			      {
				struct json_value *t;
				json_value_copy (pair->v, &t);
				json_object_set (sub_env->vars,
						 act->v.range.vars[1], t);
			      }
			  }
			else
			  sub_env->dot = pair->v;
			if (tmpl_action_eval (sub_env, &act->v.range.branches[1]))
			  break;
		      }
		    tmpl_env_free (sub_env);
		  }
		else
		  res = tmpl_action_eval (env, &act->v.range.branches[0]);
		break;

	      default:
		res = tmpl_action_eval (env, &act->v.range.branches[0]);
	      }
	  }
	  break;

	case TMPL_ACT_BLOCK:
	  {
	    struct json_value *arg;

	    arg = tmpl_pipeline_eval (env, &act->v.block.arg);
	    tmpl_run_in_env (act->v.block.tmpl, arg, env);
	    break;
	  }

	case TMPL_ACT_CONTINUE:
	  goto end;

	case TMPL_ACT_BREAK:
	  res = 1;
	  goto end;

	default:
	  abort ();
	}
    }
 end:
  return res;
}

static struct tmpl_action *
tmpl_action_new (TMPL_ACTION_LIST *head, int type)
{
  struct tmpl_action *act;

  XZALLOC (act);
  act->type = type;
  SLIST_PUSH (head, act, next);
  return act;
}

enum
  {
    TMPL_ACTION_OK,
    TMPL_ACTION_END,
    TMPL_ACTION_ERR
  };

/* Indices of parser table */
enum
  {
    PT_GLOBAL,     /* Global table: immutable */
    PT_LOCAL,      /* Local table: cleared when recursing */
    PT_INHERIT,    /* First inheritable table */
    PT_MAX = 16    /* MAX Number of tables */
  };

typedef struct action_parser_def *PARSER_TABLES[PT_MAX];

typedef int (*ACTION_PARSER) (struct tmpl_input *in, PARSER_TABLES tab, TMPL_ACTION_LIST *head, void *data);

struct action_parser_def
{
  char *name;
  ACTION_PARSER parser;
};

static void
parser_tables_copy (PARSER_TABLES dst, PARSER_TABLES src,
		    struct action_parser_def *local,
		    struct action_parser_def *inherit)
{
  int i, j;

  dst[PT_GLOBAL] = src[PT_GLOBAL];
  dst[PT_LOCAL] = local;
  for (i = j = PT_INHERIT; i < PT_MAX && src[i]; i++)
    {
      dst[j++] = src[i];
      if (src[i] == inherit)
	inherit = NULL;
    }

  if (inherit)
    {
      assert (j < PT_MAX);
      dst[j++] = inherit;
    }

  for (; j < PT_MAX; j++)
    dst[j] = NULL;
}

static int scan_action (struct tmpl_input *in,
			PARSER_TABLES tab,
			TMPL_ACTION_LIST *head,
			void *data);
static int if_action_parser (struct tmpl_input *in,
			     PARSER_TABLES tab,
			     TMPL_ACTION_LIST *head,
			     void *data);
static int with_action_parser (struct tmpl_input *in,
			       PARSER_TABLES tab,
			       TMPL_ACTION_LIST *head,
			       void *data);

struct cond_state
{
  TMPL_ACTION_LIST *head;
};

static int elseif_action_parser (struct tmpl_input *in, PARSER_TABLES tab,
				 TMPL_ACTION_LIST *head, void *data);
static int endif_action_parser (struct tmpl_input *in,
				PARSER_TABLES tab,
				TMPL_ACTION_LIST *head, void *data);

static struct action_parser_def conddef[] = {
  { "else", elseif_action_parser },
#define enddef (conddef + 1)
  { "end", endif_action_parser },
  { NULL }
};

static int
elseif_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
		      TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_tok *tok;
  struct cond_state *state = data;
  PARSER_TABLES loctab;

  tok = tmpl_gettok (in);
  if (tok->type == TMPL_TOK_IDENT && strcmp (tok->v.str, "if") == 0)
    {
      parser_tables_copy (loctab, acttab, NULL, NULL);
      if_action_parser (in, loctab, state->head, NULL);
    }
  else if (tok->type == TMPL_TOK_END)
    {
      parser_tables_copy (loctab, acttab, enddef, NULL);
      scan_action (in, loctab, state->head, NULL);
    }
  else
    return TMPL_ACTION_ERR;

  return TMPL_ACTION_END;
}

static int
endif_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
		     TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_tok *tok = tmpl_gettok (in);
  if (tok->type != TMPL_TOK_END)
    return TMPL_ACTION_ERR;
  return TMPL_ACTION_END;
}

static int
if_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
		  TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_action *act = tmpl_action_new (head, TMPL_ACT_COND);
  struct cond_state state = { &act->v.cond.branches[0] };
  PARSER_TABLES loctab;

  if (scan_pipeline (in, &act->v.cond.cond) != TMPL_TOK_END)
    {
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }

  parser_tables_copy (loctab, acttab, conddef, NULL);
  scan_action (in, loctab, &act->v.cond.branches[1], &state);

  return in->error ? TMPL_ACTION_ERR : TMPL_ACTION_OK;
}

static int elsewith_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
				   TMPL_ACTION_LIST *head, void *data);

static struct action_parser_def withdef[] = {
  { "else", elsewith_action_parser },
#define endwith (withdef + 1)
  { "end", endif_action_parser },
  { NULL }
};

static int
elsewith_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
			TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_tok *tok;
  struct cond_state *state = data;

  tok = tmpl_gettok (in);
  if (tok->type == TMPL_TOK_END)
    {
      PARSER_TABLES loctab;

      parser_tables_copy (loctab, acttab, endwith, NULL);
      scan_action (in, loctab, state->head, NULL);
    }
  else
    return TMPL_ACTION_ERR;

  return TMPL_ACTION_END;
}

static int
parse_with_var (struct tmpl_input *in, char **var)
{
  struct tmpl_tok *tok;
  size_t save_off;

  var[0] = NULL;

  save_off = in->off;

  tok = tmpl_gettok (in);
  if (tok->type == TMPL_TOK_VAR)
    {
      var[0] = tok->v.str;
      tok->v.str = NULL;
    }
  else if (tok->type != '_')
    {
      tmpl_input_putback (in);
      return 0;
    }

  tok = tmpl_gettok (in);
  if (tok->type != '=')
    {
      in->off = in->start = save_off;
      free (var[0]);
      return -1;
    }

  return 0;
}

static int
with_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
		    TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_action *act = tmpl_action_new (head, TMPL_ACT_WITH);
  struct cond_state state = { &act->v.with.branches[0] };
  PARSER_TABLES loctab;

  if (parse_with_var (in, &act->v.with.var))
    return TMPL_ACTION_ERR;

  if (scan_pipeline (in, &act->v.with.cond) != TMPL_TOK_END)
    {
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }

  parser_tables_copy (loctab, acttab, withdef, NULL);
  scan_action (in, loctab, &act->v.with.branches[1], &state);

  return in->error ? TMPL_ACTION_ERR : TMPL_ACTION_OK;
}

static int
break_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
		     TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_tok *tok = tmpl_gettok (in);
  if (tok->type == TMPL_TOK_END)
    {
      tmpl_action_new (head, TMPL_ACT_BREAK);
    }
  else
    return TMPL_ACTION_ERR;
  return TMPL_ACTION_OK;
}

static int
continue_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
			TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_tok *tok = tmpl_gettok (in);
  if (tok->type == TMPL_TOK_END)
    {
      tmpl_action_new (head, TMPL_ACT_CONTINUE);
    }
  else
    return TMPL_ACTION_ERR;
  return TMPL_ACTION_OK;
}

static struct action_parser_def rangedef[] = {
  { "else", elsewith_action_parser },
  { "end", endif_action_parser },
  { NULL }
};

static struct action_parser_def range_inherit[] = {
  { "break", break_action_parser },
  { "continue", continue_action_parser },
  { NULL }
};

static int
parse_range_vars (struct tmpl_input *in, char **vars)
{
  struct tmpl_tok *tok;

  vars[0] = vars[1] = NULL;

  tok = tmpl_gettok (in);
  if (tok->type == TMPL_TOK_VAR)
    {
      vars[0] = tok->v.str;
      tok->v.str = NULL;
    }
  else if (tok->type != '_')
    {
      tmpl_input_putback (in);
      return 0;
    }

  tok = tmpl_gettok (in);
  if (tok->type != ',')
    {
      free (vars[0]);
      in->error = TMPL_ERR_BADTOK;
      return -1;
    }

  tok = tmpl_gettok (in);
  if (tok->type == TMPL_TOK_VAR)
    {
      vars[1] = tok->v.str;
      tok->v.str = NULL;
    }
  else if (tok->type != '_')
    {
      free (vars[0]);
      in->error = TMPL_ERR_BADTOK;
      return -1;
    }

  tok = tmpl_gettok (in);
  if (tok->type != '=')
    {
      free (vars[0]);
      free (vars[1]);
      in->error = TMPL_ERR_BADTOK;
      return -1;
    }

  return 0;
}

static int
range_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
		     TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_action *act = tmpl_action_new (head, TMPL_ACT_RANGE);
  struct cond_state state = { &act->v.range.branches[0] };
  PARSER_TABLES loctab;

  if (parse_range_vars (in, act->v.range.vars))
    return TMPL_ACTION_ERR;

  if (scan_pipeline (in, &act->v.range.cond) != TMPL_TOK_END)
    {
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }

  parser_tables_copy (loctab, acttab, rangedef, range_inherit);
  scan_action (in, loctab, &act->v.range.branches[1], &state);

  return in->error ? TMPL_ACTION_ERR : TMPL_ACTION_OK;
}

static TEMPLATE template_define (const char *name, TMPL_ACTION_LIST *head);

static int
block_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
		     TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_action *act;
  struct tmpl_tok *tok;
  TMPL_PIPELINE arg = SLIST_HEAD_INITIALIZER (arg);
  char *name;
  TEMPLATE t;

  tok = tmpl_gettok (in);
  if (tok->type != TMPL_TOK_STR)
    {
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }
  name = tok->v.str;
  tok->v.str = NULL;

  if (scan_pipeline (in, &arg) != TMPL_TOK_END)
    {
      free (name);
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }

  if ((t = template_lookup (name)) == NULL)
    {
      /* Define block */
      PARSER_TABLES loctab = {
	[PT_GLOBAL] = acttab[PT_GLOBAL],
	[PT_LOCAL] = endwith
	/* Notice: no inherited tables */
      };
      TMPL_ACTION_LIST block = SLIST_HEAD_INITIALIZER (block);

      /* Define block */
      scan_action (in, loctab, &block, NULL);
      if (in->error)
	{
	  free (name);
	  return TMPL_ACTION_ERR;
	}

      t = template_define (name, &block);
    }

  // run block
  act = tmpl_action_new (head, TMPL_ACT_BLOCK);
  act->v.block.tmpl = t;
  SLIST_COPY (&act->v.block.arg, &arg);

  return TMPL_ACTION_OK;
}

static int
define_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
		      TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_tok *tok;
  char *name;
  PARSER_TABLES loctab = {
    [PT_GLOBAL] = acttab[PT_GLOBAL],
    [PT_LOCAL] = endwith
    /* Notice: no inherited tables */
  };
  TMPL_ACTION_LIST block = SLIST_HEAD_INITIALIZER (block);

  tok = tmpl_gettok (in);
  if (tok->type != TMPL_TOK_STR)
    {
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }
  name = tok->v.str;
  tok->v.str = NULL;

  tok = tmpl_gettok (in);
  if (tok->type != TMPL_TOK_END)
    {
      free (name);
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }

  /* Define block */
  scan_action (in, loctab, &block, NULL);
  if (in->error)
    {
      free (name);
      return TMPL_ACTION_ERR;
    }

  template_define (name, &block);
  return TMPL_ACTION_OK;
}

static int
template_action_parser (struct tmpl_input *in, PARSER_TABLES acttab,
			TMPL_ACTION_LIST *head, void *data)
{
  struct tmpl_action *act;
  struct tmpl_tok *tok;
  TMPL_PIPELINE arg = SLIST_HEAD_INITIALIZER (arg);
  char *name;
  TEMPLATE t;

  tok = tmpl_gettok (in);
  if (tok->type != TMPL_TOK_STR)
    {
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }
  name = tok->v.str;
  tok->v.str = NULL;

  if (scan_pipeline (in, &arg) != TMPL_TOK_END)
    {
      free (name);
      if (!in->error)
	in->error = TMPL_ERR_BADTOK;
      return TMPL_ACTION_ERR;
    }

  if ((t = template_lookup (name)) == NULL)
    {
      in->error = TMPL_ERR_NOTMPL;
      return TMPL_ACTION_ERR;
    }

  // run block
  act = tmpl_action_new (head, TMPL_ACT_BLOCK);
  act->v.block.tmpl = t;
  SLIST_COPY (&act->v.block.arg, &arg);

  return TMPL_ACTION_OK;
}

static struct action_parser_def top_parser_tab[] = {
  { "if", if_action_parser },
  { "with", with_action_parser },
  { "range", range_action_parser },
  { "block", block_action_parser },
  { "define", define_action_parser },
  { "template", template_action_parser },
  { NULL }
};

static ACTION_PARSER
find_action_parser_def (struct action_parser_def *deftab, char const *name)
{
  if (deftab)
    {
      for (; deftab->name; deftab++)
	if (strcmp (deftab->name, name) == 0)
	  return deftab->parser;
    }
  return NULL;
}

static ACTION_PARSER
find_action_parser (PARSER_TABLES tab, char const *name)
{
  int i;
  for (i = 0; i < PT_MAX; i++)
    {
      ACTION_PARSER p = find_action_parser_def(tab[i], name);
      if (p)
	return p;
    }
  return NULL;
}

static int
scan_action (struct tmpl_input *in, PARSER_TABLES acttab,
	     TMPL_ACTION_LIST *head,
	     void *data)
{
  struct tmpl_tok *tok;
  struct tmpl_action *act;
  ACTION_PARSER parser;

  while ((tok = tmpl_gettext (in))->type != TMPL_TOK_EOF)
    {
      switch (tok->type)
	{
	case TMPL_TOK_TEXT:
	  act = tmpl_action_new (head, TMPL_ACT_TEXT);
	  act->v.text = tok->v.str;
	  tok->v.str = NULL;
	  break;

	case TMPL_TOK_BEG:
	  tok = tmpl_gettok (in);
	  if (tok->type == TMPL_TOK_END)
	    /* skip */;
	  else if (tok->type == TMPL_TOK_IDENT
		   && (parser = find_action_parser (acttab, tok->v.str)))
	    {
	      int rc;
	      PARSER_TABLES tab;

	      parser_tables_copy (tab, acttab, NULL, NULL);

	      rc = parser (in, tab, head, data);
	      if (rc == TMPL_ACTION_ERR
		  || (in->tok.type != TMPL_TOK_END && in->error == TMPL_ERR_OK))
		{
		  if (!in->error)
		    in->error = TMPL_ERR_BADTOK;
		  return in->error;
		}
	      if (rc == TMPL_ACTION_END)
		return TMPL_ERR_OK;
	    }
	  else
	    {
	      tmpl_input_putback (in);
	      act = tmpl_action_new (head, TMPL_ACT_PIPELINE);
	      scan_pipeline (in, &act->v.pipeline);
	      if (in->tok.type != TMPL_TOK_END)
		{
		  if (!in->error)
		    in->error = TMPL_ERR_BADTOK;
		  return in->error;
		}
	    }
	  break;

	default:
	  return in->error = TMPL_ERR_BADTOK;
	}
    }
  return TMPL_ERR_OK;
}

struct template
{
  char *name;
  TMPL_ACTION_LIST head;
  SLIST_ENTRY (template) next;
};

static SLIST_HEAD(,template) template_list = SLIST_HEAD_INITIALIZER (template_table);

TEMPLATE
template_lookup (const char *name)
{
  TEMPLATE t;
  SLIST_FOREACH (t, &template_list, next)
    {
      if (strcmp (t->name, name) == 0)
	return t;
    }
  return NULL;
}

static TEMPLATE
template_define (const char *name, TMPL_ACTION_LIST *head)
{
  TEMPLATE t = template_lookup (name);
  if (t)
    {
      /* Free previous definition */
      tmpl_action_list_free (&t->head);
    }
  else
    {
      XZALLOC (t);
      t->name = xstrdup (name);
      SLIST_PUSH (&template_list, t, next);
    }
  SLIST_COPY (&t->head, head);
  return t;
}

#if 0
//FIXME
TEMPLATE
template_install (TEMPLATE tmpl)
#endif

void
template_free (TEMPLATE tmpl)
{
  tmpl_action_list_free (&tmpl->head);
  free (tmpl);
}

int
template_parse (char *text, TEMPLATE *ret_tmpl, size_t *end)
{
  struct tmpl_input in;
  PARSER_TABLES top_tab = {
    [PT_GLOBAL] = top_parser_tab
  };
  struct template *tmpl;
  XZALLOC (tmpl);
  SLIST_INIT (&tmpl->head);

  tmpl_input_init (&in, text);
  scan_action (&in, top_tab, &tmpl->head, NULL);

  // Free tmpl on error
  if (in.error)
    template_free (tmpl);
  else
    *ret_tmpl = tmpl;
  if (end)
    *end = in.off;
  return in.error;
}

static void
tmpl_run_in_env (TEMPLATE tmpl, struct json_value *val, struct tmpl_env *top)
{
  struct tmpl_env *env;

  env = tmpl_env_new (val, 1, top);
  tmpl_action_eval (env, &tmpl->head);
  tmpl_env_free (env);
}

void
template_run (TEMPLATE tmpl, struct json_value *val, FILE *outfile)
{
  struct tmpl_env *env;

  env = tmpl_env_new (val, 0, NULL);
  env->outfile = outfile;
  tmpl_action_eval (env, &tmpl->head);
  tmpl_env_free (env);
}
