/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.dbinterface;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.persist.EntityStore;
import com.sleepycat.persist.StoreConfig;

import java.io.File;

/**
 * DPLDbManager is a DbManager implementation using the Direct Persistence
 * Layer API.
 */
public class DPLDbManager extends EnvDbManager {
	/**
	 * Construct a DPLDbManager given the environment home directory.
	 *
	 * @param envHome the home directory name of the database environment
	 * @throws Exception on error
	 */
	public DPLDbManager(File envHome) throws Exception {
		super(envHome);
	}

	@Override
	public void setupDb() throws DatabaseException {
		EntityStore ticketStore = createStore(DPLTicketDAO.STORE_NAME);
		ticketStore.getSequence("TicketIdSeq");
		ticketStore.close();

		createStore(DPLTicketLogDAO.STORE_NAME).close();
	}

	/**
	 * Create/Open an EntityStore given its name.
	 *
	 * @param name the store name
	 * @return an EntityStore
	 * @throws DatabaseException on database error
	 */
	private EntityStore createStore(String name) throws DatabaseException {
		StoreConfig ticketCfg = new StoreConfig();
		ticketCfg.setAllowCreate(true).setTransactional(true);
		return new EntityStore(env, name, ticketCfg);
	}

	@Override
	public TicketDAO createTicketDAO() throws DatabaseException {
		return new DPLTicketDAO(this);
	}

	@Override
	public TicketLogDAO createTicketLogDAO() throws DatabaseException {
		return new DPLTicketLogDAO(this);
	}
}
