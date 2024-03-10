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
	tsdb_version_major	INTEGER;
	chunk_tm_interval	INTEGER;
	jobid			INTEGER;
BEGIN
	SELECT substring(extversion, '^\d+') INTO tsdb_version_major FROM pg_extension WHERE extname='timescaledb';

	IF (tsdb_version_major < 2)
	THEN
		SELECT (upper(ranges[1]) - lower(ranges[1])) INTO chunk_tm_interval FROM chunk_relation_size('history_log')
			ORDER BY ranges DESC LIMIT 1;

		IF NOT FOUND THEN
			chunk_tm_interval = 86400;
		END IF;

		PERFORM create_hypertable('history_log', 'clock', chunk_time_interval => chunk_tm_interval, migrate_data => true);
	ELSE
		PERFORM create_hypertable('history_log', 'clock', chunk_time_interval => (
			SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_log_old'
		), migrate_data => true);
	END IF;

	INSERT INTO history_log SELECT * FROM temp_history_log ON CONFLICT (itemid,clock,ns) DO NOTHING;

	PERFORM set_integer_now_func('history_log', 'zbx_ts_unix_now', true);

	ALTER TABLE history_log
	SET (timescaledb.compress,timescaledb.compress_segmentby='itemid',timescaledb.compress_orderby='clock,ns');

	IF (tsdb_version_major < 2)
	THEN
		PERFORM add_compress_chunks_policy('history_log', (
				SELECT (p.older_than).integer_interval
				FROM _timescaledb_config.bgw_policy_compress_chunks p
				INNER JOIN _timescaledb_catalog.hypertable h ON (h.id=p.hypertable_id)
				WHERE h.table_name='history_log_old'
			)::integer
		);
	ELSE
		SELECT add_compression_policy('history_log', (
			SELECT extract(epoch FROM (config::json->>'compress_after')::interval)
			FROM timescaledb_information.jobs
			WHERE application_name LIKE 'Compression%%' AND hypertable_schema='public'
				AND hypertable_name='history_log_old'
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
