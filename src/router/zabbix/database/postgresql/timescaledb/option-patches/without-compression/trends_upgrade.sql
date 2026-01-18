\set ON_ERROR_STOP on

\copy (select * from trends_old) TO '/tmp/trends.csv' DELIMITER ',' CSV;

CREATE TEMP TABLE temp_trends (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	num                      integer         DEFAULT '0'               NOT NULL,
	value_min                DOUBLE PRECISION DEFAULT '0.0000'          NOT NULL,
	value_avg                DOUBLE PRECISION DEFAULT '0.0000'          NOT NULL,
	value_max                DOUBLE PRECISION DEFAULT '0.0000'          NOT NULL
);

\copy temp_trends FROM '/tmp/trends.csv' DELIMITER ',' CSV

DO $$
DECLARE
	jobid			INTEGER;
	tsdb_version		TEXT;
	tsdb_version_major	INTEGER;
	tsdb_version_minor	INTEGER;
	compress_after		INTEGER;
BEGIN
	PERFORM create_hypertable('trends', 'clock', chunk_time_interval => (
		SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='trends_old'
	), migrate_data => true);

	INSERT INTO trends SELECT * FROM temp_trends ON CONFLICT (itemid,clock) DO NOTHING;


END $$;

UPDATE settings SET value_int=0 WHERE name='compression_status';
