\set ON_ERROR_STOP on

\copy (select * from history_str_old) TO '/tmp/history_str.csv' DELIMITER ',' CSV;

CREATE TEMP TABLE temp_history_str (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    varchar(255)    DEFAULT ''                NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);

\copy temp_history_str FROM '/tmp/history_str.csv' DELIMITER ',' CSV

DO $$
DECLARE
	tsdb_version_major	INTEGER;
	chunk_tm_interval	INTEGER;
	jobid			INTEGER;
BEGIN
	SELECT substring(extversion, '^\d+') INTO tsdb_version_major FROM pg_extension WHERE extname='timescaledb';

	IF (tsdb_version_major < 2)
	THEN
		SELECT (upper(ranges[1]) - lower(ranges[1])) INTO chunk_tm_interval FROM chunk_relation_size('history_str')
			ORDER BY ranges DESC LIMIT 1;

		IF NOT FOUND THEN
			chunk_tm_interval = 86400;
		END IF;

		PERFORM create_hypertable('history_str', 'clock', chunk_time_interval => chunk_tm_interval, migrate_data => true);
	ELSE
		PERFORM create_hypertable('history_str', 'clock', chunk_time_interval => (
			SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_str_old'
		), migrate_data => true);
	END IF;

	INSERT INTO history_str SELECT * FROM temp_history_str ON CONFLICT (itemid,clock,ns) DO NOTHING;

%COMPRESS
END $$;

%CONFIG_COMPR
