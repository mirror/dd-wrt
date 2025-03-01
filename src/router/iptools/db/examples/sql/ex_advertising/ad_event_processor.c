/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Location Based Personalized Advertising Example
 *
 * This example shows a location based personalized advertising application.
 * It simulates a server receiving customer location's from their smart phones
 * and sends advertisements back to the phone based on the customer's location
 * and shopping preferences.  
 */

/*
 * Event Processor
 *
 * This program simulates a server receiving GPS coordinates from smart phones
 * and sending back advertisements tailored to the user's location and shopping
 * preferences.  The adds are stored in a Berkeley DB SQL database that also
 * contains the store's location, name, and type.  The user shopping
 * preferences are stored in a Berkeley DB btree database that supports
 * multiple data entries for each key, in this case user id.
 */

#include "ad_common_utils.h"
#include "ad_bdb_filters.h"

static void *get_events __P((void *));

/* Creates several threads to receive and process events. */
int main()
{
    BDB_EVT_FILTERS *filters;
    BDB_EVT_FILTER filter[NUM_FILTERS];
    db_seq_t num_req, num_ads;
    os_thread_t pid;
    int i, j, ret;

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
	filter[i].num_dbs = 2;
	filters->filters[i] = &(filter[i]);
	filter[i].dbs = malloc(sizeof(DB *) * filter[i].num_dbs);
	if (filter[i].dbs == NULL) {
	    ret = ENOMEM;
	    goto err;
	}
	for (j = 0; j < filter[i].num_dbs; j++) {
	    filter[i].dbs[j] = NULL;
	}
    }

    /* Set up the filters. */
    if ((ret = advertise_setup(filters->filters[0])) != 0)
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

    /* Print out the total number of ads sent and ad requests. */
    if ((ret = filter[0].seq1->get(filter[0].seq1, NULL, 1, &num_req, 0)) != 0)
	goto err;
    if ((ret = filter[0].seq2->get(filter[0].seq2, NULL, 1, &num_ads, 0)) != 0)
	goto err;
    printf("%d requests processed and %d ads sent.\n", num_req, num_ads);
err:	    
    advertise_teardown(&(filter[0]));
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
