/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.sample.data.TicketLog;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Iterator;

/**
 * SQLTicketLogDAO is a TicketLogDAO implementation using the SQL interface.
 */
public class SQLTicketLogDAO implements TicketLogDAO {
	/* The save-log statement. */
	private final PreparedStatement saveStmt;

	/* The query-log statement. */
	private final PreparedStatement queryStmt;

	/**
	 * Create a data access object for ticket logs.
	 *
	 * @param conn an open JDBC connection
	 * @throws SQLException on database error
	 */
	public SQLTicketLogDAO(Connection conn) throws SQLException {
		saveStmt = conn.prepareStatement("INSERT INTO LOG" +
				"(LOG_TIME, METER_ID, TICKET_ID, ACTION, CHARGE_AMOUNT)" +
				"VALUES (?, ?, ?, ?, ?)");

		queryStmt = conn.prepareStatement("SELECT * FROM LOG " +
				"WHERE METER_ID = ? AND LOG_TIME >= ? AND LOG_TIME < ?");
	}

	@Override
	public void close() throws Exception {
		saveStmt.close();
		queryStmt.close();
	}

	@Override
	public void saveLog(TicketLog log) throws Exception {
		saveStmt.setLong(1, log.getLogTime());
		saveStmt.setString(2, log.getMeterId());
		saveStmt.setLong(3, log.getTicketId());
		saveStmt.setString(4, log.getAction().name());
		saveStmt.setInt(5, log.getChargeAmountInCents());
		saveStmt.executeUpdate();
	}

	@Override
	public CloseableIterable<TicketLog> queryLog(String meterId, long from, long until) throws Exception {
		queryStmt.setString(1, meterId);
		queryStmt.setLong(2, from);
		queryStmt.setLong(3, until);

		return new TicketLogIterable(queryStmt.executeQuery());
	}

	/* Inner helper class for making a ResultSet iterable. */
	private static class TicketLogIterable implements CloseableIterable<TicketLog> {
		/* The JDBC result set backing this iterable. */
		private final ResultSet rs;

		public TicketLogIterable(ResultSet rs) {
			this.rs = rs;
		}

		@Override
		public void close() throws Exception {
			rs.close();
		}

		@Override
		public Iterator<TicketLog> iterator() {
			try {
				rs.beforeFirst();
			} catch (SQLException e) {
				throw new RuntimeException(e);
			}

			return new Iterator<TicketLog>() {
				@Override
				public boolean hasNext() {
					try {
						return !rs.isLast();
					} catch (SQLException e) {
						throw new RuntimeException(e);
					}
				}

				@Override
				public TicketLog next() {
					try {
						if (rs.next()) {
							return createLog();
						} else {
							return null;
						}
					} catch (SQLException e) {
						throw new RuntimeException(e);
					}
				}

				/* Create a log message using values at the current row. */
				private TicketLog createLog() throws SQLException {
					return new TicketLog(rs.getLong(1), rs.getString(2),
							rs.getLong(3),
							TicketLog.Action.valueOf(rs.getString(4)),
							rs.getInt(5));
				}

				@Override
				public void remove() {
					throw new UnsupportedOperationException();
				}
			};
		}
	}
}
