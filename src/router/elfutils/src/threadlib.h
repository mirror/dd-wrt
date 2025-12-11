/* Copyright (C) 2025 Red Hat, Inc.
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

#ifndef _THREADLIB_H
#define _THREADLIB_H	1

/* Add a job to the job queue.  When the job is run using run_job, it will
   consist of start_routine called with ARG as well as a FILE *. The
   contents of the FILE will be printed to stdout once start_routine
   finishes.  */
extern void add_job (void *(*start_routine)(void *, FILE *), void *arg);

/* Run all jobs that have been added by add_job.  Jobs run in parallel
   using at most MAX_THREADS threads.

   run_jobs returns when all jobs have finished and any output from the
   jobs has been printed to stdout.  Output from each job is printed in
   the order which jobs were added using add_job.

   While run_jobs executes, new jobs should not be added with add_job.   */
extern void run_jobs (int max_threads);

#endif  /* threadlib.h */
