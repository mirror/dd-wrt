/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.db.CursorConfig;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.persist.*;
import com.sleepycat.sample.data.TicketLog;

import java.util.Iterator;

/**
 * DPLTicketLogDAO is a TicketLogDAO implementation using the Direct
 * Persistence Layer API.
 */
public class DPLTicketLogDAO implements TicketLogDAO {
	/* The name of the ticket log store. */
	static final String STORE_NAME = "Log";

	/* The database manager from which this DAO is created. */
	private final DPLDbManager dbMgr;

	/* The ticket log store. */
	private final EntityStore logStore;

	/* The primary key index. */
	private final PrimaryIndex<Long, TicketLog> logTimeIndex;

	/* The secondary key index. */
	private final SecondaryIndex<String, Long, TicketLog> meterIndex;

	/**
	 * Create a data access object for ticket logs.
	 *
	 * @param dbManager the database manager creating this object
	 * @throws DatabaseException on database error
	 */
	public DPLTicketLogDAO(DPLDbManager dbManager) throws DatabaseException {
		dbMgr = dbManager;

		StoreConfig logCfg = new StoreConfig().setTransactional(true);
		logStore = new EntityStore(dbMgr.env, STORE_NAME, logCfg);

		logTimeIndex = logStore.getPrimaryIndex(Long.class, TicketLog.class);
		meterIndex = logStore.getSecondaryIndex(logTimeIndex, String.class, "meterId");
	}

	@Override
	public void close() throws Exception {
		logStore.close();
	}

	@Override
	public void saveLog(TicketLog log) throws Exception {
		logTimeIndex.putNoOverwrite(dbMgr.getCurrentTxn(), log);
	}

	@Override
	public CloseableIterable<TicketLog> queryLog(
			String meterId, long from, long until) throws Exception {
		if (meterId == null) {
			throw new NullPointerException("meterId cannot be null.");
		}

		EntityIndex<Long, TicketLog> logIndex = meterIndex.subIndex(meterId);
		EntityCursor<TicketLog> logCursor = logIndex.entities(
				dbMgr.getCurrentTxn(), from, true, until, false,
				CursorConfig.READ_COMMITTED);

		return new TicketLogIterable(logCursor);
	}

	/* Inner helper class for making an EntityCursor auto-closeable. */
	private static class TicketLogIterable implements CloseableIterable<TicketLog> {
		/* The cursor backing this iterable. */
		private EntityCursor<TicketLog> cursor;

		public TicketLogIterable(EntityCursor<TicketLog> cursor) {
			this.cursor = cursor;
		}

		@Override
		public void close() throws Exception {
			cursor.close();
		}

		@Override
		public Iterator<TicketLog> iterator() {
			return cursor.iterator();
		}
	}
}
