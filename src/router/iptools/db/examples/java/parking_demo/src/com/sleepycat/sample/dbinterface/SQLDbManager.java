/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import java.sql.*;

/**
 * SQLDbManager is a DbManager implementation using the SQL interface.
 */
public class SQLDbManager implements DbManager {
	/* The common prefix for all JDBC urls. */
	private static final String URL_PREFIX = "jdbc:sqlite:/";

	/* The JDBC connection */
	private final Connection conn;

	/* The begin-transaction statement. */
	private final PreparedStatement beginTxnStmt;

	/* The commit-transaction statement. */
	private final PreparedStatement commitStmt;

	/* The abort-transaction statement. */
	private final PreparedStatement abortStmt;

	/**
	 * Create a SQLDbManager connecting to the given database file.
	 *
	 * @param dbFile the database file path
	 * @throws Exception on error
	 */
	public SQLDbManager(String dbFile) throws Exception {
		Class.forName("SQLite.JDBCDriver");
		conn = DriverManager.getConnection(URL_PREFIX + dbFile);
		beginTxnStmt = conn.prepareStatement("BEGIN EXCLUSIVE");
		commitStmt = conn.prepareStatement("COMMIT");
		abortStmt = conn.prepareStatement("ROLLBACK");
	}

	@Override
	public void close() throws SQLException {
		conn.close();
		beginTxnStmt.close();
		commitStmt.close();
		abortStmt.close();
	}

	@Override
	public void setupDb() throws Exception {
		Statement stmt = conn.createStatement();

		stmt.execute("CREATE TABLE IF NOT EXISTS TICKET (" +
				"ID INTEGER PRIMARY KEY," +
				"METER_ID TEXT," +
				"ISSUE_TIME INTEGER)");

		stmt.execute("SELECT create_sequence(\"TicketIdSeq\")");

		stmt.execute("CREATE TABLE IF NOT EXISTS LOG (" +
				"LOG_TIME INTEGER PRIMARY KEY," +
				"METER_ID TEXT," +
				"TICKET_ID INTEGER," +
				"ACTION TEXT," +
				"CHARGE_AMOUNT INTEGER)");

		stmt.execute("CREATE INDEX IF NOT EXISTS LOG_METER_IDX ON LOG (" +
				"METER_ID, LOG_TIME)");

		stmt.close();
	}

	@Override
	public TicketDAO createTicketDAO() throws Exception {
		return new SQLTicketDAO(conn);
	}

	@Override
	public TicketLogDAO createTicketLogDAO() throws Exception {
		return new SQLTicketLogDAO(conn);
	}

	@Override
	public void beginTxn() throws Exception {
		beginTxnStmt.execute();
	}

	@Override
	public void commit() throws Exception {
		commitStmt.execute();
	}

	@Override
	public void abort() throws Exception {
		abortStmt.execute();
	}
}
