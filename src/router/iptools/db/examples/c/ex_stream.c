/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_stream -- A basic example with blob handling:
 *	Insert and retrieve key/data pairs with the partial key/value API.
 * 
 *   This example shows how to store blobs in to and retrive it out of a BDB
 *   database. To avoid holding the entire blob in memory, partial put/get is
 *   used to store and retrieve blobs in chunks.
 *
 *   The program first creates a database object, configures its page size 
 *   and cache size, and then opens it for update.
 *
 *   After the database is opened, a cursor is opened within the database.
 *   The program uses the cursor to update partial key/data pairs into the
 *   database, updating a chunk at a time. For the sake of simplicity,
 *   we require that the blob data size must be integeral times of the chunk
 *   size. After the update operation, the program retrieves the blob back,
 *   and again one chunk at a time using partial get.
 *
 *   At the end of the program, the cursor handle and the database handle are closed
 *   respectively.
 *
 * Database: stream.db
 * Program name: ex_stream
 *
 * Options:
 *  -c	set the chunk size.
 *  -d	set the total record size.
 *  -p	set the page size.
 *  -t	choose a database type: btree(b), hash(h) or recno(r).
 *
 * $Id$
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif

#include <db.h>


#define	DATABASE	"stream.db"
#define	CHUNK_SIZE	500
#define	DATA_SIZE	CHUNK_SIZE * 100

int main __P((int, char *[]));
int usage __P((void));
int invarg __P((const char *, int, const char *));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;	/* Set by getopt(): the argv being processed. */
	extern int optind;	/* Set by getopt(): the # of argv's processed. */
	DB *dbp;		/* Handle of the database to be updated. */
	DBC *dbcp;		/* Cursor used for put or get the blob data. */
	DBT key;		/* The key to dbcp->put()/from dbcp->get(). */
	DBT data;		/* The data to dbcp->put()/from dbcp->get(). */
	DBTYPE db_type;		/* The database type. */
	int ch;			/* The current command line option char. */
	int chunk_sz;		/* The size of each chunk to put or get. */
	int chunk_off;		/* The current chunk's offset into the data buffer. */
	int data_sz;		/* The size of the blob data to put or get. */
	int i;			/* The temporary loop variable. */
	int ret;		/* Return code from call into Berkeley DB. */
	int page_sz;		/* Database page size. */
	char *database;		/* The name of the database to use. */
	char *buf;		/* The buffer for the blob data. */
	const char *progname = "ex_stream";		/* Program name. */

	chunk_sz = CHUNK_SIZE;
	data_sz = DATA_SIZE;
	chunk_off = page_sz = 0;
	db_type = DB_BTREE;

	/* Parse the command line arguments. */
	while ((ch = getopt(argc, argv, "c:d:p:t:")) != EOF)
		switch (ch) {
		case 'c':	/* Set the chunk size. */
			if ((chunk_sz = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 'd':	/* Set the total record size. */
			if ((data_sz = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 'p':	/* Set the page size. */
			if ((page_sz = atoi(optarg)) <= 0 ||
			    page_sz % 2 != 0 || page_sz < 512 ||
			    page_sz > 64 * 1024)
				return (invarg(progname, ch, optarg));
				return (invarg(progname, ch, optarg));
			break;
		case 't':	/* Choose a database type: btree(b), hash(h) or recno(r). */
			switch (optarg[0]) {
			case 'b':
				db_type = DB_BTREE;
				break;
			case 'h':
				db_type = DB_HASH;
				break;
			case 'r':
				db_type = DB_RECNO;
				break;
			default:
				return (invarg(progname, ch, optarg));
				break;
			}
			break;
		case '?':	/* Display help messages and exit. */
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	/* Accept optional database name. */
	database = *argv == NULL ? DATABASE : argv[0];

	if (chunk_sz > data_sz) {
		fprintf(stderr,
"Chunk size must be less than and a factor of the data size\n");

		return (usage());
	}

	/* Discard any existing database. */
	(void)remove(database);

	/* Create and initialize database object, open the database. */
	if ((ret = db_create(&dbp, NULL, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_create: %s\n", progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}

	/*
	 * Prefix any error messages with the name of this program and a ':'.
	 * Setting the errfile to stderr is not necessary, since that is the
	 * default; it is provided here as a placeholder showing where one
	 * could direct error messages to an application-specific log file.
	 */

	dbp->set_errfile(dbp, stderr);
	dbp->set_errpfx(dbp, progname);

	/*
	 * Configure the database to use:
	 *	A page size specified via command line options, and
	 *	a database cache size of 0 GB and 64KB, in a single region.
	 *
	 * This cache size is large enough for this trivial program. On a real
	 * production system, the cache size should be set large enough to
	 * hold the working set.
	 */

	if (page_sz != 0 && (ret = dbp->set_pagesize(dbp, page_sz)) != 0) {
		dbp->err(dbp, ret, "set_pagesize");
		goto err1;
	}
	if ((ret = dbp->set_cachesize(dbp, 0, 32 * 1024, 0)) != 0) {
		dbp->err(dbp, ret, "set_cachesize");
		goto err1;
	}
	if ((ret = dbp->open(dbp,
	    NULL, database, NULL, db_type, DB_CREATE, 0664)) != 0) {
		dbp->err(dbp, ret, "%s: open", database);
		goto err1;
	}

	/* Ensure the data size is a multiple of the chunk size. */
	data_sz = data_sz - (data_sz % chunk_sz);

	/* Initialize the key/data pair for a streaming insert. */
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	key.data = &chunk_sz;	/* Our key value does not really matter. */
	key.size = sizeof(int);
	data.ulen = data_sz;	/* Size of total data. */
	data.size = chunk_sz;	/* Size of insert/retrieve data. */
	data.data = buf = malloc(data_sz);

	/*
	 * Set data flags --
	 * DB_DBT_USERMEM: use application-controlled memory to provide alignment. 
	 * DB_DBT_PARTIAL: store and retrieve partial records.
	 */
	data.flags = DB_DBT_USERMEM | DB_DBT_PARTIAL;  

	/* Populate the data with something. */
	for (i = 0; i < data_sz; ++i)
		buf[i] = (char)('a' + i % ('z' - 'a'));

	/* Open a cursor object to update the blob. */
	if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		dbp->err(dbp, ret, "DB->cursor");
		goto err1;
	}

	/* Put the blob using the cursor. */
	for (chunk_off = 0; chunk_off < data_sz; chunk_off += chunk_sz) {
		/* 
		 * Each time, we update chunk_sz bytes of the blob starting
		 * at chunk_off of buf.
		 */
		data.size = chunk_sz;
		/*
		 * Update the chunk. The first update uses DB_KEYFIRST to
		 * locate the key, and all other updates use DB_CURRENT to
		 * update the other parts of the same data item.
		 */
		if ((ret = dbcp->put(dbcp, &key, &data,
		    (chunk_off == 0 ? DB_KEYFIRST : DB_CURRENT)) != 0)) {
			dbp->err(dbp, ret, "DBCursor->put");
			goto err2;
		}
		/* Prepare the offset for the next chunk. */
		data.doff += chunk_sz;
	}
	/* Close the cursor after put. */
	if ((ret = dbcp->close(dbcp)) != 0) {
		dbp->err(dbp, ret, "DBcursor->close");
		goto err1;
	}

	/* Open another cursor to retrieve the data item in chunks. */
	memset(data.data, 0, data.ulen);
	if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		dbp->err(dbp, ret, "DB->cursor");
		goto err1;
	}

	/* Reset the offset, the size of each retrieval and the buffer. */
	data.doff = 0;
	data.dlen = chunk_sz;
	memset(data.data, 0, data.ulen);

	/*
	 * Loop over the item, retrieving a chunk at a time.
	 * The requested chunk will be stored at the doff of data.data.
	 */
	for (chunk_off = 0; chunk_off < data_sz; chunk_off += chunk_sz) {
		if ((ret = dbcp->get(dbcp, &key, &data,
		    (chunk_off == 0 ? DB_SET : DB_CURRENT)) != 0)) {
			dbp->err(dbp, ret, "DBCursor->get");
			goto err2;
		}
		data.doff += chunk_sz;
	}

	if ((ret = dbcp->close(dbcp)) != 0) {
		dbp->err(dbp, ret, "DBcursor->close");
		goto err1;
	}
	if ((ret = dbp->close(dbp, 0)) != 0) {
		fprintf(stderr,
		    "%s: DB->close: %s\n", progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);

err2:	(void)dbcp->close(dbcp);
err1:	(void)dbp->close(dbp, 0);
	return (EXIT_FAILURE);
}

/*
 * invarg --
 *	Describe invalid command line argument, then exit.
 */
int
invarg(progname, arg, str)
	const char *progname;
	int arg;
	const char *str;
{
	(void)fprintf(stderr,
	    "%s: invalid argument for -%c: %s\n", progname, arg, str);
	return (EXIT_FAILURE);
}

/*
 * usage --
 *	Describe this program's command line options, then exit.
 */
int
usage()
{
	(void)fprintf(stderr,
"usage: ex_stream [-c int] [-d int] [-p int] [-t char] [database]\n");
	(void)fprintf(stderr, "Where options are:\n");
	(void)fprintf(stderr, "\t-c set the chunk size.\n");
	(void)fprintf(stderr, "\t-d set the total record size.\n");
	(void)fprintf(stderr,
	    "\t-t choose a database type btree (b), hash (h) or recno (r)\n");

	return (EXIT_FAILURE);
}
