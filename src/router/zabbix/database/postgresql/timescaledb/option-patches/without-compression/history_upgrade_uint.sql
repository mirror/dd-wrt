\set ON_ERROR_STOP on

\copy (select * from history_uint_old) TO '/tmp/history_uint.csv' DELIMITER ',' CSV;

CREATE TEMP TABLE temp_history_uint (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    numeric(20)     DEFAULT '0'               NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);

\copy temp_history_uint FROM '/tmp/history_uint.csv' DELIMITER ',' CSV

DO $$
DECLARE
	jobid			INTEGER;
	tsdb_version		TEXT;
	tsdb_version_major	INTEGER;
	tsdb_version_minor	INTEGER;
	compress_after		INTEGER;
BEGIN
	PERFORM create_hypertable('history_uint', 'clock', chunk_time_interval => (
		SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_uint_old'
	), migrate_data => true);

	INSERT INTO history_uint SELECT * FROM temp_history_uint ON CONFLICT (itemid,clock,ns) DO NOTHING;


END $$;

UPDATE settings SET value_int=0 WHERE name='compression_status';
