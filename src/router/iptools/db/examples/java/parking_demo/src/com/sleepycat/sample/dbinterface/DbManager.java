/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

/**
 * DbManagers are factory objects used to create data access objects (DAOs)
 * and manage transactions.
 * <p/>
 * Conceptually, a DbManager manages the environment (or a SQL database if SQL
 * interface is used) holding all databases (or SQL tables) used by the
 * application.
 */
public interface DbManager extends AutoCloseable {
	/**
	 * Create the databases (or SQL tables) and objects necessary for the
	 * application. This method should be called once before the application
	 * runs.
	 *
	 * @throws Exception on error
	 */
	void setupDb() throws Exception;

	/**
	 * Create a data access object for persisting Ticket objects.
	 *
	 * @return a TicketDAO object
	 * @throws Exception on error
	 */
	TicketDAO createTicketDAO() throws Exception;

	/**
	 * Create a data access object for persisting TicketLog objects.
	 *
	 * @return a TicketLogDAO object
	 * @throws Exception on error
	 */
	TicketLogDAO createTicketLogDAO() throws Exception;

	/**
	 * Begin a database transaction.
	 *
	 * @throws Exception on error
	 */
	void beginTxn() throws Exception;

	/**
	 * Commit the most recently opened transaction.
	 *
	 * @throws Exception on error
	 */
	void commit() throws Exception;

	/**
	 * Abort the most recently opened transaction.
	 *
	 * @throws Exception on error
	 */
	void abort() throws Exception;
}
