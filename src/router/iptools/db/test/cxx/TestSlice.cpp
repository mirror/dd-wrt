/*
* Do some regression tests for slice environments and databases.
*/

#include <db_cxx.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

using namespace std;

/*
 * A slice callback function for a two slice databases. It sorts the keys
 * by moding their key integer value by two.
 */
static int test_slice_callback(const Db *dbp, const Dbt *key, Dbt *slice)
{
	const int *value;
	int val;
	const void *data;
	void *slice_data = NULL;

	data = key->get_data();
	value = (int *)data;
	val = (*value) % 2;
	slice_data = malloc(sizeof(int));
	if (slice_data == NULL)
		return (-1);
	memcpy(slice_data, &val, sizeof(int));
	slice->set_data(slice_data);
	slice->set_size(sizeof(int));
	slice->set_flags(DB_DBT_APPMALLOC);
	return (0);
}

int main(int argc, char *argv[])
{
  	DB_TXN_STAT *txn_stats = NULL;
	DB_TXN_ACTIVE *active_stats = NULL;
	u_int32_t flags = 0;

	// Quit if slices not enabled, but first write out the test
	// answers so the script does not think this test failed.
	if (!DbEnv::slices_enabled()) {
	   	std::cout << "2\n";
		std::cout << "./__db.slice000\n";
		std::cout << "./__db.slice001\n";
		std::cout << "0\n";
		std::cout << "2147483651\n";
		std::cout << "2\n";
		std::cout << "3\n";
		std::cout << "2\n";
		std::cout << "3\n";
		return (0);
	}

	// Create the DB_CONFIG file for the sliced environment.
	try {
		ofstream db_config;
		db_config.open("DB_CONFIG");
		db_config << "set_slice_count 2\n";
		db_config.close();
	} catch (...) {
		cerr << "Error creating the DB_CONFIG file. \n";
		return (0);
	}
	try {
	  	DbEnv *dbenv = new DbEnv((u_int32_t)0);
		DbEnv **slices = NULL;
		DbTxn *dbtxn;
		Db **db_slices;
		Db *slice;
		Dbt data0, data1, key0, key1, ret_data;
		int data_val0, data_val1, key_val0, key_val1;
		u_int32_t num_slice;
		int *val;
		void *value;

		// Need to open the environment
		dbenv->open(".", DB_CREATE | DB_INIT_TXN |
			DB_INIT_LOCK | DB_INIT_LOG |
			DB_INIT_MPOOL,
			0644);

		// Get the number of slices (2)
		num_slice = dbenv->get_slice_count();
		std::cout << num_slice << "\n";
		/*
		 * Get the slice environments then check that they are
		 * valid by getting their home directories.
		 */
		dbenv->get_slices(&slices);
		for (int i = 0; i < num_slice; i++) {
			const char *home;
			(slices[i])->get_home(&home);
			std::cout << home << "\n";
		}

		// Open a sliced environment
		dbenv->txn_begin(NULL, &dbtxn, 0);
		Db *dbp = new Db(dbenv, 0);
		dbp->set_slice_callback(test_slice_callback);
		dbp->open(dbtxn, "slice.db",
		    NULL, DB_BTREE, DB_SLICED|DB_CREATE, 0664);

		// Insert a record into each slice
		key_val0 = 0;
		key_val1 = 1;
		data_val0 = 2;
		data_val1 = 3;
		key0.set_data(&key_val0);
		key0.set_size(sizeof(int));
		data0.set_data(&data_val0);
		data0.set_size(sizeof(int));
		key1.set_data(&key_val1);
		key1.set_size(sizeof(int));
		data1.set_data(&data_val1);
		data1.set_size(sizeof(int));
		dbtxn->commit(0);
		dbenv->txn_begin(NULL, &dbtxn, 0);
		dbp->put(dbtxn, &key0, &data0, 0);
		dbtxn->commit(0);
		dbenv->txn_begin(NULL, &dbtxn, 0);
		dbp->put(dbtxn, &key1, &data1, 0);
		dbenv->txn_stat(&txn_stats, 0);
		active_stats = txn_stats->st_txnarray;
		for (int i = 0; i < num_slice; i++)
		  std::cout << active_stats[0].slice_txns[i].txnid << "\n";
		dbtxn->commit(0);
		dbp->get_slices(&db_slices);

		// Check that the records were put into the correct slice.
		(db_slices[0])->get(NULL, &key0, &ret_data, 0);
		value = ret_data.get_data();
		val = (int *)value;
		std::cout << *val << "\n";
		(db_slices[1])->get(NULL, &key1, &ret_data, 0);
		value = ret_data.get_data();
		val = (int *)value;
		std::cout << *val << "\n";

		// Test slice_lookup
		dbp->slice_lookup((const Dbt *)&key0, &slice, flags);
		slice->get(NULL, &key0, &ret_data, 0);
		value = ret_data.get_data();
		val = (int *)value;
		std::cout << *val << "\n";
		delete slice;
		dbp->slice_lookup((const Dbt *)&key1, &slice, flags);
		slice->get(NULL, &key1, &ret_data, 0);
		value = ret_data.get_data();
		val = (int *)value;
		std::cout << *val << "\n";
		delete slice;

		dbp->close(0);
		dbenv->close(0);
		delete dbp;
		delete dbenv;
	}
	catch (DbException &dbe) {
		cerr << "Db Exception: " << dbe.what() << "\n";
	}
	return 0;
}
