/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.sample.data.TicketLog;

/**
 * The ticket-log data access object wraps the logic for persisting ticketing
 * log messages into the underlying database.
 */
public interface TicketLogDAO extends AutoCloseable {
	/**
	 * Save the given log message to the underlying database.
	 *
	 * @param log the log message to save
	 * @throws Exception on error
	 */
	void saveLog(TicketLog log) throws Exception;

	/**
	 * Retrieve all log messages for the given parking meter between the
	 * from (inclusive) time and until (exclusive) time.
	 *
	 * @param meterId the parking meter id
	 * @param from    the from (inclusive) time of the retrieved log messages
	 * @param until   the until (exclusive) time of the retrived log messages
	 * @return an enumeration of all log messages matching the query criteria
	 * @throws Exception on error
	 */
	CloseableIterable<TicketLog> queryLog(
			String meterId, long from, long until) throws Exception;
}
