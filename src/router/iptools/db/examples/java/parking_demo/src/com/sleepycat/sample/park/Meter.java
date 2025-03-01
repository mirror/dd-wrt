/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.park;

import com.sleepycat.sample.data.Ticket;
import com.sleepycat.sample.data.TicketLog;
import com.sleepycat.sample.dbinterface.DbManager;
import com.sleepycat.sample.dbinterface.TicketDAO;
import com.sleepycat.sample.dbinterface.TicketLogDAO;

import java.util.Date;

/**
 * The Meter class represents a parking meter which can issue parking tickets
 * and calculate parking fees given tickets. Besides, each meter has an unique
 * ID to identify it.
 */
public class Meter implements AutoCloseable {
	/* The parking fee in cents per minute. */
	private static final int FEE_PER_MINUTE = 2;

	/* The parking meter id. */
	private final String id;

	/* The database manager used by this meter. */
	private final DbManager dbMgr;

	/* The ticket data access object. */
	private final TicketDAO ticketDAO;

	/* The log message data access object. */
	private final TicketLogDAO logDAO;

	/**
	 * Create a parking meter.
	 *
	 * @param id    the parking meter id
	 * @param dbMgr the database manager used by this meter
	 * @throws Exception on error
	 */
	public Meter(String id, DbManager dbMgr) throws Exception {
		this.id = id;
		this.dbMgr = dbMgr;
		ticketDAO = dbMgr.createTicketDAO();
		logDAO = dbMgr.createTicketLogDAO();
	}

	@Override
	public void close() throws Exception {
		ticketDAO.close();
		logDAO.close();
	}

	/**
	 * A car parks at the given time and is issued a parking ticket.
	 *
	 * @param time the parking time
	 * @return a parking ticket
	 * @throws Exception on error
	 */
	public Ticket park(Date time) throws Exception {
		Ticket t = new Ticket(0L, id, time.getTime());

		dbMgr.beginTxn();
		try {
			Long ticketId = ticketDAO.saveTicket(t);
			logDAO.saveLog(new TicketLog(
					time.getTime(), id, ticketId, TicketLog.Action.ISSUE, 0));
			dbMgr.commit();
			return new Ticket(ticketId, id, time.getTime());
		} catch (Exception e) {
			dbMgr.abort();
			throw e;
		}
	}

	/**
	 * A car departs at the given time. Given its parking ticket, computes
	 * its parking fee.
	 *
	 * @param t    the parking ticket
	 * @param time the departure time
	 * @return the parking fee in cents
	 * @throws Exception on error
	 */
	public int depart(Ticket t, Date time) throws Exception {
		dbMgr.beginTxn();

		try {
			Ticket ticket = ticketDAO.getTicket(t.getTicketId());
			int fee = computeFee(ticket, time);
			logDAO.saveLog(new TicketLog(time.getTime(),
					id, ticket.getTicketId(), TicketLog.Action.CHARGE, fee));
			ticketDAO.deleteTicket(ticket.getTicketId());
			dbMgr.commit();
			return fee;
		} catch (Exception e) {
			dbMgr.abort();
			throw e;
		}
	}

	/**
	 * Computes the parking fee given a ticket and its departure time.
	 *
	 * @param t    the parking ticket
	 * @param time the departure time
	 * @return the parking fee in cents
	 */
	private int computeFee(Ticket t, Date time) {
		long departTime = time.getTime();
		long parkTime = t.getIssuingTime();

		if (parkTime < departTime) {
			return (int) ((departTime - parkTime) / 1000 / 60 * FEE_PER_MINUTE);
		} else {
			return 0;
		}
	}
}
