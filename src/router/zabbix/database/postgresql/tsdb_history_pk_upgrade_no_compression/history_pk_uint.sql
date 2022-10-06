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
	tsdb_version_major	INTEGER;
	chunk_tm_interval	INTEGER;
	jobid			INTEGER;
BEGIN
	SELECT substring(extversion, '^\d+') INTO tsdb_version_major FROM pg_extension WHERE extname='timescaledb';

	IF (tsdb_version_major < 2)
	THEN
		SELECT (upper(ranges[1]) - lower(ranges[1])) INTO chunk_tm_interval FROM chunk_relation_size('history_uint')
			ORDER BY ranges DESC LIMIT 1;

		IF NOT FOUND THEN
			chunk_tm_interval = 86400;
		END IF;

		PERFORM create_hypertable('history_uint', 'clock', chunk_time_interval => chunk_tm_interval, migrate_data => true);
	ELSE
		PERFORM create_hypertable('history_uint', 'clock', chunk_time_interval => (
			SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_uint_old'
		), migrate_data => true);
	END IF;

	INSERT INTO history_uint SELECT * FROM temp_history_uint ON CONFLICT (itemid,clock,ns) DO NOTHING;


END $$;

UPDATE config SET compression_status=0;
