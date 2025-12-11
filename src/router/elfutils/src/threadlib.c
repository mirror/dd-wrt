/* Functions for running jobs concurrently.
   Copyright (C) 2025 Red Hat, Inc.
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <error.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#include "threadlib.h"

/* Dynamic buffer for thread output.  */
typedef struct {
  size_t sizeloc;
  char *buf;
  FILE *file;
} output_stream_t;

/* Allocate resources for STREAM.  */
static void
init_thread_output_stream (output_stream_t *stream)
{
  stream->buf = NULL;
  stream->sizeloc = 0;
  stream->file = open_memstream (&(stream->buf), &(stream->sizeloc));

  if (stream->file == NULL)
    error (1, 0, _("cannot open thread output stream"));
}

/* Print and deallocate resources for STREAM.  */
static void
print_thread_output_stream (output_stream_t *stream)
{
  /* fclose may update stream->buf.  */
  if (fclose (stream->file) != 0)
    error (1, 0, _("cannot close thread output stream"));

  fwrite (stream->buf, 1, stream->sizeloc, stdout);
  free (stream->buf);
}

typedef enum {
  /* pthread_create has not been called.  */
  NOT_STARTED,

  /* pthread_create has been called.  */
  STARTED,

  /* The thread has finished running the job but has not been joined.  */
  DONE,

  /* pthread_join has been called.  */
  JOINED
} thread_state_t;

struct job_t {
  /* A job consists of calling this function then printing any output
     to stdout.  This function is run from thread_start_job, which also
     initializes the FILE *.  */
  void *(*start_routine)(void *, FILE *);

  /* Arg passed to start_routine.  */
  void *arg;

  /* Thread to run start_routine.  */
  pthread_t thread;

  /* See thread_state_t.  */
  _Atomic thread_state_t state;

  /* Dynamic buffer for output generated during start_routine.
     Contents will get printed to stdout when a job finishes.  */
  output_stream_t stream;

  /* Next job in the linked list.  */
  struct job_t *next;
};

typedef struct {
  struct job_t *head;
  struct job_t *tail;
} job_queue_t;

/* If HEAD is NULL, no jobs are currently in the queue.  While jobs are
   being added with add_job, HEAD points to the first job that was added
   and TAIL points to the most recently added job.  When run_jobs returns,
   HEAD and TAIL will be NULL.  */
static job_queue_t jobs = { NULL, NULL };

void
add_job (void *(*start_routine)(void *, FILE *), void *arg)
{
  struct job_t *job = malloc (sizeof (struct job_t));

  if (job == NULL)
    error (1, 0, _("cannot create job"));

  job->start_routine = start_routine;
  job->arg = arg;
  job->next = NULL;
  atomic_store (&job->state, NOT_STARTED);

  /* Insert job into the job queue.  */
  if (jobs.head == NULL)
    {
      assert (jobs.tail == NULL);
      jobs.head = job;
      jobs.tail = job;
    }
  else
    {
      assert (jobs.tail != NULL);
      jobs.tail->next = job;
      jobs.tail = job;
    }
}

static pthread_mutex_t job_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t job_cv = PTHREAD_COND_INITIALIZER;

/* Thread entry point.  */
static void *
thread_start_job (void *arg)
{
  struct job_t *job = (struct job_t *) arg;

  init_thread_output_stream (&job->stream);
  job->start_routine (job->arg, job->stream.file);

  /* Wake up main thread.  */
  pthread_mutex_lock(&job_mutex);
  atomic_store (&job->state, DONE);
  pthread_cond_signal (&job_cv);
  pthread_mutex_unlock (&job_mutex);

  return NULL;
}

/* Run all jobs that have been added to the job queue by add_job.  */
void
run_jobs (int max_threads)
{
  if (jobs.head == NULL)
    {
      assert (jobs.tail == NULL);
      return;
    }
  assert (jobs.tail != NULL);

  /* jobs.tail was only needed to facilitate adding jobs.  */
  jobs.tail = NULL;
  int num_threads = 0;

  /* Start no more than MAX_THREAD jobs.  */
  for (struct job_t *job = jobs.head;
       job != NULL && num_threads < max_threads;
       job = job->next)
    {
      assert (job->start_routine != NULL);
      atomic_store (&job->state, STARTED);

      if (pthread_create (&job->thread, NULL,
			  thread_start_job, (void *) job) != 0)
	error(1, 0, _("cannot create thread"));
      num_threads++;
    }

  int available_threads = max_threads - num_threads;
  assert (available_threads >= 0);

  pthread_mutex_lock (&job_mutex);

  /* Iterate over the jobs until all have completed and all output has
     been printed.  */
  while (jobs.head != NULL)
    {
      /* Job output should be printed in the same order that the jobs
	 were added.  Track whether there is at least one previous job
         whose output hasn't been printed yet.  If true, then defer
         printing for the current job.  */
      bool wait_to_print = false;
      bool made_progress = false;

      struct job_t *job = jobs.head;
      struct job_t *prev = NULL;
      while (job != NULL)
	{
          /* Check whether this job should be started.  */
	  if (atomic_load (&job->state) == NOT_STARTED)
	    {
	      /* Start this job if there is an available thread.  */
	      if (available_threads > 0)
		{
		  atomic_store (&job->state, STARTED);
		  if (pthread_create (&job->thread, NULL,
				      thread_start_job, (void *) job) != 0)
		    error (1, 0, _("cannot create thread"));

		  available_threads--;
		  made_progress = true;
		}
	    }

	  /* Join thread if we haven't done so already.  */
	  if (atomic_load (&job->state) == DONE)
	    {
	      if (pthread_join (job->thread, NULL) != 0)
		error (1, 0, _("cannot join thread"));

	      atomic_store (&job->state, JOINED);
	      available_threads++;
	      made_progress = true;
	    }

	  /* Print job output if it hasn't already been printed and
	     there is no unprinted output from a previous job.

	     Once a job's output has been printed all resources for
	     the job can be freed and it can be removed from the
	     job queue.  */
	  if (atomic_load (&job->state) == JOINED && !wait_to_print)
	    {
	      print_thread_output_stream (&job->stream);

	      /* Remove this job from the queue.  */
	      if (prev == NULL)
		{
		  /* This job is at the beginning of the queue.  */
		  jobs.head = job->next;

	          free (job);
		  job = jobs.head;
		}
	      else
		{
		  prev->next = job->next;

		  free (job);
		  job = prev->next;
		}

	      made_progress = true;
	      continue;
	    }

	  prev = job;
	  job = job->next;
	  wait_to_print = true;
	}

      if (!made_progress && jobs.head != NULL)
        pthread_cond_wait(&job_cv, &job_mutex);
    }

  pthread_mutex_unlock(&job_mutex);
}
