/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include <iostream>
#include <errno.h>

#include <db_cxx.h>
#include "RepWrforConfigInfo.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::flush;

#define CACHESIZE (10 * 1024 * 1024)
#define DATABASE "quote.db"
#define SLEEPTIME 3

const char *progname = "excxx_repquote_gsg_wrfor";

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#define    sleep(s)        Sleep(1000 * (s))

extern "C" {
  extern int getopt(int, char * const *, const char *);
  extern char *optarg;
}
#endif

class RepMgrWrforGSG
{
public:
    RepMgrWrforGSG();
    int init(RepConfigInfo* config);
    int doloop();
    int terminate();

private:
    // Disable copy constructor.
    RepMgrWrforGSG(const RepMgrWrforGSG &);
    void operator = (const RepMgrWrforGSG &);

    // Internal data members.
    RepConfigInfo   *app_config;
    DbEnv           dbenv;

    // Private methods.
    static int print_stocks(Db *dbp);
};

static void usage()
{
    cerr << "usage: " << progname << endl
        << "-h home -l|-L host:port [-r host:port]" << endl;

    cerr 
        << "\t -h home directory (required)" << endl
        << "\t -l host:port (required unless -L is specified;"
        << "\t    l stands for local)" << endl
        << "\t -L host:port (optional, L means group creator)" << endl
        << "\t -r host:port (optional; r stands for remote; any "
        << "number of these" << endl
        << "\t    may be specified)" << endl;

    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    RepConfigInfo config;
    char ch, *last_colon, *portstr, *tmphost;
    int tmpport;
    int ret;

    // Extract the command line parameters.
    while ((ch = getopt(argc, argv, "h:l:L:r:")) != EOF) {
        switch (ch) {
        case 'h':
            config.home = optarg;
            break;
        case 'L':
            config.this_host.creator = true; // FALLTHROUGH
        case 'l':
            config.this_host.host = optarg;
            /*
             * The final colon in host:port string is the
             * boundary between the host and the port portions
             * of the string.
             */
            if ((last_colon = strrchr(optarg, ':')) == NULL) {
                cerr << "Bad local host specification." << endl;
                usage();
            }
            /*
             * Separate the host and port portions of the 
             * string for further processing.
             */
            portstr = last_colon + 1;
            *last_colon = '\0';
            config.this_host.port = (unsigned short)atoi(portstr);
            config.got_listen_address = true;
            break;
        case 'r':
            tmphost = optarg;
            /*
             * The final colon in host:port string is the 
             * boundary between the host and the port portions
             * of the string.
             */
            if ((last_colon = strrchr(tmphost, ':')) == NULL) {
                cerr << "Bad remote host specification." << endl;
                usage();
            }
            /*
             * Separate the host and port portions of the 
             * string for further processing.
             */
            portstr = last_colon + 1;
            *last_colon = '\0';
            tmpport = (unsigned short)atoi(portstr);
            config.addOtherHost(tmphost, tmpport);
            break;
        case '?':
        default:
            usage();
        }
    }

    // Error check command line.
    if ((!config.got_listen_address) || config.home == NULL)
        usage();

    RepMgrWrforGSG runner;
    try {
        if((ret = runner.init(&config)) != 0)
            goto err;
        if((ret = runner.doloop()) != 0)
            goto err;
    } catch (DbException dbe) {
        cerr << "Caught an exception during initialization or"
            << " processing: " << dbe.what() << endl;
    }
err:
    runner.terminate();
    return 0;
}

RepMgrWrforGSG::RepMgrWrforGSG() : app_config(0), dbenv((u_int32_t)0)
{
}

int RepMgrWrforGSG::init(RepConfigInfo *config)
{
    int ret = 0;

    app_config = config;

    dbenv.set_errfile(stderr);
    dbenv.set_errpfx(progname);

    DbSite *dbsite;
    dbenv.repmgr_site(app_config->this_host.host,
        app_config->this_host.port, &dbsite, 0);
    dbsite->set_config(DB_LOCAL_SITE, 1);
    if (app_config->this_host.creator)
        dbsite->set_config(DB_GROUP_CREATOR, 1);

    dbsite->close();

    int i = 1;
    for ( REP_HOST_INFO *cur = app_config->other_hosts;
        cur != NULL && i <= app_config->nrsites;
        cur = cur->next, i++) {

        dbenv.repmgr_site(cur->host, cur->port, &dbsite, 0);
        dbsite->set_config(DB_BOOTSTRAP_HELPER, 1);

        dbsite->close();
    }

    // We can now open our environment, although we're not ready to
    // begin replicating.  However, we want to have a dbenv around
    // so that we can send it into any of our message handlers.
    dbenv.set_cachesize(0, CACHESIZE, 0);
    dbenv.set_flags(DB_TXN_NOSYNC, 1);

    try {
        dbenv.open(app_config->home, DB_CREATE | DB_RECOVER |
            DB_THREAD | DB_INIT_REP | DB_INIT_LOCK | DB_INIT_LOG | 
            DB_INIT_MPOOL | DB_INIT_TXN, 0);
    } catch(DbException dbe) {
        cerr << "Caught an exception during DB environment open." << endl
            << "Ensure that the home directory is created prior to starting"
            << " the application." << endl;
        ret = ENOENT;
        goto err;
    }

    /* Configure Replication Manager write forwarding. */
    dbenv.rep_set_config(DB_REPMGR_CONF_FORWARD_WRITES, 1);

    if ((ret = dbenv.repmgr_start(3, app_config->start_policy)) != 0)
        goto err;

err:
    return ret;
}

int RepMgrWrforGSG::terminate()
{
    try {
        dbenv.close(0);
    } catch (DbException dbe) {
        cerr << "error closing environment: " << dbe.what() << endl;
    }
    return 0;
}

// Provides the main data processing function for our application.
// This function provides a command line prompt to which the user
// can provide a ticker string and a stock price.  Once a value is
// entered to the application, the application writes the value to
// the database and then displays the entire database.
#define BUFSIZE 1024
int RepMgrWrforGSG::doloop()
{
    Dbt key, data;
    Db *dbp;
    char buf[BUFSIZE], *rbuf;
    int ret;

    dbp = 0;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    ret = 0;

    for (;;) {
        if (dbp == 0) {
            dbp = new Db(&dbenv, 0);

            try {
                dbp->open(NULL, DATABASE, NULL, DB_BTREE,
                    app_config->this_host.creator ? DB_CREATE | DB_AUTO_COMMIT :
                    DB_AUTO_COMMIT, 0);
            } catch(DbException dbe) {
                // It is expected that this condition will be triggered
                // when client sites start up.  It can take a while for 
                // the master site to be found and synced, and no DB will
                // be available until then.
                if (dbe.get_errno() == ENOENT) {
                    cout << "No stock db available yet - retrying." << endl;
                    try {
                        dbp->close(0);
                    } catch (DbException dbe2) {
                        cout << "Unexpected error closing after failed" <<
                            " open, message: " << dbe2.what() << endl;
                        dbp = NULL;
                        goto err;
                    }
                    dbp = NULL;
                    sleep(SLEEPTIME);
                    continue;
                } else {
                    dbenv.err(ret, "DB->open");
                    throw dbe;
                }
            }
        }

        cout << "QUOTESERVER" ;
        cout << "> " << flush;

        if (fgets(buf, sizeof(buf), stdin) == NULL)
            break;
        if (strtok(&buf[0], " \t\n") == NULL) {
            switch ((ret = print_stocks(dbp))) {
            case 0:
                continue;
            case DB_REP_HANDLE_DEAD:
                (void)dbp->close(DB_NOSYNC);
                cout << "closing db handle due to rep handle dead" << endl;
                dbp = NULL;
                continue;
            default:
                dbp->err(ret, "Error traversing data");
                goto err;
            }
        }
        rbuf = strtok(NULL, " \t\n");
        if (rbuf == NULL || rbuf[0] == '\0') {
            if (strncmp(buf, "exit", 4) == 0 ||
                strncmp(buf, "quit", 4) == 0)
                break;
            dbenv.errx("Format: TICKER VALUE");
            continue;
        }

        key.set_data(buf);
        key.set_size((u_int32_t)strlen(buf));

        data.set_data(rbuf);
        data.set_size((u_int32_t)strlen(rbuf));

        if ((ret = dbp->put(NULL, &key, &data, 0)) != 0)
        {
            dbp->err(ret, "DB->put");
            switch (ret) {
            case DB_REP_HANDLE_DEAD:
                /* Must close and reopen the handle, then can retry. */
                (void)dbp->close(0); 
                dbp = NULL;
                /* FALLTHROUGH */
            case DB_LOCK_DEADLOCK:
            case DB_TIMEOUT:
            case EACCES:
                /*
                 * Simply retry after a deadlock, timeout or permission error.
                 * A forwarded put operation can return a timeout error if
                 * the operation takes too long.  A forwarded put operation
                 * can return a permission error if there is currently no
                 * master.
                 */
                dbenv.errx("Could not update data, retry operation");
            case DB_KEYEXIST:
                continue;
            default:
                dbp->err(ret, "Error updating data");
                goto err;
            }
        }
    }

err:    if (dbp != 0)
            (void)dbp->close(DB_NOSYNC);

    return (ret);
}

// Display all the stock quote information in the database.
int RepMgrWrforGSG::print_stocks(Db *dbp)
{
    Dbc *dbc;
    Dbt key, data;
#define    MAXKEYSIZE    10
#define    MAXDATASIZE    20
    char keybuf[MAXKEYSIZE + 1], databuf[MAXDATASIZE + 1];
    int ret, t_ret;
    u_int32_t keysize, datasize;

     if ((ret = dbp->cursor(NULL, &dbc, 0)) != 0) {
        dbp->err(ret, "can't open cursor");
        return (ret);
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    cout << "\tSymbol\tPrice" << endl
        << "\t======\t=====" << endl;

    for (ret = dbc->get(&key, &data, DB_FIRST);
        ret == 0;
        ret = dbc->get(&key, &data, DB_NEXT)) {
        keysize = key.get_size() > MAXKEYSIZE ? MAXKEYSIZE : key.get_size();
        memcpy(keybuf, key.get_data(), keysize);
        keybuf[keysize] = '\0';

        datasize = data.get_size() >=
            MAXDATASIZE ? MAXDATASIZE : data.get_size();
        memcpy(databuf, data.get_data(), datasize);
        databuf[datasize] = '\0';

        cout << "\t" << keybuf << "\t" << databuf << endl;
    }
    cout << endl << flush;

    if ((t_ret = dbc->close()) != 0 && ret == 0) {
        cout << "closed cursor" << endl;
        ret = t_ret;
    }

    switch (ret) {
    case 0:
    case DB_NOTFOUND:
    case DB_LOCK_DEADLOCK:
        return (0);
    default:
        return (ret);
    }
}

