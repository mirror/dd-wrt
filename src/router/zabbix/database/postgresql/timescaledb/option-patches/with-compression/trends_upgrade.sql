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

	PERFORM set_integer_now_func('trends', 'zbx_ts_unix_now', true);

	-- extversion is a version string in format "2.19.5"
	SELECT extversion INTO tsdb_version FROM pg_extension WHERE extname = 'timescaledb';

	tsdb_version_major := substring(tsdb_version, '^\d+')::INTEGER;
	tsdb_version_minor := substring(tsdb_version, '^\d+\.(\d+)')::INTEGER;

	-- Check if TimescaleDB version is greater than or equal to 2.18.x
	IF tsdb_version_major > 2 OR (tsdb_version_major = 2 AND tsdb_version_minor >= 18)
	THEN
		-- Available since TimescaleDB 2.18.0
		ALTER TABLE trends
		SET (
			timescaledb.enable_columnstore=true,
			timescaledb.segmentby='itemid',
			timescaledb.orderby='clock'
		);

		-- application_name is like 'Columnstore Policy%' in the newer TimescaleDB versions.
		-- application_name is like 'Compression%' in the older TimescaleDB versions
		-- before around TimescaleDB 2.18.
		SELECT extract(epoch FROM (config::json->>'compress_after')::interval)::integer
		INTO compress_after
		FROM timescaledb_information.jobs
		WHERE (application_name LIKE 'Columnstore Policy%%' OR application_name LIKE 'Compression%%')
			AND hypertable_schema = 'public'
			AND hypertable_name = 'trends_old';

		-- Available since TimescaleDB 2.18.0
		CALL add_columnstore_policy('trends', after => compress_after);

		SELECT job_id
		INTO jobid
		FROM timescaledb_information.jobs
		WHERE (application_name LIKE 'Columnstore Policy%%' OR application_name LIKE 'Compression%%')
			AND hypertable_schema = 'public'
			AND hypertable_name = 'trends';
	ELSE
		-- Deprecated since TimescaleDB 2.18.0
		ALTER TABLE trends
		SET (
			timescaledb.compress,
			timescaledb.compress_segmentby='itemid',
			timescaledb.compress_orderby='clock'
		);

		SELECT add_compression_policy('trends', (
			SELECT extract(epoch FROM (config::json->>'compress_after')::interval)
			FROM timescaledb_information.jobs
			WHERE application_name LIKE 'Compression%%' AND hypertable_schema='public'
				AND hypertable_name='trends_old'
			)::integer
		) INTO jobid;
	END IF;

	IF jobid IS NULL
	THEN
		RAISE EXCEPTION 'Failed to add compression policy';
	END IF;

	PERFORM alter_job(jobid, scheduled => true, next_start => now());

END $$;

UPDATE settings SET value_int=1 WHERE name='compression_status';
