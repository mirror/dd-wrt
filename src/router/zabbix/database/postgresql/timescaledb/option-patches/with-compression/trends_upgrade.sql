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
	chunk_tm_interval	INTEGER;
	jobid			INTEGER;
BEGIN
	PERFORM create_hypertable('trends', 'clock', chunk_time_interval => (
		SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='trends_old'
	), migrate_data => true);

	INSERT INTO trends SELECT * FROM temp_trends ON CONFLICT (itemid,clock) DO NOTHING;

	PERFORM set_integer_now_func('trends', 'zbx_ts_unix_now', true);

	ALTER TABLE trends
	SET (timescaledb.compress,timescaledb.compress_segmentby='itemid',timescaledb.compress_orderby='clock');

	SELECT add_compression_policy('trends', (
		SELECT extract(epoch FROM (config::json->>'compress_after')::interval)
		FROM timescaledb_information.jobs
		WHERE application_name LIKE 'Compression%%' AND hypertable_schema='public'
			AND hypertable_name='trends_old'
		)::integer
	) INTO jobid;

	IF jobid IS NULL
	THEN
		RAISE EXCEPTION 'Failed to add compression policy';
	END IF;

	PERFORM alter_job(jobid, scheduled => true, next_start => now());

END $$;

UPDATE config SET compression_status=1;
