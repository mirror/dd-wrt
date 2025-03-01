/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package db;

import com.sleepycat.db.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.OutputStream;
import java.util.Arrays;

/*
 * An example of a program for using the external files interface,
 * which is designed for working with very large records
 * (such as those over 1 megabyte in size).
 *
 * For comparison purposes, this example uses a similar structure
 * as examples/c/ex_external_file.c.
 */
public class ExternalFileExample {
    private static final String progname = "ExternalFileExample";
    private Database db;
    private Environment dbenv;

    public static void main(String[] argv) {
	File envHome = new File("EXTERNALFILE_EX");
	ExternalFileExample example = null;

        for (int i = 0; i < argv.length; ++i) {
            if (argv[i].equals("-h")) {
                if (++i >= argv.length)
                    usage();
                envHome = new File(argv[i]);
            } else
		usage();
        }

        try {
	    // Clear the example directory.
	    clearDirectory(envHome);

	    // Set up the Environment and a database with external files
	    // enabled.
	    example = new ExternalFileExample(envHome);
            
	    System.out.println("Insert external file records");
	    example.insertExternalFiles();

	    System.out.println("Read external files");
	    example.readExternalFiles();
        } catch (DatabaseException dbe) {
            System.err.println(progname + ": environment open: " + dbe.toString());
            System.exit (1);
        } catch (FileNotFoundException fnfe) {
            System.err.println(progname + ": unexpected open environment error  " + fnfe);
            System.exit (1);
        }

	// Close the database and environment.
	if (example != null)
	    example.close();
    }

    public ExternalFileExample(File envHome)
	throws DatabaseException, FileNotFoundException {

	System.out.println("Setup env");
        setupEnvironment(envHome, System.err);

	System.out.println("Create the database");
	createExternalFileDatabase();
    }

    private void setupEnvironment(File home, OutputStream errs)
        throws DatabaseException, FileNotFoundException {

	if (!home.exists())
	    home.mkdirs();
        // Create an environment object and initialize it for error reporting.
        EnvironmentConfig config = new EnvironmentConfig();
        config.setErrorStream(errs);
        config.setErrorPrefix(progname);

        // Increase the cache size to handle the large records.
        config.setCacheSize(64 * 1024 * 1024);

	//
        // Open the environment with full transactional support.
	// External files have full transactional support when it is enabled.
	//
        config.setAllowCreate(true);
        config.setInitializeCache(true);
        config.setTransactional(true);
        config.setInitializeLocking(true);

        dbenv = new Environment(home, config);
    }

    private void createExternalFileDatabase()
        throws DatabaseException, FileNotFoundException {
         
        // Open a database configured for external files.
        DatabaseConfig dbconfig = new DatabaseConfig();

        // The database is DB_BTREE.
        dbconfig.setAllowCreate(true);
        dbconfig.setMode(0644);
        dbconfig.setType(DatabaseType.BTREE);
	dbconfig.setTransactional(true);

	//
	// Enable external files by setting a threshold.  Any record 1000
	// bytes or larger will automatically be stored externally from the
	// database.  The external file threshold must be set at database
	// creation time, and the value will be ignored if set later.
	//
	dbconfig.setExternalFileThreshold(1000);

        db = dbenv.openDatabase(null, "ExternalFileExample.db", null, dbconfig);

    }

    //
    // Once a database is configured to support external files, there are two
    // ways to create an external file.  Insert a record that is larger than the
    // configured external file threashold, or insert a record of any size that
    // is configured as an external file and use DatabaseStream to fill in the
    // data.
    //
    public void insertExternalFiles()
	throws DatabaseException {
	char[] dataBuffer = new char[1001];
	Arrays.fill(dataBuffer, 'a');
	String dataString = new String(dataBuffer);
	String keyBuffer = "1";

	System.out.println("Insert external file key = 1, data = 'aaa...'.");
	//Insert a record that is larger than the threshold (1000 bytes).
	DatabaseEntry data = new DatabaseEntry(dataString.getBytes());
	// Set the key as an integer value.
	DatabaseEntry key = new DatabaseEntry(keyBuffer.getBytes());

	//
	// Begin a transaction and insert the record into the database.  The
	// record will automatically be stored as a single file outside of the
	// database, instead of being broken into peices so it will fit on the
	// database pages.
	//
	Transaction txn = dbenv.beginTransaction(null, null);
	try {
	    db.put(txn, key, data);
	} catch (DatabaseException e) {
	    txn.abort();
	    throw e;
	}
	txn.commit();

	//
	// When streaming large files into the database, it is useful to 
	// configure the record as an external file regardless of size, then
	// fill in the data using DatabaseStream.
	//
	System.out.println("Insert external file key = 2, data = 'bbb...'.");
	data = new DatabaseEntry();
	keyBuffer = "2";
	key = new DatabaseEntry(keyBuffer.getBytes());
	Arrays.fill(dataBuffer, 'b');
	dataString = new String(dataBuffer);

	Cursor cursor = null;
	txn = dbenv.beginTransaction(null, null);
	try {
	    // Use the Cursor to create a zero length external file.
	    cursor = db.openCursor(txn, null);
	    // Configure the entry as an external file.
	    data.setExternalFile(true);
	    cursor.put(key, data);

	    // Create a DatabaseStream from the new record.
	    DatabaseStreamConfig dsConfig = new DatabaseStreamConfig();

	    //
	    // Each DatabaseStream write is flushed to disk to prevent data
	    // loss in case of a crash.  Turning off SyncPerWrite increases
	    // streaming performance but introduces as small risk of data loss
	    // or corruption.
	    //
	    dsConfig.setSyncPerWrite(false);
	    DatabaseStream stream = cursor.openDatabaseStream(dsConfig);
	    data = new DatabaseEntry(dataString.getBytes());

	    // Add data to the file.
	    stream.write(data, 0);
	    // Append even more data to the file.
	    stream.write(data, stream.size());

	    // Close the stream before closing the cursor.
	    stream.close();
	    cursor.close();
	} catch (DatabaseException e) {
	    if (cursor != null)
		cursor.close();
	    txn.abort();
	    throw e;
	}
	txn.commit();
    }

    //
    // External files can be read all at once using the Database or Cursor
    // get functions, or in pieces using DatabaseStream.
    //
    public void readExternalFiles()
	throws DatabaseException {

	// Get the external file using Database.get.
	String keyBuffer = "1";
	DatabaseEntry key = new DatabaseEntry(keyBuffer.getBytes());
	DatabaseEntry data = new DatabaseEntry();
	Transaction txn = dbenv.beginTransaction(null, null);
	try {
	    db.get(txn, key, data, LockMode.DEFAULT);
	} catch (DatabaseException e) {
	    txn.abort();
	    throw e;
	}
	txn.commit();
	keyBuffer = new String(key.getData());
	String dataSample = new String(data.getData(), 0, 10);
	System.out.println("Read external file key = "
	    + keyBuffer + " data = " + dataSample + "...");

	// Get the external file using DatabaseStream.
	Cursor cursor = null;
	keyBuffer = "2";
	data = new DatabaseEntry();
	key = new DatabaseEntry(keyBuffer.getBytes());
	txn = dbenv.beginTransaction(null, null);
	try {
	    // Open a cursor and position it on the external file.
	    cursor = db.openCursor(txn, null);
	    //
	    // Use the Partial API to prevent the cursor from reading any
	    // data from the file when positioning the cursor.
	    //  
	    data.setPartial(0, 0, true);
	    cursor.getSearchKey(key, data, LockMode.DEFAULT);

	    // Open the stream and read the first 10 bytes from the file.
	    DatabaseStream stream = cursor.openDatabaseStream(null);
	    data = new DatabaseEntry();
	    stream.read(data, 0, 10);
	    dataSample = new String(data.getData());
	    System.out.println("Read external file key = "
		+ keyBuffer + " data = " + dataSample + "...");

	    // Close the stream and cursor.
	    stream.close();
	    cursor.close();
	} catch (DatabaseException e) {
	    txn.abort();
	    throw e;
	}
	txn.commit();
    }

    // Close the Database and Environment.
    public void close() {
	if (db != null) {
	    try {
		db.close();
	    } catch (Exception e) {}
	}
	if (dbenv != null) {
	    try {
		dbenv.close();
	    } catch (Exception e) {}
	}
    }

    private static void clearDirectory(File dir) {
	try {
	    if (dir.exists()) {
		final File[] files = dir.listFiles();
		for (File file: files) {
		    if (file.isDirectory())
			clearDirectory(file);
		    file.delete();
		}
	    }
	} catch (Exception e) {}
    }

    private static void usage() {
        System.err.println("usage: java db.ExternalFileExample [-h home]");
        System.exit(1);
    }
}
