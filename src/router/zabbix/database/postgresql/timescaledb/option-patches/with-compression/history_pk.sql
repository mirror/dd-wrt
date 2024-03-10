\set ON_ERROR_STOP on

\copy (select * from history_old) TO '/tmp/history.csv' DELIMITER ',' CSV;

CREATE TEMP TABLE temp_history (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    DOUBLE PRECISION DEFAULT '0.0000'          NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);

\copy temp_history FROM '/tmp/history.csv' DELIMITER ',' CSV

DO $$
DECLARE
	tsdb_version_major	INTEGER;
	chunk_tm_interval	INTEGER;
	jobid			INTEGER;
BEGIN
	SELECT substring(extversion, '^\d+') INTO tsdb_version_major FROM pg_extension WHERE extname='timescaledb';

	IF (tsdb_version_major < 2)
	THEN
		SELECT (upper(ranges[1]) - lower(ranges[1])) INTO chunk_tm_interval FROM chunk_relation_size('history')
			ORDER BY ranges DESC LIMIT 1;

		IF NOT FOUND THEN
			chunk_tm_interval = 86400;
		END IF;

		PERFORM create_hypertable('history', 'clock', chunk_time_interval => chunk_tm_interval, migrate_data => true);
	ELSE
		PERFORM create_hypertable('history', 'clock', chunk_time_interval => (
			SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_old'
		), migrate_data => true);
	END IF;

	INSERT INTO history SELECT * FROM temp_history ON CONFLICT (itemid,clock,ns) DO NOTHING;

	PERFORM set_integer_now_func('history', 'zbx_ts_unix_now', true);

	ALTER TABLE history
	SET (timescaledb.compress,timescaledb.compress_segmentby='itemid',timescaledb.compress_orderby='clock,ns');

	IF (tsdb_version_major < 2)
	THEN
		PERFORM add_compress_chunks_policy('history', (
				SELECT (p.older_than).integer_interval
				FROM _timescaledb_config.bgw_policy_compress_chunks p
				INNER JOIN _timescaledb_catalog.hypertable h ON (h.id=p.hypertable_id)
				WHERE h.table_name='history_old'
			)::integer
		);
	ELSE
		SELECT add_compression_policy('history', (
			SELECT extract(epoch FROM (config::json->>'compress_after')::interval)
			FROM timescaledb_information.jobs
			WHERE application_name LIKE 'Compression%%' AND hypertable_schema='public'
				AND hypertable_name='history_old'
			)::integer
		) INTO jobid;

		IF jobid IS NULL
		THEN
			RAISE EXCEPTION 'Failed to add compression policy';
		END IF;

		PERFORM alter_job(jobid, scheduled => true, next_start => now());
	END IF;

END $$;

UPDATE config SET compression_status=1;
