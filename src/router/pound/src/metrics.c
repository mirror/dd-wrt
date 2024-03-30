/* Openmetrics support for pound
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
#include "pound.h"
#include "extern.h"
#include "json.h"

/*
 * Metric labels
 */
struct metric_label
{
  char *name;
  char *value;
  DLIST_ENTRY (metric_label) next;
};

typedef DLIST_HEAD (,metric_label) METRIC_LABELS;

#define metric_labels_empty DLIST_EMPTY

/*
 * Add label NAME=VALUE to the label list HEAD.
 */
static int
metric_labels_add (METRIC_LABELS *head, char const *name, char const *value)
{
  struct metric_label *label;
  size_t nlen = strlen (name);

  if ((label = malloc (sizeof (*label) + nlen + strlen (value) + 2)) == NULL)
    {
      lognomem ();
      return -1;
    }
  label->name = (char*) (label + 1);
  strcpy (label->name, name);
  label->value = label->name + nlen + 1;
  strcpy (label->value, value);

  DLIST_PUSH (head, label, next);

  return 0;
}

/* Remove last label from the list. */
static void
metric_labels_pop (METRIC_LABELS *head)
{
  struct metric_label *label = DLIST_LAST (head);
  DLIST_REMOVE (head, label, next);
  free (label);
}

/* Copy labels from SRC to DST.  Notice: does not initialize or reset DST. */
static int
metric_labels_copy (METRIC_LABELS *dst, METRIC_LABELS const *src)
{
  if (src)
    {
      struct metric_label *label;
      DLIST_FOREACH (label, src, next)
	{
	  if (metric_labels_add (dst, label->name, label->value))
	    return -1;
	}
    }
  return 0;
}

/* Free the label list. */
static void
metric_labels_free (METRIC_LABELS *head)
{
  while (!DLIST_EMPTY (head))
    {
      struct metric_label *p = DLIST_FIRST (head);
      DLIST_SHIFT (head, next);
      free (p);
    }
}

/*
 * Metric samples.
 */
struct metric_sample
{
  METRIC_LABELS labels;
  double number;
  SLIST_ENTRY (metric_sample) next;
};

typedef SLIST_HEAD (,metric_sample) METRIC_SAMPLES;

/*
 * Allocate a new metric sample and append it to the samples list.
 * On success, return the new sample.
 * On error, log the failure and return NULL.
 */
static struct metric_sample *
metric_samples_add (METRIC_SAMPLES *samples)
{
  struct metric_sample *sp;

  if ((sp = malloc (sizeof (*sp))) != NULL)
    {
      DLIST_INIT (&sp->labels);
      sp->number = 0;
      SLIST_PUSH (samples, sp, next);
    }
  else
    lognomem ();
  return sp;
}

/* Free all samples in the list. */
static void
metric_samples_free (METRIC_SAMPLES *samples)
{
  while (!SLIST_EMPTY (samples))
    {
      struct metric_sample *s = SLIST_FIRST (samples);
      SLIST_SHIFT (samples, next);
      metric_labels_free (&s->labels);
      free (s);
    }
}

/*
 * Metrics.
 */
struct metric
{
  struct metric_family const *family;
  METRIC_SAMPLES samples;
  SLIST_ENTRY (metric) next;
};

/* Exposition is a list of metrics. */
typedef SLIST_HEAD (,metric) EXPOSITION;

#define EXPOSITION_INIT(exp) SLIST_INIT(exp)

/*
 * Allocate a new metric sample and append it to the sample list in
 * the metric.  Set sample labels to PFX.
 * Return the new sample or NULL on allocation error.  In the latter
 * case the partially initialized object might be left attached to
 * the metric.  Thus, it will be freed when the caller disposes of
 * the metric.
 */
static struct metric_sample *
metric_add_sample (struct metric *metric, METRIC_LABELS *pfx)
{
  struct metric_sample *samp = metric_samples_add (&metric->samples);
  if (samp)
    {
      if (metric_labels_copy (&samp->labels, pfx))
	return NULL;
    }
  return samp;
}

/* Free the metric and everything associated with it. */
static void
metric_free (struct metric *metric)
{
  metric_samples_free (&metric->samples);
}

/*
 * Find in the exposition a metric with the given family.  If not there,
 * allocate one and add to the exposition.
 * Return the metric.
 * On error, log the failure and return NULL.
 */
static struct metric *
exposition_metric_find (EXPOSITION *exp, struct metric_family const *family)
{
  struct metric *mp;

  SLIST_FOREACH (mp, exp, next)
    {
      if (mp->family == family)
	return mp;
    }

  if ((mp = malloc (sizeof (*mp))) != NULL)
    {
      mp->family = family;
      SLIST_INIT (&mp->samples);
      SLIST_PUSH (exp, mp, next);
    }
  else
    lognomem ();
  return mp;
}

/* Free the exposition. */
static void
exposition_free (EXPOSITION *exp)
{
  while (!SLIST_EMPTY (exp))
    {
      struct metric *mp = SLIST_FIRST (exp);
      SLIST_SHIFT (exp, next);
      metric_free (mp);
      free (mp);
    }
}

/*
 * Mertric families.
 */
struct metric_family
{
  char const *name;
  char const *type;
  char const *unit;
  char const *help;
  int (*setfn) (EXPOSITION *, struct metric *, METRIC_LABELS *, struct json_value *);
};

static int gen_listener_enabled (EXPOSITION *exp, struct metric *metric,
				 METRIC_LABELS *pfx, struct json_value *obj);
static int gen_listener_info (EXPOSITION *exp, struct metric *metric,
			      METRIC_LABELS *pfx, struct json_value *obj);
static int gen_backends_count (EXPOSITION *exp, struct metric *metric,
			       METRIC_LABELS *pfx, struct json_value *obj);
static int gen_service_info (EXPOSITION *exp, struct metric *metric,
			     METRIC_LABELS *pfx, struct json_value *obj);
static int gen_service_enabled (EXPOSITION *exp, struct metric *metric,
				METRIC_LABELS *pfx, struct json_value *obj);
static int gen_service_pri (EXPOSITION *exp, struct metric *metric,
			    METRIC_LABELS *pfx, struct json_value *obj);
static int gen_backend_state (EXPOSITION *exp, struct metric *metric,
			      METRIC_LABELS *pfx, struct json_value *obj);
static int gen_backend_requests (EXPOSITION *exp, struct metric *metric,
				 METRIC_LABELS *pfx, struct json_value *obj);
static int gen_backend_request_time_avg (EXPOSITION *exp, struct metric *metric,
				 METRIC_LABELS *pfx, struct json_value *obj);
static int gen_backend_request_stddev (EXPOSITION *exp, struct metric *metric,
				 METRIC_LABELS *pfx, struct json_value *obj);
static int gen_workers (EXPOSITION *exp, struct metric *metric,
			METRIC_LABELS *pfx, struct json_value *obj);

static struct metric_family listener_metric_families[] = {
  { "pound_listener_enabled",
    "stateset",
    NULL,
    "State of a listener: enabled/disabled.",
    gen_listener_enabled,
  },
  { "pound_listener_info",
    "info",
    NULL,
    "Description of a listener.",
    gen_listener_info },
  { NULL }
};

static struct metric_family service_metric_families[] = {
  { "pound_service_info",
    "info",
    NULL,
    "Description of a service.",
    gen_service_info },
  { "pound_service_enabled",
    "stateset",
    NULL,
    "State of a particular service.",
    gen_service_enabled },
  { "pound_service_pri",
    "gauge",
    NULL,
    "Service priority value.",
    gen_service_pri },
  { "pound_backends",
    "gauge",
    NULL,
    "Number of backends per service: total, alive, enabled, and active (both alive and enabled).",
    gen_backends_count,
  },
  { NULL }
};

static struct metric_family backend_metric_families[] = {
  { "pound_backend_state",
    "stateset",
    NULL,
    "Backend states: alive and enabled.",
    gen_backend_state },
  { "pound_backend_requests",
    "gauge",
    NULL,
    "Number of requests processed by backend,",
    gen_backend_requests },
  { "pound_backend_request_time_avg_nanoseconds",
    "gauge",
    "nanoseconds",
    "Average time per request spent in backend.",
    gen_backend_request_time_avg },
  { "pound_backend_request_stddev_nanoseconds",
    "gauge",
    "nanoseconds",
    "Standard deviation of the average time per request.",
    gen_backend_request_stddev },
  { NULL }
};

static struct metric_family workers_metric_families[] = {
  { "pound_workers",
    "gauge",
    NULL,
    "Number of pound workers.",
    gen_workers },
  { NULL }
};


/*
 * Metric family definitions describe how to iterate over the root
 * object and, if necessary, what family to descend into for each
 * object retrieved.
 */
struct metric_family_defn
{
  struct metric_family const *family;
  char const *attr;        /* JSON attribute that contains array of objects
			      of this family. */
  char const *index_label; /* Label to hold index of the current object. */
  int descend_family;      /* After handling each retrieved object, descend
			      into that family. */
};

/* The table below is indexed by these constants: */
enum
  {
    METRIC_FAMILY_NONE = -1,
    METRIC_FAMILY_LISTENER,
    METRIC_FAMILY_SERVICE,
    METRIC_FAMILY_BACKEND
  };

struct metric_family_defn metric_family_defn[] = {
  { listener_metric_families,
    "listeners",
    "listener",
    METRIC_FAMILY_SERVICE },
  { service_metric_families,
    "services",
    "service",
    METRIC_FAMILY_BACKEND },
  { backend_metric_families,
    "backends",
    "backend",
    METRIC_FAMILY_NONE },
  { NULL }
};

/*
 * Version of json_object_get with error and type checking.
 * Retrieves from the object OBJ the value of attribute ATTR.
 * If found, checks that it has the given TYPE, and if so,
 * stores it in the memory location pointed to by RETVAL and
 * returns 0.  On error (attribute not found, is of wrong type,
 * etc.) logs the failure and returns -1.
 *
 * When checking types, json_number and json_integer are considered
 * to be the same type.
 */
static int
json_object_get_type (struct json_value *obj, char const *attr, int type,
		      struct json_value **retval)
{
  struct json_value *jv;

  if (json_object_get (obj, attr, &jv))
    {
      if (errno == ENOENT)
	logmsg (LOG_NOTICE, "attribute \"%s\" not found", attr);
      else
	logmsg (LOG_NOTICE, "attribute lookup error: %s", strerror (errno));
      return -1;
    }

  switch (type)
    {
    case json_number:
    case json_integer:
      if (jv->type == json_number || jv->type == json_integer)
	{
	  *retval = jv;
	  return 0;
	}
      break;

    default:
      if (jv->type == type)
	{
	  *retval = jv;
	  return 0;
	}
    }
  logmsg (LOG_NOTICE, "attribute \"%s\" type mismatch (expected %d, got %d)",
	  attr, type, jv->type);
  errno = EINVAL;
  return -1;
}

static int
json_object_get_name (struct json_value *obj, struct json_value **retval)
{
  struct json_value *jv;

  if (json_object_get (obj, "name", &jv))
    {
      if (errno == ENOENT)
	logmsg (LOG_NOTICE, "attribute \"%s\" not found", "name");
      else
	logmsg (LOG_NOTICE, "attribute lookup error: %s", strerror (errno));
      return -1;
    }

  switch (jv->type)
    {
    case json_string:
    case json_null:
      *retval = jv;
      break;

    default:
      logmsg (LOG_NOTICE, "attribute \"%s\" type mismatch (expected %d or %d, got %d)",
	      "name", json_string, json_null, jv->type);
      errno = EINVAL;
      return -1;
    }

  return 0;
}

/*
 * For each entry in the FAMILY array, apply its setfn function to the metric
 * of this family in the exposition.  EXP, PFX and VAL are passed to setfn.
 *
 * Return 0 on success.
 * On error, log the failure and return -1.
 */
int
exposition_apply_family (EXPOSITION *exp,
			 METRIC_LABELS *pfx,
			 struct metric_family const *family,
			 struct json_value *val)
{
  int res = 0;
  for (; family->name; family++)
    {
      static struct metric *metric;

      if ((metric = exposition_metric_find (exp, family)) == NULL)
	{
	  logmsg (LOG_ERR, "INTERNAL ERROR at %s:%d: metric family not found; please report", __FILE__, __LINE__);
	  res = -1;
	  break;
	}

      if ((res = family->setfn (exp, metric, pfx, val)) != 0)
	break;
    }
  return res;
}

/*
 * Iterate over each value in OBJ using metric family definition at
 * index FN.
 *
 * Return 0 on success.
 * On error, log the failure and return -1.
 */
int
exposition_iterate (EXPOSITION *exp, METRIC_LABELS const *pfx, int fn,
		    struct json_value *obj)
{
  struct metric_family_defn const *defn = metric_family_defn + fn;
  struct json_value *arr, *jv;
  size_t i, n;
  char nbuf[80];
  METRIC_LABELS pfxcopy;
  int res;

  if (json_object_get (obj, defn->attr, &arr))
    {
      if (errno == ENOENT)
	return 0;
      else
	{
	  logmsg (LOG_NOTICE, "attribute lookup error: %s", strerror (errno));
	  return -1;
	}
    }
  if (arr->type != json_array)
    {
      logmsg (LOG_NOTICE, "attribute \"%s\" type mismatch (expected %d, got %d)",
	      defn->attr, json_array, arr->type);
      return -1;
    }

  DLIST_INIT (&pfxcopy);
  metric_labels_copy (&pfxcopy, pfx);

  res = 0;
  n = json_array_length (arr);
  for (i = 0; i < n; i++)
    {
      jv = arr->v.a->ov[i];
      snprintf (nbuf, sizeof nbuf, "%zu", i);
      if ((res = metric_labels_add (&pfxcopy, defn->index_label, nbuf)) != 0)
	break;
      if ((res = exposition_apply_family (exp, &pfxcopy, defn->family, jv)) != 0)
	break;
      if (defn->descend_family != METRIC_FAMILY_NONE)
	{
	  if ((res = exposition_iterate (exp, &pfxcopy, defn->descend_family, jv)) != 0)
	    break;
	}
      metric_labels_pop (&pfxcopy);
    }
  metric_labels_free (&pfxcopy);
  return res;
}

static int
gen_listener_enabled (EXPOSITION *exp, struct metric *metric,
		      METRIC_LABELS *pfx, struct json_value *obj)
{
  struct json_value *jv;
  struct metric_sample *samp;

  if (json_object_get_type (obj, "enabled", json_bool, &jv))
    return -1;

  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  samp->number = jv->v.b;
  return 0;
}

static int
gen_listener_info (EXPOSITION *exp, struct metric *metric,
		   METRIC_LABELS *pfx, struct json_value *obj)
{
  struct metric_sample *samp;
  struct json_value *jv;

  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;

  if (json_object_get_name (obj, &jv))
    return -1;
  if (metric_labels_add (&samp->labels, "name",
			 jv->type == json_string ? jv->v.s : ""))
    return -1;

  if (json_object_get_type (obj, "address", json_string, &jv))
    return -1;
  metric_labels_add (&samp->labels, "address", jv->v.s);
  if (json_object_get_type (obj, "protocol", json_string, &jv))
    return -1;
  if (metric_labels_add (&samp->labels, "protocol", jv->v.s))
    return -1;
  samp->number = 1;
  return 0;
}

static int
gen_backends_count (EXPOSITION *exp, struct metric *metric,
		    METRIC_LABELS *pfx, struct json_value *obj)
{
  struct metric_sample *samp;
  struct json_value *arr, *jv;
  size_t n_alive = 0;
  size_t n_enabled = 0;
  size_t n_active = 0;
  size_t i, n;

  if (json_object_get_type (obj, "backends", json_array, &arr))
    return -1;

  n = json_array_length (arr);
  for (i = 0; i < n; i++)
    {
      int enabled = 0;

      jv = arr->v.a->ov[i];

      if (json_object_get_type (arr->v.a->ov[i], "enabled", json_bool, &jv) == 0 &&
	  (enabled = jv->v.b) != 0)
	n_enabled++;

      if (json_object_get_type (arr->v.a->ov[i], "alive", json_bool, &jv) == 0 &&
	  jv->v.b)
	{
	  n_alive++;
	  if (enabled)
	    n_active++;
	}
    }

  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "state", "total"))
    return -1;
  samp->number = n;

  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "state", "enabled"))
    return -1;
  samp->number = n_enabled;

  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "state", "alive"))
    return -1;
  samp->number = n_alive;

  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "state", "active"))
    return -1;
  samp->number = n_active;
  return 0;
}

static int
gen_service_info (EXPOSITION *exp, struct metric *metric,
		  METRIC_LABELS *pfx, struct json_value *obj)
{
  struct metric_sample *samp;
  struct json_value *jv;

  if (json_object_get_name (obj, &jv))
    return -1;
  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "name",
			 jv->type == json_string ? jv->v.s : ""))
    return -1;
  samp->number = 1;
  return 0;
}

static int
gen_service_enabled (EXPOSITION *exp, struct metric *metric,
		     METRIC_LABELS *pfx, struct json_value *obj)
{
  struct metric_sample *samp;
  struct json_value *jv;

  if (json_object_get_type (obj, "enabled", json_bool, &jv))
    return -1;
  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  samp->number = jv->v.b;
  return 0;
}

static int
gen_service_pri (EXPOSITION *exp, struct metric *metric,
		 METRIC_LABELS *pfx, struct json_value *obj)
{
  struct metric_sample *samp;
  struct json_value *jv;

  if (json_object_get_type (obj, "tot_pri", json_integer, &jv))
    return -1;
  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "entity", "total"))
    return -1;
  samp->number = jv->v.n;

  if (json_object_get_type (obj, "abs_pri", json_integer, &jv))
    return -1;
  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "entity", "absolute"))
    return -1;
  samp->number = jv->v.n;

  return 0;
}

static int
gen_backend_state (EXPOSITION *exp, struct metric *metric,
		   METRIC_LABELS *pfx, struct json_value *obj)
{
  struct metric_sample *samp;
  struct json_value *jv;

  if (json_object_get_type (obj, "alive", json_bool, &jv))
    return -1;
  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "state", "alive"))
    return -1;
  samp->number = jv->v.b;

  if (json_object_get_type (obj, "enabled", json_bool, &jv))
    return -1;
  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  if (metric_labels_add (&samp->labels, "state", "enabled"))
    return -1;
  samp->number = jv->v.b;

  return 0;
}

static int
gen_backend_stats (EXPOSITION *exp, struct metric *metric,
		   METRIC_LABELS *pfx, struct json_value *obj, char const *attr)
{
  struct metric_sample *samp;
  struct json_value *stats, *jv;

  if (json_object_get (obj, "stats", &stats))
    {
      if (errno == ENOENT)
	return 0;
      else
	{
	  logmsg (LOG_NOTICE, "attribute lookup error: %s", strerror (errno));
	  return -1;
	}
    }
  if (stats->type != json_object)
    {
      logmsg (LOG_NOTICE, "attribute \"%s\" type mismatch (expected %d, got %d)",
	      "stats", json_object, jv->type);
      return -1;
    }

  if (json_object_get (stats, attr, &jv))
    {
      if (errno == ENOENT)
	return 0;
      else
	{
	  logmsg (LOG_NOTICE, "attribute lookup error: %s", strerror (errno));
	  return -1;
	}
    }
  if (jv->type != json_number)
    {
      logmsg (LOG_NOTICE, "stats: attribute \"%s\" type mismatch (expected %d, got %d)",
	      attr, json_number, jv->type);
      return -1;
    }

  if ((samp = metric_add_sample (metric, pfx)) == NULL)
    return -1;
  samp->number = jv->v.n;

  return 0;
}

static int
gen_backend_requests (EXPOSITION *exp, struct metric *metric,
		      METRIC_LABELS *pfx, struct json_value *obj)
{
  return gen_backend_stats (exp, metric, pfx, obj, "request_count");
}

static int
gen_backend_request_time_avg (EXPOSITION *exp, struct metric *metric,
			      METRIC_LABELS *pfx, struct json_value *obj)
{
  return gen_backend_stats (exp, metric, pfx, obj, "request_time_avg");
}

static int
gen_backend_request_stddev (EXPOSITION *exp, struct metric *metric,
			    METRIC_LABELS *pfx, struct json_value *obj)
{
  return gen_backend_stats (exp, metric, pfx, obj, "request_time_stddev");
}

static int
gen_workers (EXPOSITION *exp, struct metric *metric,
	     METRIC_LABELS *pfx, struct json_value *obj)
{
  char *attr[] = { "active", "count", "max", "min", NULL };
  int i;

  for (i = 0; attr[i]; i++)
    {
      struct json_value *val;
      struct metric_sample *samp;

      if (json_object_get_type (obj, attr[i], json_number, &val))
	return -1;
      if ((samp = metric_add_sample (metric, pfx)) == NULL)
	return -1;
      if (metric_labels_add (&samp->labels, "type", attr[i]))
	return -1;
      samp->number = val->v.n;
    }
  return 0;
}

/*
 * Initialize the exposition and fill it, using OBJ as input.
 */
static int
exposition_fill (EXPOSITION *exp, struct json_value *obj)
{
  struct json_value *val;

  EXPOSITION_INIT (exp);

  if (json_object_get_type (obj, "workers", json_object, &val))
    return -1;

  if (exposition_apply_family (exp, NULL, workers_metric_families, val))
    return -1;

  if (exposition_iterate (exp, NULL, METRIC_FAMILY_LISTENER, obj))
    return -1;

  return exposition_iterate (exp, NULL, METRIC_FAMILY_SERVICE, obj);
}

/*
 * Print the exposition in Openmetrics format into stringbuf.
 */
static char *
exposition_format (struct stringbuf *sb, EXPOSITION *exp)
{
  struct metric *metric;

  SLIST_FOREACH (metric, exp, next)
    {
      if (!SLIST_EMPTY (&metric->samples))
	{
	  struct metric_sample *samp;
	  struct metric_family const *family = metric->family;

	  stringbuf_printf (sb, "# TYPE %s %s\n",
			    family->name, family->type);
	  if (family->unit)
	    stringbuf_printf (sb, "# UNIT %s %s\n", family->name,
			      family->unit);
	  stringbuf_printf (sb, "# HELP %s %s\n", family->name, family->help);

	  SLIST_FOREACH (samp, &metric->samples, next)
	    {
	      stringbuf_add_string (sb, family->name);
	      if (!metric_labels_empty (&samp->labels))
		{
		  struct metric_label *label;

		  stringbuf_add_char (sb, '{');
		  DLIST_FOREACH (label, &samp->labels, next)
		    {
		      stringbuf_add_string (sb, label->name);
		      stringbuf_add_char (sb, '=');
		      stringbuf_add_char (sb, '"');
		      //FIXME: escape value
		      stringbuf_add_string (sb, label->value);
		      stringbuf_add_char (sb, '"');
		      if (DLIST_NEXT (label, next))
			stringbuf_add_char (sb, ',');
		    }
		  stringbuf_add_char (sb, '}');
		}
	      stringbuf_add_char (sb, ' ');
	      stringbuf_printf (sb, "%.0f\n", samp->number);
	    }
	}
    }
  stringbuf_add_string (sb, "# EOF\n");
  return stringbuf_finish (sb);
}

static int
send_reply (POUND_HTTP *phttp, char const *content)
{
  BIO_printf (phttp->cl,
	      "HTTP/1.%d %d %s\r\n"
	      "Content-Type: application/openmetrics-text; version=1.0.0; charset=utf-8\r\n"
	      "Content-Length: %"PRICLEN"\r\n"
	      "\r\n"
	      "%s",
	      phttp->request.version,
	      200, "OK", (CONTENT_LENGTH) strlen (content), content);
  BIO_flush (phttp->cl);
  return 0;
}

int
metrics_response (POUND_HTTP *phttp)
{
  struct json_value *obj;
  EXPOSITION exp;
  int res;

  if ((obj = pound_serialize ()) == NULL)
    return -1;

  EXPOSITION_INIT (&exp);

  if ((res = exposition_fill (&exp, obj)) == 0)
    {
      struct stringbuf sb;
      char *content;

      stringbuf_init_log (&sb);
      if ((content = exposition_format (&sb, &exp)) == NULL)
	res = -1;
      else
	res = send_reply (phttp, content);
      stringbuf_free (&sb);
    }
  exposition_free (&exp);
  json_value_free (obj);
  return res == 0 ? 0 : HTTP_STATUS_INTERNAL_SERVER_ERROR;
}
