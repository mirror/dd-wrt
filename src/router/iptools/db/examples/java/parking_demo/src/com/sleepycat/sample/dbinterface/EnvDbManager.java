/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.Environment;
import com.sleepycat.db.EnvironmentConfig;
import com.sleepycat.db.Transaction;

import java.io.File;

/**
 * EnvDbManager is the base class for database managers that manage
 * environments.
 * <p/>
 * For the sake of simplicity, this implementation does not support nested
 * transactions.
 */
public abstract class EnvDbManager implements DbManager {
	/* The managed database environment handle. */
	protected final Environment env;

	/* The current open transaction. */
	private Transaction currentTxn;

	/**
	 * Construct a database manager for the database environment at the given
	 * home directory.
	 *
	 * @param envHome the home directory name of the database environment
	 * @throws Exception on error
	 */
	protected EnvDbManager(File envHome) throws Exception {
		// open/create a transactional environment
		EnvironmentConfig envConfig = new EnvironmentConfig();
		envConfig.setAllowCreate(true);
		envConfig.setInitializeCache(true);
		envConfig.setInitializeLocking(true);
		envConfig.setInitializeLogging(true);
		envConfig.setTransactional(true);

		env = new Environment(envHome, envConfig);
		currentTxn = null;
	}

	@Override
	public void close() throws DatabaseException {
		env.close();
	}

	@Override
	public void beginTxn() throws DatabaseException {
		currentTxn = env.beginTransaction(null, null);
	}

	@Override
	public void commit() throws DatabaseException {
		currentTxn.commit();
		currentTxn = null;
	}

	@Override
	public void abort() throws DatabaseException {
		currentTxn.abort();
		currentTxn = null;
	}

	/**
	 * Get the currently opened transaction. Return null if no transaction
	 * is opened.
	 *
	 * @return an open transaction
	 */
	protected Transaction getCurrentTxn() {
		return currentTxn;
	}
}
