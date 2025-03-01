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
import com.sleepycat.sample.data.TicketLog;

import java.util.Iterator;

/**
 * BasicTicketLogDAO is a TicketLogDAO implementation using the base API.
 */
public class BasicTicketLogDAO implements TicketLogDAO {
	/* The file name of the ticket log database. */
	static final String DB_NAME = "ticket_log.db";

	/* The file name of the secondary meter id index database. */
	static final String IDX_DB_NAME = "meter_idx.sdb";

	/* The index key creator. */
	static final MeterIdKeyCreator KEY_CREATOR = new MeterIdKeyCreator();

	/* The database manager from which this DAO is created. */
	private final BasicDbManager dbMgr;

	/* The ticket log database handle. */
	private final Database logDb;

	/* The meter id index database handle. */
	private final SecondaryDatabase meterIdIdxDb;

	/* The binding for key database entries. */
	private final TupleBinding<Long> keyBinding;

	/* The binding for value database entries. */
	private final LogBinding valueBinding;

	/* The binding for index key database entries. */
	private final TupleBinding<String> idxKeyBinding;

	/**
	 * Create a data access object for ticket logs.
	 *
	 * @param dbManager the database manager creating this object
	 * @throws Exception on error
	 */
	public BasicTicketLogDAO(BasicDbManager dbManager) throws Exception {
		dbMgr = dbManager;

		DatabaseConfig dbConfig = new DatabaseConfig();
		dbConfig.setTransactional(true);
		dbConfig.setType(DatabaseType.BTREE);
		logDb = dbMgr.env.openDatabase(null, DB_NAME, null, dbConfig);

		SecondaryConfig idxConfig = new SecondaryConfig();
		idxConfig.setTransactional(true);
		idxConfig.setType(DatabaseType.BTREE);
		idxConfig.setSortedDuplicates(true);
		idxConfig.setKeyCreator(KEY_CREATOR);
		meterIdIdxDb = dbMgr.env.openSecondaryDatabase(null, IDX_DB_NAME,
				null, logDb, idxConfig);

		keyBinding = TupleBinding.getPrimitiveBinding(Long.class);
		valueBinding = new LogBinding();
		idxKeyBinding = TupleBinding.getPrimitiveBinding(String.class);
	}

	@Override
	public void close() throws Exception {
		meterIdIdxDb.close();
		logDb.close();
	}

	@Override
	public void saveLog(TicketLog log) throws Exception {
		DatabaseEntry key = new DatabaseEntry();
		DatabaseEntry value = new DatabaseEntry();

		keyBinding.objectToEntry(log.getLogTime(), key);
		valueBinding.objectToEntry(log, value);

		logDb.putNoOverwrite(dbMgr.getCurrentTxn(), key, value);
	}

	@Override
	public CloseableIterable<TicketLog> queryLog(
			String meterId, long from, long until) throws Exception {
		SecondaryCursor cursor =
				meterIdIdxDb.openSecondaryCursor(dbMgr.getCurrentTxn(), null);

		return new TicketLogIterable(cursor, meterId, from, until);
	}

	/**
	 * Helper class to convert between ticket logs and byte arrays.
	 */
	private static class LogBinding extends TupleBinding<TicketLog> {
		@Override
		public TicketLog entryToObject(TupleInput in) {
			return new TicketLog(in.readLong(), in.readString(), in.readLong(),
					TicketLog.Action.valueOf(in.readString()), in.readInt());
		}

		@Override
		public void objectToEntry(TicketLog ticketLog, TupleOutput out) {
			out.writeLong(ticketLog.getLogTime());
			out.writeString(ticketLog.getMeterId());
			out.writeLong(ticketLog.getTicketId());
			out.writeString(ticketLog.getAction().name());
			out.writeInt(ticketLog.getChargeAmountInCents());
		}
	}

	/**
	 * Helper class to create the index key on meter id.
	 */
	private static class MeterIdKeyCreator implements SecondaryKeyCreator {
		/* The helper log binding object. */
		private LogBinding logBinding = new LogBinding();

		/* The helper index binding object. */
		private TupleBinding<String> idxBinding =
				TupleBinding.getPrimitiveBinding(String.class);

		@Override
		public boolean createSecondaryKey(
				SecondaryDatabase db, DatabaseEntry key, DatabaseEntry data,
				DatabaseEntry result) throws DatabaseException {
			TicketLog log = logBinding.entryToObject(data);
			idxBinding.objectToEntry(log.getMeterId(), result);
			return true;
		}
	}

	/**
	 * A helper class for converting a secondary cursor into an iterable.
	 */
	private class TicketLogIterable implements CloseableIterable<TicketLog> {
		/* The index cursor. */
		private final SecondaryCursor cursor;

		/* The meter id to search for. */
		private final String meterId;

		/* The upper bound of the returned range. */
		private final long until;

		/* The cached next value. */
		private TicketLog nextVal;

		/* Create a ticket log iterable. */
		public TicketLogIterable(SecondaryCursor cursor,
		                         String meterId, long from, long until)
				throws DatabaseException {
			this.cursor = cursor;
			this.meterId = meterId;
			this.until = until;

			DatabaseEntry idxKey = new DatabaseEntry();
			DatabaseEntry pKey = new DatabaseEntry();
			DatabaseEntry ticketVal = new DatabaseEntry();

			idxKeyBinding.objectToEntry(meterId, idxKey);
			keyBinding.objectToEntry(from, pKey);

			OperationStatus s = this.cursor.getSearchBothRange(
					idxKey, pKey, ticketVal, LockMode.READ_COMMITTED);

			setNextVal(s, ticketVal);
		}

		/* Cache the next value according to cursor operation result. */
		private void setNextVal(OperationStatus s, DatabaseEntry e) {
			if (s == OperationStatus.SUCCESS) {
				nextVal = valueBinding.entryToObject(e);
			} else {
				nextVal = null;
			}
		}

		@Override
		public void close() throws Exception {
			cursor.close();
		}

		@Override
		public Iterator<TicketLog> iterator() {
			return new Iterator<TicketLog>() {
				@Override
				public boolean hasNext() {
					return nextVal != null;
				}

				@Override
				public TicketLog next() {
					TicketLog current = nextVal;

					DatabaseEntry idxKey = new DatabaseEntry();
					DatabaseEntry pKey = new DatabaseEntry();
					DatabaseEntry ticketVal = new DatabaseEntry();

					try {
						OperationStatus s = cursor.getNextDup(
								idxKey, pKey, ticketVal, LockMode.READ_COMMITTED);

						setNextVal(s, ticketVal);
						if (nextVal != null && nextVal.getLogTime() >= until) {
							nextVal = null;
						}
					} catch (DatabaseException e) {
						throw new RuntimeException(e);
					}

					return current;
				}

				@Override
				public void remove() {
				}
			};
		}
	}
}
