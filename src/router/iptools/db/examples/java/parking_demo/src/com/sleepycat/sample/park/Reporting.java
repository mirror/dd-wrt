/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.park;

import com.sleepycat.sample.data.TicketLog;
import com.sleepycat.sample.dbinterface.CloseableIterable;
import com.sleepycat.sample.dbinterface.DbManager;
import com.sleepycat.sample.dbinterface.TicketLogDAO;

/**
 * The Reporting class represents a report generator that creates business
 * reports for parking log managers.
 */
public class Reporting implements AutoCloseable {
	/* The database manager used by this meter. */
	private final DbManager dbMgr;

	/* The log message data access object. */
	private final TicketLogDAO logDAO;

	/**
	 * Create a reporting component.
	 *
	 * @param dbMgr the database manager used by this component
	 * @throws Exception on error
	 */
	public Reporting(DbManager dbMgr) throws Exception {
		this.dbMgr = dbMgr;
		logDAO = dbMgr.createTicketLogDAO();
	}

	@Override
	public void close() throws Exception {
		logDAO.close();
	}

	/**
	 * Computes the total number of cars parked during the given period for
	 * a given parking meter.
	 *
	 * @param from    the period from (inclusive)
	 * @param until   the period until (exclusive)
	 * @param meterId the parking meter id
	 * @return the total number of cars parked
	 * @throws Exception on error
	 */
	public int totalCarsParkedDuring(long from, long until, String meterId)
			throws Exception {
		int total = 0;

		try (CloseableIterable<TicketLog> iter = logDAO.queryLog(meterId, from, until)) {
			for (TicketLog log : iter) {
				if (log.getAction() == TicketLog.Action.ISSUE) {
					total++;
				}
			}
		}

		return total;
	}

	/**
	 * Computes the total parking fees collected during the given period for
	 * a given parking meter.
	 *
	 * @param from    the period from (inclusive)
	 * @param until   the period until (exclusive)
	 * @param meterId the parking meter id
	 * @return the total parking fees collected in cents
	 * @throws Exception on error
	 */
	public int totalFeesCollectedDuring(long from, long until, String meterId)
			throws Exception {
		int total = 0;

		try (CloseableIterable<TicketLog> iter = logDAO.queryLog(meterId, from, until)) {
			for (TicketLog log : iter) {
				if (log.getAction() == TicketLog.Action.CHARGE) {
					total += log.getChargeAmountInCents();
				}
			}
		}

		return total;
	}
}
