/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Automatic Toll Booth Event Processing Example
 *
 * This application simulates an automated highway toll booth.  The application
 * receives a data stream of licence plates and timestamps of cars passing
 * through the toll booth.  The application uses this information to perform
 * billing, send traffic alerts, and check for stolen cars.
 */

/*
 * Event Processor
 *
 * This program receives events from the event generator that describe cars
 * passing through an automated toll booth.  The events are filtered by filters
 * that check for stolen cars, perform billing, and monitor traffic in order to
 * send out heavy traffic alerts.
 */

#include "toll_common_utils.h"
#include "toll_bdb_filters.h"

static void *get_events __P((void *));

/* Creates several threads to receive and process events. */
int main()
{
    BDB_EVT_FILTERS *filters;
    BDB_EVT_FILTER filter[NUM_FILTERS];
    db_seq_t num_cars;
    os_thread_t pid;
    int i, ret;

    /*
     * Create the filters that sort the events and forward them to
     * different modules based on the event.
     */
    filters = malloc(sizeof(BDB_EVT_FILTERS));
    if (filters == NULL)
	return (ENOMEM);
    filters->filters = NULL;
    filters->filters = malloc(sizeof(BDB_EVT_FILTER *) * NUM_FILTERS);
    if (filters->filters == NULL) {
	free(filters);
	return (ENOMEM);
    }
    filters->num_filters = NUM_FILTERS;
    for (i = 0; i < filters->num_filters; i++) {
	memset(&(filter[i]), 0, sizeof(BDB_EVT_FILTER));
	filters->filters[i] = &(filter[i]);
    }

    /* Set up the filters. */
    if ((ret = stolen_setup(filters->filters[0])) != 0)
	goto err;
    if ((ret = billing_setup(filters->filters[1])) != 0)
	goto err;
    if ((ret = traffic_setup(filters->filters[2])) != 0)
	goto err;

    /* Start recieving and processing events. */
    for (i = 0; i < NUM_THREADS; i++) {
	if (os_thread_create(&pid, get_events, filters))
	    register_thread_id(pid);
	else
	    fprintf(stderr, "Failed to create thread\n");
	usleep(10);
    }
    join_threads();

    /* Print out the total number of cars that passed the toll booth. */
    if ((ret = filter[2].seq->get(filter[2].seq, NULL, 1, &num_cars, 0)) != 0)
	goto err;
    printf("%d cars passed the automatic toll booth.\n", num_cars);
err:	    
    stolen_teardown(&(filter[0]));
    billing_teardown(&(filter[1]));
    traffic_teardown(&(filter[2]));
    if (filters->filters != NULL)
	free(filters->filters);
    free(filters);
    return (ret);
}

/* Receive and process events. */
static void *get_events(args)
		void *args;
{
		BDB_EVT_FILTERS *filters;

		filters = (BDB_EVT_FILTERS *)args;
		(void)receive_events(filters);
		return (NULL);
}
