/*
 * out_db.c		Database Output
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 *                                     NOTE
 *
 * This whole code is a realy mess and written in a hurry, most interactions
 * should be cached to save traffic. It really is meant as experimental
 * features.
 */

#include <bmon/bmon.h>
#include <bmon/node.h>
#include <bmon/output.h>
#include <bmon/graph.h>
#include <bmon/input.h>
#include <bmon/utils.h>

#include <dbi/dbi.h>

static char *c_driverdir = NULL;
static char *c_driver = "mysql";
static char *c_host = "localhost";
static char *c_username = "bmon";
static char *c_dbname = "bmon";
static char *c_mask = "mhd";
static char *c_password;
static int   c_interval = 3;

static int do_read;
static int do_sec;
static int do_min;
static int do_hour;
static int do_day;

static dbi_conn db_conn;

static int create_tables(void)
{
	dbi_result r;

	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS nodes (" \
	"	id INT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE," \
	"	name TEXT NOT NULL," \
	"	source TEXT," \
	"	PRIMARY KEY (id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);
	
	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS items (" \
	"	id INT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE," \
	"	name TEXT NOT NULL," \
	"       description TEXT," \
	"	node INT UNSIGNED NOT NULL," \
	"	handle INT UNSIGNED," \
	"	parent INT UNSIGNED," \
	"	indent INT UNSIGNED," \
	"       rx_usage SMALLINT NOT NULL," \
	"       tx_usage SMALLINT NOT NULL," \
	"	PRIMARY KEY (id)," \
	"	FOREIGN KEY (node) REFERENCES nodes(id)," \
	"	FOREIGN KEY (parent) REFERENCES items(id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);

	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS attrs (" \
	"	id INT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE," \
	"	name CHAR(15) NOT NULL," \
	"	item INT UNSIGNED NOT NULL," \
	"	rx_rate INT UNSIGNED," \
	"	tx_rate INT UNSIGNED," \
	"	rx_counter BIGINT UNSIGNED NOT NULL," \
	"	tx_counter BIGINT UNSIGNED NOT NULL," \
	"	PRIMARY KEY (id)," \
	"	FOREIGN KEY (item) REFERENCES items(id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);

	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS attr_desc (" \
	"	id CHAR(15) NOT NULL UNIQUE," \
	"	is_num SMALLINT UNSIGNED NOT NULL," \
	"	txt TEXT NOT NULL," \
	"	PRIMARY KEY(id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);

	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS hist_r (" \
	"	attr INT UNSIGNED NOT NULL," \
	"	ts INT UNSIGNED NOT NULL," \
	"	offset INT UNSIGNED NOT NULL," \
	"	rx_rate INT UNSIGNED NOT NULL," \
	"	tx_rate INT UNSIGNED NOT NULL," \
	"	PRIMARY KEY (attr, ts, offset)," \
	"	FOREIGN KEY (attr) REFERENCES attrs(id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);

	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS hist_s (" \
	"	attr INT UNSIGNED NOT NULL," \
	"	ts INT UNSIGNED NOT NULL," \
	"	offset INT UNSIGNED NOT NULL," \
	"	rx_rate INT UNSIGNED NOT NULL," \
	"	tx_rate INT UNSIGNED NOT NULL," \
	"	PRIMARY KEY (attr, ts, offset)," \
	"	FOREIGN KEY (attr) REFERENCES attrs(id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);

	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS hist_m (" \
	"	attr INT UNSIGNED NOT NULL," \
	"	ts INT UNSIGNED NOT NULL," \
	"	offset INT UNSIGNED NOT NULL," \
	"	rx_rate INT UNSIGNED NOT NULL," \
	"	tx_rate INT UNSIGNED NOT NULL," \
	"	PRIMARY KEY (attr, ts, offset)," \
	"	FOREIGN KEY (attr) REFERENCES attrs(id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);

	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS hist_h (" \
	"	attr INT UNSIGNED NOT NULL," \
	"	ts INT UNSIGNED NOT NULL," \
	"	offset INT UNSIGNED NOT NULL," \
	"	rx_rate INT UNSIGNED NOT NULL," \
	"	tx_rate INT UNSIGNED NOT NULL," \
	"	PRIMARY KEY (attr, ts, offset)," \
	"	FOREIGN KEY (attr) REFERENCES attrs(id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);

	r = dbi_conn_query(db_conn,
	"CREATE TABLE IF NOT EXISTS hist_d (" \
	"	attr INT UNSIGNED NOT NULL," \
	"	ts INT UNSIGNED NOT NULL," \
	"	offset INT UNSIGNED NOT NULL," \
	"	rx_rate INT UNSIGNED NOT NULL," \
	"	tx_rate INT UNSIGNED NOT NULL," \
	"	PRIMARY KEY (attr, ts, offset)," \
	"	FOREIGN KEY (attr) REFERENCES attrs(id)" \
	")");

	if (!r)
		return 0;
	else
		dbi_result_free(r);

	return 1;
}

static int insert_attr_desc(struct attr_type *a, void *arg)
{
	dbi_result r = dbi_conn_queryf(db_conn,
	    "SELECT id FROM attr_desc WHERE id = '%s'", a->name);

	if (!r)
		return -1;

	if (dbi_result_first_row(r) == 1) {
		dbi_result_free(r);
		return 0;
	}

	dbi_result_free(r);

	r = dbi_conn_queryf(db_conn,
	    "INSERT INTO attr_desc (id, is_num, txt) VALUES " \
	    "('%s', %d, '%s')", a->name, a->unit == U_NUMBER,
	    a->desc);

	if (!r)
		return -1;

	dbi_result_free(r);
	return 0;
}

static int insert_attr_descs(void)
{
	return !foreach_attr_type(&insert_attr_desc, NULL);
}

static void add_history(const char *table, int attr_id, timestamp_t *ts,
			rate_cnt_t rx, rate_cnt_t tx)
{
	dbi_result r = dbi_conn_queryf(db_conn,
		"INSERT INTO %s (attr, ts, offset, rx_rate, tx_rate) " \
		"values (%d, %u, %u, %u, %u)",
		table, attr_id, (unsigned int) ts->tv_sec,
		(unsigned int) ts->tv_usec, rx, tx);

	if (r)
		dbi_result_free(r);
}


static inline int attr_exists(const char *name, int item_id)
{
	int ret = -1;

	dbi_result r = dbi_conn_queryf(db_conn,
	    "SELECT id FROM attrs WHERE name = '%s' AND " \
	    "item = %d", name, item_id);

	if (r) {
		if (dbi_result_first_row(r) == 1)
			ret = (int) dbi_result_get_long(r, "id");
		dbi_result_free(r);
	}

	return ret;
}

static inline int insert_attr(stat_attr_t *a, int item_id)
{
	dbi_result r;

	r = dbi_conn_queryf(db_conn,
		"INSERT INTO attrs (name, item, rx_rate, tx_rate, " \
		"rx_counter, tx_counter) VALUES ('%s', %d, %d, %d, " \
		"%lld, %lld)", type2name(a->a_type), item_id,
		attr_get_rx_rate(a), attr_get_tx_rate(a),
		attr_get_rx(a), attr_get_tx(a));

	if (!r)
		return -1;

	dbi_result_free(r);
	return attr_exists(type2name(a->a_type), item_id);

}

static inline int up_attr(stat_attr_t *a, int attr_id)
{
	dbi_result r;

	r = dbi_conn_queryf(db_conn,
		"UPDATE attrs SET rx_rate = %d, tx_rate = %d, " \
		"rx_counter = %lld, tx_counter = %lld WHERE id = %d",
		attr_get_rx_rate(a), attr_get_tx_rate(a),
		attr_get_rx(a), attr_get_tx(a), attr_id);

	if (!r)
		return -1;

	dbi_result_free(r);

	return 0;
}

static inline int item_exists(item_t *it, node_t *node, int node_id)
{
	int ret = -1;
	dbi_result r;

	if (it->i_flags & ITEM_FLAG_IS_CHILD) {
		int parent;
		item_t *p = get_item(node, it->i_parent);
		if (!p)
			goto no_parent;

		parent = item_exists(p, node, node_id);
		if (parent < 0)
			goto no_parent;

		r = dbi_conn_queryf(db_conn,
		    "SELECT id FROM items WHERE node = %d AND " \
		    "name = '%s' AND handle = %d AND parent = %d",
		    node_id, it->i_name, it->i_handle, parent);
		goto skip;
	}

no_parent:
	r = dbi_conn_queryf(db_conn,
	    "SELECT id FROM items WHERE node = %d AND " \
	    "name = '%s' AND handle = %d", node_id, it->i_name, it->i_handle);

skip:
	if (r) {
		if (dbi_result_first_row(r) == 1)
			ret = (int) dbi_result_get_long(r, "id");
		dbi_result_free(r);
	}

	return ret;
}

static inline int insert_item(item_t *it, node_t *node, int node_id)
{
	dbi_result r;

	if (it->i_flags & ITEM_FLAG_IS_CHILD) {
		int parent;
		item_t *p = get_item(node, it->i_parent);
		if (!p)
			goto no_parent;

		parent = item_exists(p, node, node_id);
		if (parent < 0)
			goto no_parent;

		r = dbi_conn_queryf(db_conn,
			"INSERT INTO items (name, node, handle, parent, " \
			"indent, description) VALUES ('%s', %d, %d, %d, " \
			"%d, %s%s%s)",
			it->i_name, node_id, it->i_handle,
			parent, it->i_level,
			it->i_desc ? "'" : "",
			it->i_desc ? it->i_desc : "NULL",
			it->i_desc ? "'" : "");
		goto skip;
	}

no_parent:
	r = dbi_conn_queryf(db_conn,
			"INSERT INTO items (name, node, handle) VALUES " \
			"('%s', %d, %d)", it->i_name, node_id, it->i_handle);

skip:
	if (!r)
		return -1;

	dbi_result_free(r);
	return item_exists(it, node, node_id);
}

static inline int up_item(item_t *it, int item_id)
{
	dbi_result r;

	r = dbi_conn_queryf(db_conn,
		"UPDATE attrs SET rx_usage = %d, tx_usage = %d " \
		"WHERE id = %d",
		it->i_rx_usage, it->i_tx_usage, item_id);

	if (!r)
		return -1;

	dbi_result_free(r);

	return 0;
}

static inline int node_exists(const char *name)
{
	int ret = -1;
	dbi_result r = dbi_conn_queryf(db_conn,
	    "SELECT id FROM nodes WHERE name = '%s'", name);

	if (r) {
		if (dbi_result_first_row(r) == 1)
			ret = (int) dbi_result_get_long(r, "id");
		dbi_result_free(r);
	}

	return ret;
}

static inline int insert_node(const char *name, const char *source)
{
	dbi_result r;

	if (source)
		r = dbi_conn_queryf(db_conn,
			"INSERT INTO nodes (name, source) VALUES ('%s', '%s')",
			name, source);
	else
		r = dbi_conn_queryf(db_conn,
			"INSERT INTO nodes (name) VALUES ('%s')", name);

	if (!r)
		return -1;

	dbi_result_free(r);

	return node_exists(name);

}

static inline int cur_hist_index(int index)
{
	return index == 0 ? (HISTORY_SIZE - 1) : (index - 1);
}

struct xdata {
	int node_id;
	node_t *node;
};

static void write_per_attr(stat_attr_t *a, void *arg)
{
	int attr_id;
	int item_id = (int) arg;

	attr_id = attr_exists(type2name(a->a_type), item_id);
	if (attr_id < 0) {
		attr_id = insert_attr(a, item_id);
		if (attr_id < 0)
			return;
	} else
		up_attr(a, attr_id);

	if (a->a_flags & ATTR_FLAG_HISTORY) {
		history_t *h = &((stat_attr_hist_t *) a)->a_hist;

		if (do_read && get_read_interval() != 1.0f) {
			int idx = cur_hist_index(h->h_read.he_index);
			add_history("hist_r", attr_id,
				    &h->h_read.he_last_update,
				    h->h_read.he_rx.hd_data[idx],
				    h->h_read.he_tx.hd_data[idx]);
		}

		if (do_sec && diff_now(&h->h_sec.he_last_update) < 0.5f) {
			int idx = cur_hist_index(h->h_sec.he_index);
			add_history("hist_s", attr_id,
				    &h->h_sec.he_last_update,
				    h->h_sec.he_rx.hd_data[idx],
				    h->h_sec.he_tx.hd_data[idx]);
		}

		if (do_min && diff_now(&h->h_min.he_last_update) < 30.0f) {
			int idx = cur_hist_index(h->h_min.he_index);
			add_history("hist_m", attr_id,
				    &h->h_min.he_last_update,
				    h->h_min.he_rx.hd_data[idx],
				    h->h_min.he_tx.hd_data[idx]);
		}

		if (do_hour && diff_now(&h->h_hour.he_last_update) < 1800.0f) {
			int idx = cur_hist_index(h->h_hour.he_index);
			add_history("hist_h", attr_id,
				    &h->h_hour.he_last_update,
				    h->h_hour.he_rx.hd_data[idx],
				    h->h_hour.he_tx.hd_data[idx]);
		}

		if (do_day && diff_now(&h->h_day.he_last_update) < 43200.0f) {
			int idx = cur_hist_index(h->h_day.he_index);
			add_history("hist_d", attr_id,
				    &h->h_day.he_last_update,
				    h->h_day.he_rx.hd_data[idx],
				    h->h_day.he_tx.hd_data[idx]);
		}
	}
}

static void write_per_item(item_t *item, void *arg)
{
	int item_id;
	struct xdata *x = (struct xdata *) arg;

	item_id = item_exists(item, x->node, x->node_id);
	if (item_id < 0)
		item_id = insert_item(item, x->node, x->node_id);

	if (item_id < 0)
		return;

	up_item(item, item_id);

	foreach_attr(item, &write_per_attr, (void *) item_id);
}

static void write_per_node(node_t *node, void *arg)
{
	struct xdata x = { .node = node };

	x.node_id = node_exists(node->n_name);

	if (x.node_id < 0)
		x.node_id = insert_node(node->n_name, node->n_from);

	if (x.node_id < 0)
		return;

	foreach_item(node, write_per_item, &x);
}

void db_draw(void)
{
	static int rem = 1;

	if (--rem)
		return;
	else
		rem = c_interval;

	foreach_node(write_per_node, NULL);
}

static void print_module_help(void)
{
	printf(
	"DB - Database Output\n" \
	"\n" \
	"  Writes current rate estimations into a database for\n" \
	"  other tools to pick up.\n" \
	"\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    driverdir=DIR    Directory containing libdi drivers\n" \
	"    driver=DRIVER    DB driver (default: mysql)\n" \
	"    host=HOST        Host the database is on (default: localhost)\n" \
	"    dbname=NAME      Name of database (default: bmon)\n" \
	"    username=NAME    Authentication username (default: bmon)\n" \
	"    password=TEXT    Authentication password\n" \
	"    mask=MASK        Write selection mask (default: mhd)\n" \
	"    interval=SEC     Update interval in seconds (default: 3)\n" \
	"\n" \
	"  Mask Attributes:\n" \
	"    r                Read interval\n" \
	"    s                Seconds\n" \
	"    m                Minutes\n" \
	"    h                Hours\n" \
	"    d                Days\n" \
	"\n" \
	"  Examples:\n" \
	"    -O 'db:password=bmon;mask=rmhd'");
}

static void db_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "driverdir") && attrs->value)
			c_driverdir = attrs->value;
		else if (!strcasecmp(attrs->type, "driver") && attrs->value)
			c_driver = attrs->value;
		else if (!strcasecmp(attrs->type, "host") && attrs->value)
			c_host = attrs->value;
		else if (!strcasecmp(attrs->type, "username") && attrs->value)
			c_username = attrs->value;
		else if (!strcasecmp(attrs->type, "dbname") && attrs->value)
			c_dbname = attrs->value;
		else if (!strcasecmp(attrs->type, "password") && attrs->value)
			c_password = attrs->value;
		else if (!strcasecmp(attrs->type, "mask") && attrs->value)
			c_mask = attrs->value;
		else if (!strcasecmp(attrs->type, "interval") && attrs->value)
			c_interval = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "help")) {
			print_module_help();
			exit(0);
		}
		
		attrs = attrs->next;
	}
}

static int db_probe(void)
{
	if (strchr(c_mask, 'r'))
		do_read = 1;

	if (strchr(c_mask, 's'))
		do_sec = 1;

	if (strchr(c_mask, 'm'))
		do_min = 1;

	if (strchr(c_mask, 'h'))
		do_hour = 1;

	if (strchr(c_mask, 'd'))
		do_day = 1;
		
	if (c_password == NULL) {
		fprintf(stderr, "You must specify the database password\n");
		return 0;
	}
	
	if (dbi_initialize(c_driverdir) < 0) {
		fprintf(stderr, "Cannot initialize DBI layer\n");
		return 0;
	}

	db_conn = dbi_conn_new(c_driver);
	if (db_conn == NULL) {
		fprintf(stderr, "Cannot initialize connection \"%s\"\n",
		    c_driver);
		return 0;
	}
	
	dbi_conn_set_option(db_conn, "host", c_host);
	dbi_conn_set_option(db_conn, "username", c_username);
	dbi_conn_set_option(db_conn, "password", c_password);
	dbi_conn_set_option(db_conn, "dbname", c_dbname);
	
	if (dbi_conn_connect(db_conn) < 0) {
		fprintf(stderr, "Cannot open database \"%s\" connection on " \
		    "%s@%s\n", c_dbname, c_username, c_host);
		return 0;
	}

	if (!create_tables()) {
		fprintf(stderr, "Could not create tables\n");
		return 0;
	}

	if (!insert_attr_descs()) {
		fprintf(stderr, "Could not insert attribute descriptions\n");
		return 0;
	}

	return 1;
}

static void db_shutdown(void)
{
	if (db_conn)
		dbi_conn_close(db_conn);

	dbi_shutdown();
}

static struct output_module db_ops = {
	.om_name = "db",
	.om_draw = db_draw,
	.om_set_opts = db_set_opts,
	.om_probe = db_probe,
	.om_shutdown db_shutdown,
};

static void __init db_init(void)
{
	register_secondary_output_module(&db_ops);
}
