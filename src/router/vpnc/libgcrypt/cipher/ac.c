/* ac.c - Alternative interface for asymmetric cryptography.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
 
   This file is part of Libgcrypt.
  
   Libgcrypt is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser general Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
  
   Libgcrypt is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.
  
   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "g10lib.h"
#include "cipher.h"



/* At the moment the ac interface is a wrapper around the pk
   interface, but this might change somewhen in the future, depending
   on how much people prefer the ac interface.  */

/* Mapping of flag numbers to the according strings as it is expected
   for S-expressions.  */
struct number_string
{
  int number;
  const char *string;
} gcry_ac_flags[] =
  {
    { GCRY_AC_FLAG_NO_BLINDING, "no-blinding" },
    { 0, NULL },
  };

/* The positions in this list correspond to the values contained in
   the gcry_ac_key_type_t enumeration list.  */
static const char *ac_key_identifiers[] =
  {
    "private-key",
    "public-key",
  };

/* These specifications are needed for key-pair generation; the caller
   is allowed to pass additional, algorithm-specific `specs' to
   gcry_ac_key_pair_generate.  This list is used for decoding the
   provided values according to the selected algorithm.  */
struct gcry_ac_key_generate_spec
{
  int algorithm;		/* Algorithm for which this flag is
				   relevant.  */
  const char *name;		/* Name of this flag.  */
  size_t offset;		/* Offset in the cipher-specific spec
				   structure at which the MPI value
				   associated with this flag is to be
				   found.  */
} gcry_ac_key_generate_specs[] =
  {
    { GCRY_AC_RSA, "rsa-use-e", offsetof (gcry_ac_key_spec_rsa_t, e) },
    { 0 },
  };

/* Handle structure.  */
struct gcry_ac_handle
{
  int algorithm;		/* Algorithm ID associated with this
				   handle.  */
  const char *algorithm_name;	/* Name of the algorithm.  */
  unsigned int flags;		/* Flags, not used yet.  */
  gcry_module_t module;	        /* Reference to the algorithm
				   module.  */
};

/* A named MPI value.  */
typedef struct gcry_ac_mpi
{
  const char *name;		/* Name of MPI value. */
  gcry_mpi_t mpi;		/* MPI value.         */
  unsigned int flags;		/* Flags.             */
} gcry_ac_mpi_t;

/* A data set, that is simply a list of named MPI values.  */
struct gcry_ac_data
{
  gcry_ac_mpi_t *data;		/* List of named values.      */
  unsigned int data_n;		/* Number of values in DATA.  */
};

/* The key in `native' ac form and as an S-expression. */
struct gcry_ac_key
{
  gcry_ac_data_t data;		/* Data in native ac structure.  */
  gcry_sexp_t data_sexp;	/* Data as an S-expression.      */
  gcry_ac_key_type_t type;	/* Type of the key.              */
};

/* Two keys.  */
struct gcry_ac_key_pair
{
  gcry_ac_key_t public;
  gcry_ac_key_t secret;
};



/*
 * Primitive functions for the manipulation of `data sets'.
 */

/* Create a copy of the data set DATA and store it in DATA_CP.  */
static gcry_err_code_t
gcry_ac_data_copy_internal (gcry_ac_data_t *data_cp, gcry_ac_data_t data)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_ac_data_t data_new;
  void *p = NULL;
  int i = 0;

  /* Allocate data set.  */
  err = _gcry_malloc (sizeof (struct gcry_ac_data), 0, &p);
  data_new = p;
  if (! err)
    data_new->data_n = data->data_n;

  if (! err)
    /* Allocate space for named MPIs.  */
    err = _gcry_malloc (sizeof (gcry_ac_mpi_t) * data->data_n, 0,
			(void **) &data_new->data);

  if (! err)
    {
      /* Copy named MPIs.  */
      
      for (i = 0; i < data_new->data_n && (! err); i++)
	{
	  data_new->data[i].name = NULL;
	  data_new->data[i].mpi = NULL;

	  /* Name.  */
	  data_new->data[i].name = gcry_strdup (data->data[i].name);
	  if (! data_new->data[i].name)
	    err = gpg_err_code_from_errno (errno);

	  if (! err)
	    {
	      /* MPI.  */
	      data_new->data[i].mpi = gcry_mpi_copy (data->data[i].mpi);
	      if (! data_new->data[i].mpi)
		err = gpg_err_code_from_errno (errno);
	    }
	}
    }

  if (! err)
    {
      /* Copy out.  */
      *data_cp = data_new;
    }
  else
    {
      /* Deallocate resources.  */
      if (data_new)
	{
	  if (data_new->data)
	    {
	      for (; i >= 0; i--)
		{
		  if (data_new->data[i].name)
		    free ((void *) data_new->data[i].name);
		  if (data_new->data[i].mpi)
		    gcry_mpi_release (data_new->data[i].mpi);
		}
	      gcry_free (data_new->data);
	    }
	  gcry_free (data_new);
	}
    }

  return err;
}



/* 
 * Functions for converting data between the native ac and the
 * S-expression structure.
 */

/* Extract the S-Expression DATA_SEXP into DATA under the control of
   TYPE and NAME.  This function assumes that S-Expressions are of the
   following structure:

     (IDENTIFIER <data to be ignored>
                 (ALGORITHM <list of named MPI values>))

  IDENTIFIER is one of `private-key', `public-key', `enc-val',
  `sig-val'; ALGORITHM is the name of the algorithm used.  */
static gcry_err_code_t
gcry_ac_data_extract (const char *identifier, const char *algorithm,
		      gcry_sexp_t data_sexp, gcry_ac_data_t *data)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_sexp_t data_element_sexp = NULL;
  gcry_sexp_t inner_data_sexp = NULL;
  size_t inner_data_n;

  const char *name;
  size_t name_n;

  gcry_mpi_t data_elem_mpi = NULL;
  char *data_elem_name = NULL;

  gcry_ac_data_t data_new = NULL;

  int i = 0;

  /* Verify that the S-expression contains the correct identifier.  */
  name = gcry_sexp_nth_data (data_sexp, 0, &name_n);
  if (! name)
    err = GPG_ERR_INTERNAL;
  else if (strncmp (identifier, name, name_n))
    err = GPG_ERR_INTERNAL;

  if (! err)
    {
      /* Extract inner S-expression.  */
      inner_data_sexp = gcry_sexp_find_token (data_sexp, algorithm, 0);
      if (! inner_data_sexp)
	err = GPG_ERR_INTERNAL;
      else
	/* Count data elements, this includes the name of the
	   algorithm.  */
	inner_data_n = gcry_sexp_length (inner_data_sexp);
    }

  if (! err)
    {
      /* Allocate new data set.  */
      data_new = gcry_malloc (sizeof (struct gcry_ac_data));
      if (! data_new)
	err = gpg_err_code_from_errno (errno);
      else
	{
	  data_new->data = gcry_malloc (sizeof (gcry_ac_mpi_t)
                                        * (inner_data_n - 1));
	  if (! data_new->data)
	    err = gpg_err_code_from_errno (errno);
	}
    }

  if (! err)
    {
      /* Iterate through list of data elements and add them to the
	 data set.  */

      for (i = 1; i < inner_data_n; i++)
	{
	  data_new->data[i - 1].name = NULL;
	  data_new->data[i - 1].mpi = NULL;

	  /* Get the S-expression of the named MPI, that contains the
	     name and the MPI value.  */
	  data_element_sexp = gcry_sexp_nth (inner_data_sexp, i);
	  if (! data_element_sexp)
	    err = GPG_ERR_INTERNAL;

	  if (! err)
	    {
	      /* Extract the name.  */
	      name = gcry_sexp_nth_data (data_element_sexp, 0, &name_n);
	      if (! name)
		err = GPG_ERR_INTERNAL;
	    }

	  if (! err)
	    {
	      /* Extract the MPI value.  */
	      data_elem_mpi = gcry_sexp_nth_mpi (data_element_sexp, 1,
						 GCRYMPI_FMT_USG);
	      if (! data_elem_mpi)
		err = GPG_ERR_INTERNAL;
	    }

	  if (! err)
	    {
	      /* Duplicate the name.  */
	      data_elem_name = gcry_malloc (name_n + 1);
	      if (! data_elem_name)
		
		err = gpg_err_code_from_errno (errno);
	      else
		{
		  strncpy (data_elem_name, name, name_n);
		  data_elem_name[name_n] = 0;
		}
	    }

	  /* Done.  */

	  if (data_element_sexp)
	    gcry_sexp_release (data_element_sexp);

	  if (! err)
	    {
	      data_new->data[i - 1].name = data_elem_name;
	      data_new->data[i - 1].mpi = data_elem_mpi;
	    }
	  else
	    break;
	}
    }

  if (! err)
    {
      /* Copy out.  */
      data_new->data_n = inner_data_n - 1;
      *data = data_new;
    }
  else
    {
      /* Deallocate resources.  */

      if (data_new)
	{
	  if (data_new->data)
	    {
	      int j;
	     
	      for (j = 0; j < i - 1; j++)
		{
		  if (data_new->data[j].name)
		    gcry_free ((void *) data_new->data[j].name);
		  if (data_new->data[j].mpi)
		    gcry_mpi_release (data_new->data[j].mpi);
		}

	      gcry_free (data_new->data);
	    }
	  gcry_free (data_new);
	}
    }

  return err;
}

/* Construct an S-expression from the DATA and store it in
   DATA_SEXP. The S-expression will be of the following structure:

     (IDENTIFIER [(flags [...])]
                 (ALGORITHM <list of named MPI values>))  */
static gcry_err_code_t
gcry_ac_data_construct (const char *identifier, int include_flags,
			unsigned int flags, const char *algorithm,
			gcry_ac_data_t data, gcry_sexp_t *data_sexp)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  void **arg_list = NULL;

  gcry_sexp_t data_sexp_new = NULL;

  size_t data_format_n = 0;
  char *data_format = NULL;

  int i;

  /* We build a list of arguments to pass to
     gcry_sexp_build_array().  */
  arg_list = gcry_malloc (sizeof (void *) * data->data_n);
  if (! arg_list)
    err = gpg_err_code_from_errno (errno);
  else
    /* Fill list with MPIs.  */
    for (i = 0; i < data->data_n; i++)
      arg_list[i] = (void *) &data->data[i].mpi;

  if (! err)
    {
      /* Calculate size of format string.  */

      data_format_n = (5 + (include_flags ? 7 : 0)
                       + strlen (identifier) + strlen (algorithm));

      for (i = 0; i < data->data_n; i++)
        {
          /* Per-element sizes.  */
          data_format_n += 4 + strlen (data->data[i].name);
        }

      if (include_flags)
        {
          /* Add flags.  */
          for (i = 0; gcry_ac_flags[i].number; i++)
            if (flags & gcry_ac_flags[i].number)
              data_format_n += strlen (gcry_ac_flags[i].string) + 1;
        }

      /* Done.  */
      data_format = gcry_malloc (data_format_n);
      if (! data_format)
	err = gpg_err_code_from_errno (errno);
    }

  if (! err)
    {
      /* Construct the format string.  */

      *data_format = 0;
      strcat (data_format, "(");
      strcat (data_format, identifier);
      if (include_flags)
	{
	  strcat (data_format, "(flags");
	  for (i = 0; gcry_ac_flags[i].number; i++)
	    if (flags & gcry_ac_flags[i].number)
	      {
		strcat (data_format, " ");
		strcat (data_format, gcry_ac_flags[i].string);
	      }
	  strcat (data_format, ")");
	}
      strcat (data_format, "(");
      strcat (data_format, algorithm);
      for (i = 0; i < data->data_n; i++)
	{
	  strcat (data_format, "(");
	  strcat (data_format, data->data[i].name);
	  strcat (data_format, "%m)");
	}
      strcat (data_format, "))");

      /* Create final S-expression.  */
      err = gcry_sexp_build_array (&data_sexp_new, NULL,
				   data_format, arg_list);
    }

  if (err)
    {
      /* Deallocate resources.  */

      if (arg_list)
	gcry_free (arg_list);
      if (data_format)
	gcry_free (data_format);
      if (data_sexp_new)
	gcry_sexp_release (data_sexp_new);
    }

  else
    /* Copy-out.  */
    *data_sexp = data_sexp_new;

  return err;
}



/* 
 * Functions for working with data sets.
 */

/* Creates a new, empty data set and stores it in DATA.  */
gcry_error_t
gcry_ac_data_new (gcry_ac_data_t *data)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_ac_data_t data_new = NULL;

  data_new = gcry_malloc (sizeof (struct gcry_ac_data));
  if (! data_new)
    err = gpg_err_code_from_errno (errno);

  if (! err)
    {
      data_new->data = NULL;
      data_new->data_n = 0;
      *data = data_new;
    }

  return gcry_error (err);
}

/* Destroys the data set DATA.  */
void
gcry_ac_data_destroy (gcry_ac_data_t data)
{
  int i;

  for (i = 0; i < data->data_n; i++)
    {
      gcry_free ((void *) data->data[i].name);
      gcry_mpi_release (data->data[i].mpi);
    }
  gcry_free (data->data);
  gcry_free (data);
}

/* Add the value MPI to DATA with the label NAME.  If FLAGS contains
   GCRY_AC_FLAG_DATA_COPY, the data set will contain copies of NAME
   and MPI.  If FLAGS contains GCRY_AC_FLAG_DATA_DEALLOC or
   GCRY_AC_FLAG_DATA_COPY, the values contained in the data set will
   be deallocated when they are to be removed from the data set.  */
gcry_error_t
gcry_ac_data_set (gcry_ac_data_t data, unsigned int flags,
		  const char *name, gcry_mpi_t mpi)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_ac_mpi_t *ac_mpi = NULL;
  gcry_mpi_t mpi_add = NULL;
  char *name_add = NULL;
  unsigned int i = 0;

  if (flags & ~(GCRY_AC_FLAG_DEALLOC | GCRY_AC_FLAG_COPY))
    err = GPG_ERR_INV_ARG;
  else
    {
      if (flags & GCRY_AC_FLAG_COPY)
	{
	  /* Create copies.  */

	  name_add = gcry_strdup (name);
	  if (! name_add)
	    err = GPG_ERR_ENOMEM;
	  if (! err)
	    {
	      mpi_add = gcry_mpi_copy (mpi);
	      if (! mpi_add)
		err = GPG_ERR_ENOMEM;
	    }
	}
      else
	{
	  name_add = (char *) name;
	  mpi_add = mpi;
	}
      
      /* Search for existing entry.  */
      for (i = 0; (i < data->data_n) && (! ac_mpi); i++)
	if (! strcmp (name, data->data[i].name))
	  ac_mpi = data->data + i;
      
      if (ac_mpi)
	{
	  /* An entry for NAME does already exist.  */
	  if (ac_mpi->flags & GCRY_AC_FLAG_DEALLOC)
	    {
	      /* Deallocate old values.  */
	      gcry_free ((char *) ac_mpi->name);
	      gcry_mpi_release (ac_mpi->mpi);
	    }
	}
      else
	{
	  /* Create a new entry.  */
	  
	  gcry_ac_mpi_t *ac_mpis = NULL;
	  
	  ac_mpis = realloc (data->data, sizeof (*data->data) * (data->data_n + 1));
	  if (! ac_mpis)
	    err = gpg_err_code_from_errno (errno);
	  
	  if (data->data != ac_mpis)
	    data->data = ac_mpis;
	  ac_mpi = data->data + data->data_n;
	  data->data_n++;
	}
      
      ac_mpi->flags = flags;
      ac_mpi->name = name_add;
      ac_mpi->mpi = mpi_add;
    }

  return gcry_error (err);
}

/* Create a copy of the data set DATA and store it in DATA_CP.  */
gcry_error_t
gcry_ac_data_copy (gcry_ac_data_t *data_cp, gcry_ac_data_t data)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  err = gcry_ac_data_copy_internal (data_cp, data);

  return gcry_error (err);
}

/* Returns the number of named MPI values inside of the data set
   DATA.  */
unsigned int
gcry_ac_data_length (gcry_ac_data_t data)
{
  return data->data_n;
}

/* Store the value labelled with NAME found in DATA in MPI.  If FLAGS
   contains GCRY_AC_FLAG_COPY, store a copy of the MPI value contained
   in the data set.  MPI may be NULL.  */
gcry_error_t
gcry_ac_data_get_name (gcry_ac_data_t data, unsigned int flags,
		       const char *name, gcry_mpi_t *mpi)
{
  gcry_err_code_t err = GPG_ERR_NO_DATA;
  gcry_mpi_t mpi_found = NULL;
  unsigned int i = 0;

  if (flags & ~(GCRY_AC_FLAG_COPY))
    err = GPG_ERR_INV_ARG;
  else
    {
      for (i = 0; i < data->data_n && (! mpi_found); i++)
	if (! strcmp (data->data[i].name, name))
	  {
	    if (flags & GCRY_AC_FLAG_COPY)
	      {
		mpi_found = gcry_mpi_copy (data->data[i].mpi);
		if (! mpi_found)
		  err = GPG_ERR_ENOMEM;
	      }
	    else
	      mpi_found = data->data[i].mpi;
	    
	    if (mpi_found)
	      err = GPG_ERR_NO_ERROR;
	  }
    }

  if (! err)
    if (mpi)
      *mpi = mpi_found;

  return gcry_error (err);
}

/* Stores in NAME and MPI the named MPI value contained in the data
   set DATA with the index IDX.  If FLAGS contains GCRY_AC_FLAG_COPY,
   store copies of the values contained in the data set. NAME or MPI
   may be NULL.  */
gcry_error_t
gcry_ac_data_get_index (gcry_ac_data_t data, unsigned int flags, unsigned int idx,
			const char **name, gcry_mpi_t *mpi)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_mpi_t mpi_return = NULL;
  char *name_return = NULL;

  if (flags & ~(GCRY_AC_FLAG_COPY))
    err = GPG_ERR_INV_ARG;
  else
    {
      if (idx < data->data_n)
	{
	  if (flags & GCRY_AC_FLAG_COPY)
	    {
	      /* Return copies to the user.  */
	      if (name)
		name_return = gcry_strdup (data->data[idx].name);
	      if (mpi)
		mpi_return = gcry_mpi_copy (data->data[idx].mpi);
	      
	      if (! (name_return && mpi_return))
		{
		  if (name_return)
		    free (name_return);
		  if (mpi_return)
		    gcry_mpi_release (mpi_return);
		  err = GPG_ERR_ENOMEM;
		}
	    }
	  else
	    {
	      name_return = (char *) data->data[idx].name;
	      mpi_return = data->data[idx].mpi;
	    }
	}
      else
	err = GPG_ERR_NO_DATA;
    }

  if (! err)
    {
      if (name)
	*name = name_return;
      if (mpi)
	*mpi = mpi_return;
    }

  return gcry_error (err);
}

/* Destroys any values contained in the data set DATA.  */
void
gcry_ac_data_clear (gcry_ac_data_t data)
{
  gcry_free (data->data);
  data->data = NULL;
  data->data_n = 0;
}



/*
 * Handle management.
 */

/* Creates a new handle for the algorithm ALGORITHM and store it in
   HANDLE.  FLAGS is not used yet.  */
gcry_error_t
gcry_ac_open (gcry_ac_handle_t *handle,
	      gcry_ac_id_t algorithm, unsigned int flags)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_module_t module = NULL;
  gcry_ac_handle_t handle_new;
  const char *algorithm_name;

  *handle = NULL;

  /* Get name.  */
  algorithm_name = _gcry_pk_aliased_algo_name (algorithm);
  if (! algorithm_name)
    err = GPG_ERR_PUBKEY_ALGO;

  if (! err)  /* Acquire reference to the pubkey module.  */
    err = _gcry_pk_module_lookup (algorithm, &module);
  
  if (! err)
    {
      /* Allocate.  */
      handle_new = gcry_malloc (sizeof (struct gcry_ac_handle));
      if (! handle_new)
	err = gpg_err_code_from_errno (errno);
    }

  if (! err)
    {
      /* Done.  */
      handle_new->algorithm = algorithm;
      handle_new->algorithm_name = algorithm_name;
      handle_new->flags = flags;
      handle_new->module = module;
      *handle = handle_new;
    }
  else
    {
      /* Deallocate resources.  */
      if (module)
	_gcry_pk_module_release (module);
    }

  return gcry_error (err);
}

/* Destroys the handle HANDLE.  */
void
gcry_ac_close (gcry_ac_handle_t handle)
{
  /* Release reference to pubkey module.  */
  if (handle)
    {
      _gcry_pk_module_release (handle->module);
      gcry_free (handle);
    }
}



/* 
 * Key management.
 */

/* Creates a new key of type TYPE, consisting of the MPI values
   contained in the data set DATA and stores it in KEY.  */
gcry_error_t
gcry_ac_key_init (gcry_ac_key_t *key,
		  gcry_ac_handle_t handle,
		  gcry_ac_key_type_t type,
		  gcry_ac_data_t data)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_ac_data_t data_new = NULL;
  gcry_sexp_t data_sexp = NULL;
  gcry_ac_key_t key_new = NULL;

  /* Allocate.  */
  key_new = gcry_malloc (sizeof (struct gcry_ac_key));
  if (! key_new)
    err = gpg_err_code_from_errno (errno);

  if (! err)
    {
      /* Create S-expression from data set.  */
      err = gcry_ac_data_construct (ac_key_identifiers[type], 0, 0,
                                    handle->algorithm_name, data, &data_sexp);
    }

  if (! err)
    {
      /* Copy data set.  */
      err = gcry_ac_data_copy_internal (&data_new, data);
    }

  if (! err)
    {
      /* Done.  */
      key_new->data_sexp = data_sexp;
      key_new->data = data_new;
      key_new->type = type;
      *key = key_new;
    }
  else
    {
      /* Deallocate resources.  */
      if (key_new)
	gcry_free (key_new);
      if (data_sexp)
	gcry_sexp_release (data_sexp);
    }

  return gcry_error (err);
}

/* Generates a new key pair via the handle HANDLE of NBITS bits and
   stores it in KEY_PAIR.  In case non-standard settings are wanted, a
   pointer to a structure of type gcry_ac_key_spec_<algorithm>_t,
   matching the selected algorithm, can be given as KEY_SPEC.
   MISC_DATA is not used yet.  */
gcry_error_t
gcry_ac_key_pair_generate (gcry_ac_handle_t handle, unsigned int nbits, void *key_spec,
			   gcry_ac_key_pair_t *key_pair, gcry_mpi_t **misc_data)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  gcry_ac_key_pair_t key_pair_new = NULL;

  gcry_sexp_t genkey_sexp_request = NULL;
  gcry_sexp_t genkey_sexp_reply = NULL;

  char *genkey_format = NULL;
  size_t genkey_format_n = 0;

  void **arg_list = NULL;
  size_t arg_list_n = 0;

  unsigned int i = 0;

  /* Allocate key pair.  */
  key_pair_new = gcry_malloc (sizeof (struct gcry_ac_key_pair));
  if (! key_pair_new)
    err = gpg_err_code_from_errno (errno);

  if (! err)
    {
      /* Allocate keys.  */
      key_pair_new->secret = gcry_malloc (sizeof (struct gcry_ac_key));
      key_pair_new->public = gcry_malloc (sizeof (struct gcry_ac_key));

      if (! (key_pair_new->secret || key_pair_new->public))
 	err = gpg_err_code_from_errno (errno);
      else
	{
	  key_pair_new->secret->type = GCRY_AC_KEY_SECRET;
	  key_pair_new->public->type = GCRY_AC_KEY_PUBLIC;
	  key_pair_new->secret->data_sexp = NULL;
	  key_pair_new->public->data_sexp = NULL;
	  key_pair_new->secret->data = NULL;
	  key_pair_new->public->data = NULL;
	}
    }

  if (! err)
    {
      /* Calculate size of the format string, that is used for
	 creating the request S-expression.  */
      genkey_format_n = 23;

      /* Respect any relevant algorithm specific commands.  */
      if (key_spec)
	for (i = 0; gcry_ac_key_generate_specs[i].algorithm; i++)
	  if (handle->algorithm == gcry_ac_key_generate_specs[i].algorithm)
	    genkey_format_n += 6;

      /* Create format string.  */
      genkey_format = gcry_malloc (genkey_format_n);
      if (! genkey_format)
	err = gpg_err_code_from_errno (errno);
      else
	{
	  /* Fill format string.  */
	  *genkey_format = 0;
	  strcat (genkey_format, "(genkey(%s(nbits%d)");
	  if (key_spec)
	    for (i = 0; gcry_ac_key_generate_specs[i].algorithm; i++)
	      if (handle->algorithm == gcry_ac_key_generate_specs[i].algorithm)
		strcat (genkey_format, "(%s%m)");
	  strcat (genkey_format, "))");
	}
    }

  if (! err)
    {
      /* Build list of argument pointers, the algorithm name and the
	 nbits are needed always.  */
      arg_list_n = 2;

      /* Now the algorithm specific arguments.  */
      if (key_spec)
	for (i = 0; gcry_ac_key_generate_specs[i].algorithm; i++)
	  if (handle->algorithm == gcry_ac_key_generate_specs[i].algorithm)
	    arg_list_n += 2;

      /* Allocate list.  */
      arg_list = gcry_malloc (sizeof (void *) * arg_list_n);
      if (! arg_list)
	err = gpg_err_code_from_errno (errno);
      else
	{
	  /* Fill argument list. */
	  
	  int j;

	  arg_list[0] = (void *) &handle->algorithm_name;
	  arg_list[1] = (void *) &nbits;

	  if (key_spec)
	    for (j = 2, i = 0; gcry_ac_key_generate_specs[i].algorithm; i++)
	      if (handle->algorithm == gcry_ac_key_generate_specs[i].algorithm)
		{
		  /* Add name of this specification flag and the
		     according member of the spec strucuture.  */
		  arg_list[j++] = (void *)(&gcry_ac_key_generate_specs[i].name);
		  arg_list[j++] = (void *)
                                  (((char *) key_spec)
                                   + gcry_ac_key_generate_specs[i].offset);
		}
	}
    }

  if (! err)
    /* Construct final request S-expression.  */
    err = gcry_err_code (gcry_sexp_build_array (&genkey_sexp_request, NULL,
					       genkey_format, arg_list));

  if (! err)
    /* Perform genkey operation.  */
    err = gcry_err_code (gcry_pk_genkey (&genkey_sexp_reply,
					genkey_sexp_request));

  /* Split keys.  */
  if (! err)
    {
      key_pair_new->secret->data_sexp = gcry_sexp_find_token (genkey_sexp_reply,
							      "private-key", 0);
      if (! key_pair_new->secret->data_sexp)
	err = GPG_ERR_INTERNAL;
    }
  if (! err)
    {
      key_pair_new->public->data_sexp = gcry_sexp_find_token (genkey_sexp_reply,
							      "public-key", 0);
      if (! key_pair_new->public->data_sexp)
	err = GPG_ERR_INTERNAL;
    }

  /* Extract key material.  */
  if (! err)
    err = gcry_ac_data_extract ("private-key", handle->algorithm_name,
				key_pair_new->secret->data_sexp,
				&key_pair_new->secret->data);
  if (! err)
    err = gcry_ac_data_extract ("public-key", handle->algorithm_name,
				key_pair_new->public->data_sexp,
				&key_pair_new->public->data);

  /* Done.  */

  if (! err)
    *key_pair = key_pair_new;
  else
    {
      /* Deallocate resources.  */

      if (key_pair_new)
	{
	  if (key_pair_new->secret)
	    gcry_ac_key_destroy (key_pair_new->secret);
	  if (key_pair_new->public)
	    gcry_ac_key_destroy (key_pair_new->public);

	  gcry_free (key_pair_new);
	}

      if (arg_list)
	gcry_free (arg_list);

      if (genkey_format)
	gcry_free (genkey_format);

      if (genkey_sexp_request)
	gcry_sexp_release (genkey_sexp_request);
      if (genkey_sexp_reply)
	gcry_sexp_release (genkey_sexp_reply);
    }

  return gcry_error (err);
}

/* Returns the key of type WHICH out of the key pair KEY_PAIR.  */
gcry_ac_key_t
gcry_ac_key_pair_extract (gcry_ac_key_pair_t key_pair,
			  gcry_ac_key_type_t witch)
{
  gcry_ac_key_t key = NULL;

  switch (witch)
    {
    case GCRY_AC_KEY_SECRET:
      key = key_pair->secret;
      break;

    case GCRY_AC_KEY_PUBLIC:
      key = key_pair->public;
      break;
    }

  return key;
}

/* Destroys the key KEY.  */
void
gcry_ac_key_destroy (gcry_ac_key_t key)
{
  int i;

  if (key)
    {
      if (key->data)
        {
          for (i = 0; i < key->data->data_n; i++)
            if (key->data->data[i].mpi != NULL)
              gcry_mpi_release (key->data->data[i].mpi);
          gcry_free (key->data);
        }
      if (key->data_sexp)
        gcry_sexp_release (key->data_sexp);
      gcry_free (key);
    }
}

/* Destroys the key pair KEY_PAIR.  */
void
gcry_ac_key_pair_destroy (gcry_ac_key_pair_t key_pair)
{
  if (key_pair)
    {
      gcry_ac_key_destroy (key_pair->secret);
      gcry_ac_key_destroy (key_pair->public);
      gcry_free (key_pair);
    }
}

/* Returns the data set contained in the key KEY.  */
gcry_ac_data_t
gcry_ac_key_data_get (gcry_ac_key_t key)
{
  return  key->data;
}

/* Verifies that the key KEY is sane via HANDLE.  */
gcry_error_t
gcry_ac_key_test (gcry_ac_handle_t handle, gcry_ac_key_t key)
{
  gcry_err_code_t err;

  err = gcry_err_code (gcry_pk_testkey (key->data_sexp));

  return gcry_error (err);
}

/* Stores the number of bits of the key KEY in NBITS via HANDLE.  */
gcry_error_t
gcry_ac_key_get_nbits (gcry_ac_handle_t handle, gcry_ac_key_t key, unsigned int *nbits)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  unsigned int n;

  n = gcry_pk_get_nbits (key->data_sexp);
  if (n)
    *nbits = n;
  else
    err = GPG_ERR_PUBKEY_ALGO;

  return gcry_error (err);
}

/* Writes the 20 byte long key grip of the key KEY to KEY_GRIP via
   HANDLE.  */
gcry_error_t
gcry_ac_key_get_grip (gcry_ac_handle_t handle, gcry_ac_key_t key, unsigned char *key_grip)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  unsigned char *ret;

  ret = gcry_pk_get_keygrip (key->data_sexp, key_grip);
  if (! ret)
    err = GPG_ERR_INV_OBJ;

  return gcry_error (err);
}



/* 
 * Functions performing cryptographic operations.
 */

/* Encrypts the plain text MPI value DATA_PLAIN with the key public
   KEY under the control of the flags FLAGS and stores the resulting
   data set into DATA_ENCRYPTED.  */
gcry_error_t
gcry_ac_data_encrypt (gcry_ac_handle_t handle,
		      unsigned int flags,
		      gcry_ac_key_t key,
		      gcry_mpi_t data_plain,
		      gcry_ac_data_t *data_encrypted)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_sexp_t sexp_request = NULL;
  gcry_sexp_t sexp_reply = NULL;
  char *request_format = NULL;
  size_t request_format_n = 0;
  gcry_ac_data_t data;
  
  int i;

  if (key->type != GCRY_AC_KEY_PUBLIC)
    err = GPG_ERR_WRONG_KEY_USAGE;

  if (! err)
    {
      /* Calculate request format string.  */

      request_format_n += 23;
      for (i = 0; gcry_ac_flags[i].number; i++)
	if (flags & gcry_ac_flags[i].number)
	  request_format_n += strlen (gcry_ac_flags[i].string) + 1;

      /* Allocate request format string.  */
      request_format = gcry_malloc (request_format_n);
      if (! request_format)
	err = gpg_err_code_from_errno (errno);
    }

  if (! err)
    {
      /* Fill format string.  */
      *request_format = 0;
      strcat (request_format, "(data(flags");
      for (i = 0; gcry_ac_flags[i].number; i++)
	if (flags & gcry_ac_flags[i].number)
	  {
	    strcat (request_format, " ");
	    strcat (request_format, gcry_ac_flags[i].string);
	  }
      strcat (request_format, ")(value%m))");

      /* Create S-expression.  */
      err = gcry_sexp_build (&sexp_request, NULL,
			     request_format, data_plain);
    }

  if (! err)
    /* Encrypt.  */
    err = gcry_pk_encrypt (&sexp_reply, sexp_request, key->data_sexp);

  if (! err)
    /* Extract data.  */
    err = gcry_ac_data_extract ("enc-val", handle->algorithm_name,
				sexp_reply, &data);

  /* Deallocate resources.  */

  if (sexp_request)
    gcry_sexp_release (sexp_request);
  if (sexp_reply)
    gcry_sexp_release (sexp_reply);

  if (! err)
    /* Copy out.  */
    *data_encrypted = data;

  return gcry_error (err);
}

/* Decrypts the encrypted data contained in the data set
   DATA_ENCRYPTED with the secret key KEY under the control of the
   flags FLAGS and stores the resulting plain text MPI value in
   DATA_PLAIN.  */
gcry_error_t
gcry_ac_data_decrypt (gcry_ac_handle_t handle,
		      unsigned int flags,
		      gcry_ac_key_t key,
		      gcry_mpi_t *data_plain,
		      gcry_ac_data_t data_encrypted)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_mpi_t data_decrypted = NULL;
  gcry_sexp_t sexp_request = NULL;
  gcry_sexp_t sexp_reply = NULL;

  if (key->type != GCRY_AC_KEY_SECRET)
    err = GPG_ERR_WRONG_KEY_USAGE;

  if (! err)
    /* Create S-expression from data.  */
    err = gcry_ac_data_construct ("enc-val", 1, flags, handle->algorithm_name,
				  data_encrypted, &sexp_request);

  if (! err)
    /* Decrypt.  */
    err = gcry_pk_decrypt (&sexp_reply, sexp_request, key->data_sexp);

  if (! err)
    {
      /* Extract plain text. */

      gcry_sexp_t l;

      l = gcry_sexp_find_token (sexp_reply, "value", 0);
      if (! l)
	err = GPG_ERR_GENERAL;
      else
	{
	  data_decrypted = gcry_sexp_nth_mpi (l, 1, GCRYMPI_FMT_USG);
	  if (! data_decrypted)
	    err = GPG_ERR_GENERAL;
	  gcry_sexp_release (l);
	}
    }

  /* Done.  */

  if (err)
    {
      /* Deallocate resources.  */
      if (sexp_request)
	gcry_sexp_release (sexp_request);
      if (sexp_reply)
	gcry_sexp_release (sexp_reply);
    }
  else
    *data_plain = data_decrypted;

  return gcry_error (err);

}

/* Signs the data contained in DATA with the secret key KEY and stores
   the resulting signature data set in DATA_SIGNATURE.  */
gcry_error_t
gcry_ac_data_sign (gcry_ac_handle_t handle,
		   gcry_ac_key_t key,
		   gcry_mpi_t data,
		   gcry_ac_data_t *data_signature)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_sexp_t sexp_request = NULL;
  gcry_sexp_t sexp_reply = NULL;
  gcry_ac_data_t ac_data;

  if (key->type != GCRY_AC_KEY_SECRET)
    err = GPG_ERR_WRONG_KEY_USAGE;

  if (! err)
    /* Create S-expression holding the data.  */
    err = gcry_sexp_build (&sexp_request, NULL,
			   "(data(flags)(value%m))", data);
  if (! err)
    /* Sign.  */
    err = gcry_pk_sign (&sexp_reply, sexp_request, key->data_sexp);

  if (! err)
    /* Extract data.  */
    err = gcry_ac_data_extract ("sig-val", handle->algorithm_name,
				sexp_reply, &ac_data);

  /* Done.  */

  if (sexp_request)
    gcry_sexp_release (sexp_request);
  if (sexp_reply)
    gcry_sexp_release (sexp_reply);

  if (! err)
    *data_signature = ac_data;

  return gcry_error (err);
}

/* Verifies that the signature contained in the data set
   DATA_SIGNATURE is indeed the result of signing the data contained
   in DATA with the secret key belonging to the public key KEY.  */
gcry_error_t
gcry_ac_data_verify (gcry_ac_handle_t handle,
		     gcry_ac_key_t key,
		     gcry_mpi_t data,
		     gcry_ac_data_t data_signature)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_sexp_t sexp_request = NULL;
  gcry_sexp_t sexp_data = NULL;

  if (key->type != GCRY_AC_KEY_PUBLIC)
    err = GPG_ERR_WRONG_KEY_USAGE;

  if (! err)
    /* Construct S-expression holding the signature data.  */
    err = gcry_ac_data_construct ("sig-val", 1, 0, handle->algorithm_name,
				  data_signature, &sexp_request);

  if (! err)
    /* Construct S-expression holding the data.  */
    err = gcry_sexp_build (&sexp_data, NULL,
			   "(data(flags)(value%m))", data);

  if (! err)
    /* Verify signature.  */
    err = gcry_pk_verify (sexp_request, sexp_data, key->data_sexp);

  /* Done.  */

  if (sexp_request)
    gcry_sexp_release (sexp_request);
  if (sexp_data)
    gcry_sexp_release (sexp_data);

  return gcry_error (err);
}



/* 
 * General functions.
 */

/* Stores the textual representation of the algorithm whose id is
   given in ALGORITHM in NAME.  */
gcry_error_t
gcry_ac_id_to_name (gcry_ac_id_t algorithm, const char **name)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  const char *n;

  n = gcry_pk_algo_name (algorithm);
  if (*n)
    *name = n;
  else
    err = GPG_ERR_PUBKEY_ALGO;

  return gcry_error (err);
}

/* Stores the numeric ID of the algorithm whose textual representation
   is contained in NAME in ALGORITHM.  */
gcry_error_t
gcry_ac_name_to_id (const char *name, gcry_ac_id_t *algorithm)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  int algo;

  algo = gcry_pk_map_name (name);
  if (algo)
    *algorithm = algo;
  else
    err = GPG_ERR_PUBKEY_ALGO;
    
  return gcry_error (err);
}

gcry_err_code_t
_gcry_ac_init (void)
{
  return 0;
}
