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
	chunk_tm_interval	INTEGER;
	jobid			INTEGER;
BEGIN
	PERFORM create_hypertable('history', 'clock', chunk_time_interval => (
		SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_old'
	), migrate_data => true);

	INSERT INTO history SELECT * FROM temp_history ON CONFLICT (itemid,clock,ns) DO NOTHING;

	PERFORM set_integer_now_func('history', 'zbx_ts_unix_now', true);

	ALTER TABLE history
	SET (timescaledb.compress,timescaledb.compress_segmentby='itemid',timescaledb.compress_orderby='clock,ns');

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

END $$;

UPDATE config SET compression_status=1;
