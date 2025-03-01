/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.sample.data.Ticket;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * SQLTicketDAO is a TicketDAO implementation using the SQL interface.
 */
public class SQLTicketDAO implements TicketDAO {
	/* The save ticket statement. */
	private final PreparedStatement saveStmt;

	/* The get ticket statement. */
	private final PreparedStatement getStmt;

	/* The delete ticket statement. */
	private final PreparedStatement delStmt;

	/* The get-next-ticket-id statement. */
	private final PreparedStatement idStmt;

	/**
	 * Create a data access object for tickets.
	 *
	 * @param conn an open JDBC connection
	 * @throws SQLException on database error
	 */
	public SQLTicketDAO(Connection conn) throws SQLException {
		saveStmt = conn.prepareStatement(
				"INSERT INTO TICKET(ID, METER_ID, ISSUE_TIME) VALUES (?, ?, ?)");

		getStmt = conn.prepareStatement(
				"SELECT * FROM TICKET WHERE ID = ?");

		delStmt = conn.prepareStatement(
				"DELETE FROM TICKET WHERE ID = ?");

		idStmt = conn.prepareStatement("SELECT nextval(\"TicketIdSeq\")");
	}

	@Override
	public void close() throws Exception {
		saveStmt.close();
		getStmt.close();
		delStmt.close();
		idStmt.close();
	}

	@Override
	public Long saveTicket(Ticket t) throws Exception {
		Long nextId = null;
		try (ResultSet rs = idStmt.executeQuery()) {
			if (rs.first()) {
				nextId = rs.getLong(1);
				saveStmt.setLong(1, nextId);
				saveStmt.setString(2, t.getMeterId());
				saveStmt.setLong(3, t.getIssuingTime());
				saveStmt.executeUpdate();
			}
		}
		return nextId;
	}

	@Override
	public Ticket getTicket(long ticketId) throws Exception {
		getStmt.setLong(1, ticketId);
		try (ResultSet rs = getStmt.executeQuery()) {
			if (rs.first()) {
				return new Ticket(rs.getLong(1), rs.getString(2), rs.getLong(3));
			} else {
				return null;
			}
		}
	}

	@Override
	public void deleteTicket(long ticketId) throws Exception {
		delStmt.setLong(1, ticketId);
		delStmt.executeUpdate();
	}
}
