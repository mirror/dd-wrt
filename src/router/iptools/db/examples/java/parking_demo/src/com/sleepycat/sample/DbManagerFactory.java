/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample;

import com.sleepycat.sample.dbinterface.BasicDbManager;
import com.sleepycat.sample.dbinterface.DPLDbManager;
import com.sleepycat.sample.dbinterface.DbManager;
import com.sleepycat.sample.dbinterface.SQLDbManager;

import java.io.File;
import java.io.IOException;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.attribute.BasicFileAttributes;

/**
 * DbManagerFactory is a factory class for creating DbManagers.
 * <p/>
 * This is a helper class for this sample application, as it contains specific
 * logic to locate directories for database files of various implementations.
 */
public class DbManagerFactory {
	/**
	 * Names of the different APIs used to implement DbManagers.
	 */
	public enum API {
		BASE,
		DPL,
		SQL
	}

	/* The database name for the SQL implementation. */
	private static final String SQL_DB_NAME = "park.db";

	/* The common root directory used to host environments or SQL data file. */
	private final File homeDir;

	/* The API in which all DbManagers created by this factory are implemented. */
	private final API api;

	/**
	 * Create a DbManagerFactory object.
	 *
	 * @param rootDir the parent root directory for environments
	 * @param api     the implementation api of created DbManagers
	 * @throws IOException on I/O error
	 */
	public DbManagerFactory(String rootDir, API api) throws IOException {
		this.api = api;

		/*
		 * For base API, the environment home directory is <rootDir>/base
		 * For DPL API, the environment home directory is <rootDir>/dpl
		 * For SQL API, the root directory of the database file is <rootDir>/sql
		 */
		this.homeDir =
				new File(rootDir + File.separator + api.name().toLowerCase());

		createHomeDir();
	}

	/**
	 * Create a new DbManager.
	 *
	 * @return a new DbManager
	 * @throws Exception on error
	 */
	public DbManager createManager() throws Exception {
		switch (api) {
			case BASE:
				return new BasicDbManager(homeDir);
			case DPL:
				return new DPLDbManager(homeDir);
			case SQL:
				File dbFile = new File(homeDir, SQL_DB_NAME);
				return new SQLDbManager(dbFile.getPath());
			default:
				return null;
		}
	}

	/**
	 * Create the environment home directory or the root directory for SQL
	 * database files.
	 */
	private void createHomeDir() throws IOException {
		if (!homeDir.exists()) {
			homeDir.mkdirs();
		} else if (api == API.SQL) {
			// The SQL DDL statements in SQLDbManager.setupDb cannot be
			// executed multiple times on the same database. Neither can we
			// insert TicketLogs with the same LOG_TIME into the same database.
			// Therefore, the sample application cannot run as is multiple
			// times on the same database without first clean up it.
			cleanDir(homeDir);
		}
	}

	/**
	 * Remove all entries (files and sub-directories) in the given directory.
	 *
	 * @param rootDir the directory to be cleaned up
	 * @throws IOException on I/O error
	 */
	private void cleanDir(final File rootDir) throws IOException {
		Files.walkFileTree(rootDir.toPath(), new SimpleFileVisitor<Path>() {
			@Override
			public FileVisitResult visitFile(
					Path file, BasicFileAttributes attrs) throws IOException {
				Files.delete(file);
				return FileVisitResult.CONTINUE;
			}

			@Override
			public FileVisitResult postVisitDirectory(
					Path dir, IOException exc) throws IOException {
				if (exc == null) {
					if (!dir.equals(rootDir.toPath())) {
						Files.delete(dir);
					}
					return FileVisitResult.CONTINUE;
				} else {
					throw exc;
				}
			}
		});
	}
}
