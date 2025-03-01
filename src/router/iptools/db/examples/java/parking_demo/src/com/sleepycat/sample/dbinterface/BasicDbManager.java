/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.db.*;

import java.io.File;

/**
 * BasicDbManager is a DbManager implementation using the base API.
 */
public class BasicDbManager extends EnvDbManager {
	/**
	 * Construct a BasicDbManager given the environment home directory.
	 *
	 * @param envHome the environment home directory
	 * @throws Exception on error
	 */
	public BasicDbManager(File envHome) throws Exception {
		super(envHome);
	}

	@Override
	public void setupDb() throws Exception {
		Database ticketDb = createDb(BasicTicketDAO.DB_NAME);
		createSeq(ticketDb, BasicTicketDAO.ID_SEQ_KEY).close();
		ticketDb.close();

		Database logDb = createDb(BasicTicketLogDAO.DB_NAME);
		createIndex(BasicTicketLogDAO.IDX_DB_NAME,
				BasicTicketLogDAO.KEY_CREATOR, logDb).close();
		logDb.close();
	}

	/**
	 * Create/Open a database given its file name.
	 *
	 * @param fileName the database file name
	 * @return an open Database
	 * @throws Exception on error
	 */
	private Database createDb(String fileName) throws Exception {
		DatabaseConfig dbConfig = new DatabaseConfig();
		dbConfig.setAllowCreate(true);
		dbConfig.setTransactional(true);
		dbConfig.setType(DatabaseType.BTREE);
		return env.openDatabase(null, fileName, null, dbConfig);
	}

	/**
	 * Create/Open a sequence given its key.
	 *
	 * @param db  the database holding the sequence
	 * @param key the key of the sequence
	 * @return an open Sequence
	 * @throws DatabaseException on database error
	 */
	private Sequence createSeq(Database db, DatabaseEntry key)
			throws DatabaseException {
		SequenceConfig seqConfig = new SequenceConfig();
		seqConfig.setAllowCreate(true);
		return db.openSequence(null, key, seqConfig);
	}

	/**
	 * Create/Open a secondary index database given its file name and primary
	 * database.
	 *
	 * @param fileName the database file name
	 * @param indexKey the index key creator
	 * @param db       the primary database
	 * @return an open secondary database
	 * @throws DatabaseException on database error
	 */
	private SecondaryDatabase createIndex(
			String fileName, SecondaryKeyCreator indexKey, Database db)
			throws Exception {
		SecondaryConfig idxConfig = new SecondaryConfig();
		idxConfig.setAllowCreate(true);
		idxConfig.setTransactional(true);
		idxConfig.setType(DatabaseType.BTREE);
		idxConfig.setSortedDuplicates(true);
		idxConfig.setKeyCreator(indexKey);
		return env.openSecondaryDatabase(null, fileName, null, db, idxConfig);
	}

	@Override
	public TicketDAO createTicketDAO() throws Exception {
		return new BasicTicketDAO(this);
	}

	@Override
	public TicketLogDAO createTicketLogDAO() throws Exception {
		return new BasicTicketLogDAO(this);
	}
}
