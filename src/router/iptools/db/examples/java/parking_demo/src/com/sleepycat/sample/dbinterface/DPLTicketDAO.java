/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.LockMode;
import com.sleepycat.persist.EntityStore;
import com.sleepycat.persist.PrimaryIndex;
import com.sleepycat.persist.StoreConfig;
import com.sleepycat.sample.data.Ticket;

/**
 * DPLTicketDAO is a TicketDAO implementation using the Direct Persistence
 * Layer API.
 */
public class DPLTicketDAO implements TicketDAO {
	/* The name of the ticket store. */
	static final String STORE_NAME = "Ticket";

	/* The database manager from which this DAO is created. */
	private final DPLDbManager dbMgr;

	/* The ticket store. */
	private final EntityStore ticketStore;

	/* The primary key index. */
	private final PrimaryIndex<Long, Ticket> idIndex;

	/**
	 * Create a data access object for tickets.
	 *
	 * @param dbManager the database manager creating this object
	 * @throws DatabaseException on database error
	 */
	public DPLTicketDAO(DPLDbManager dbManager) throws DatabaseException {
		dbMgr = dbManager;

		StoreConfig ticketCfg = new StoreConfig().setTransactional(true);
		ticketStore = new EntityStore(dbMgr.env, STORE_NAME, ticketCfg);

		idIndex = ticketStore.getPrimaryIndex(Long.class, Ticket.class);
	}

	@Override
	public void close() throws DatabaseException {
		ticketStore.close();
	}

	@Override
	public Long saveTicket(Ticket t) throws DatabaseException {
		Ticket copy = new Ticket(null, t.getMeterId(), t.getIssuingTime());
		idIndex.putNoOverwrite(dbMgr.getCurrentTxn(), copy);
		return copy.getTicketId();
	}

	@Override
	public Ticket getTicket(long ticketId) throws DatabaseException {
		return idIndex.get(dbMgr.getCurrentTxn(), ticketId, LockMode.READ_COMMITTED);
	}

	@Override
	public void deleteTicket(long ticketId) throws DatabaseException {
		idIndex.delete(dbMgr.getCurrentTxn(), ticketId);
	}
}
