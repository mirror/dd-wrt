\set ON_ERROR_STOP on

\copy (select * from history_log_old) TO '/tmp/history_log.csv' DELIMITER ',' CSV;

CREATE TEMP TABLE temp_history_log (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	timestamp                integer         DEFAULT '0'               NOT NULL,
	source                   varchar(64)     DEFAULT ''                NOT NULL,
	severity                 integer         DEFAULT '0'               NOT NULL,
	value                    text            DEFAULT ''                NOT NULL,
	logeventid               integer         DEFAULT '0'               NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);

\copy temp_history_log FROM '/tmp/history_log.csv' DELIMITER ',' CSV

DO $$
DECLARE
	jobid			INTEGER;
	tsdb_version		TEXT;
	tsdb_version_major	INTEGER;
	tsdb_version_minor	INTEGER;
	compress_after		INTEGER;
BEGIN
	PERFORM create_hypertable('history_log', 'clock', chunk_time_interval => (
		SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_log_old'
	), migrate_data => true);

	INSERT INTO history_log SELECT * FROM temp_history_log ON CONFLICT (itemid,clock,ns) DO NOTHING;


END $$;

UPDATE settings SET value_int=0 WHERE name='compression_status';
