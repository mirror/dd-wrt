/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.bind.tuple.TupleBinding;
import com.sleepycat.bind.tuple.TupleInput;
import com.sleepycat.bind.tuple.TupleOutput;
import com.sleepycat.db.*;
import com.sleepycat.sample.data.Ticket;

import java.io.UnsupportedEncodingException;

/**
 * BasicTicketDAO is a TicketDAO implementation using the base API.
 */
public class BasicTicketDAO implements TicketDAO {
	/* The file name of the ticket database. */
	static final String DB_NAME = "Ticket";

	/* The key of the ticket id sequence. */
	static final DatabaseEntry ID_SEQ_KEY;

	static {
		DatabaseEntry idSeqKey;
		try {
			idSeqKey = new DatabaseEntry("TicketIdSeq".getBytes("UTF-8"));
		} catch (UnsupportedEncodingException e) {
			idSeqKey = new DatabaseEntry("TicketIdSeq".getBytes());
		}
		ID_SEQ_KEY = idSeqKey;
	}

	/* The database manager from which this DAO is created. */
	private final BasicDbManager dbMgr;

	/* The ticket database handle. */
	private final Database ticketDb;

	/* The ticket id sequence handle. */
	private final Sequence idSeq;

	/* The binding for key database entries. */
	private final TupleBinding<Long> keyBinding;

	/* The binding for value database entries. */
	private final TicketBinding valueBinding;

	/**
	 * Create a data access object for tickets.
	 *
	 * @param dbManager the database manager creating this object
	 * @throws Exception on database error
	 */
	public BasicTicketDAO(BasicDbManager dbManager) throws Exception {
		dbMgr = dbManager;

		DatabaseConfig dbConfig = new DatabaseConfig();
		dbConfig.setTransactional(true);
		dbConfig.setType(DatabaseType.BTREE);
		ticketDb = dbMgr.env.openDatabase(null, DB_NAME, null, dbConfig);

		SequenceConfig seqConfig = new SequenceConfig();
		seqConfig.setAutoCommitNoSync(true);
		seqConfig.setCacheSize(100);
		idSeq = ticketDb.openSequence(null, ID_SEQ_KEY, null);

		keyBinding = TupleBinding.getPrimitiveBinding(Long.class);
		valueBinding = new TicketBinding();
	}

	@Override
	public void close() throws Exception {
		idSeq.close();
		ticketDb.close();
	}

	@Override
	public Long saveTicket(Ticket t) throws Exception {
		DatabaseEntry key = new DatabaseEntry();
		DatabaseEntry value = new DatabaseEntry();

		Long ticketId = idSeq.get(null, 1);

		keyBinding.objectToEntry(ticketId, key);
		valueBinding.objectToEntry(
				new Ticket(ticketId, t.getMeterId(), t.getIssuingTime()), value);

		ticketDb.putNoOverwrite(dbMgr.getCurrentTxn(), key, value);

		return ticketId;
	}

	@Override
	public Ticket getTicket(long ticketId) throws Exception {
		DatabaseEntry key = new DatabaseEntry();
		DatabaseEntry value = new DatabaseEntry();

		keyBinding.objectToEntry(ticketId, key);

		OperationStatus s = ticketDb.get(
				dbMgr.getCurrentTxn(), key, value, LockMode.READ_COMMITTED);

		if (s == OperationStatus.SUCCESS) {
			return valueBinding.entryToObject(value);
		} else {
			return null;
		}
	}

	@Override
	public void deleteTicket(long ticketId) throws Exception {
		DatabaseEntry key = new DatabaseEntry();

		keyBinding.objectToEntry(ticketId, key);

		ticketDb.delete(dbMgr.getCurrentTxn(), key);
	}

	/**
	 * Helper class to convert between ticket and byte arrays.
	 */
	private static class TicketBinding extends TupleBinding<Ticket> {
		@Override
		public Ticket entryToObject(TupleInput in) {
			return new Ticket(in.readLong(), in.readString(), in.readLong());
		}

		@Override
		public void objectToEntry(Ticket ticket, TupleOutput out) {
			out.writeLong(ticket.getTicketId());
			out.writeString(ticket.getMeterId());
			out.writeLong(ticket.getIssuingTime());
		}
	}
}
