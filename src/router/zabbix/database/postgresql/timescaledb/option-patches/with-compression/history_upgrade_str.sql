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
	chunk_tm_interval	INTEGER;
	jobid			INTEGER;
BEGIN
	PERFORM create_hypertable('history_str', 'clock', chunk_time_interval => (
		SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_str_old'
	), migrate_data => true);

	INSERT INTO history_str SELECT * FROM temp_history_str ON CONFLICT (itemid,clock,ns) DO NOTHING;

	PERFORM set_integer_now_func('history_str', 'zbx_ts_unix_now', true);

	ALTER TABLE history_str
	SET (timescaledb.compress,timescaledb.compress_segmentby='itemid',timescaledb.compress_orderby='clock,ns');

	SELECT add_compression_policy('history_str', (
		SELECT extract(epoch FROM (config::json->>'compress_after')::interval)
		FROM timescaledb_information.jobs
		WHERE application_name LIKE 'Compression%%' AND hypertable_schema='public'
			AND hypertable_name='history_str_old'
		)::integer
	) INTO jobid;

	IF jobid IS NULL
	THEN
		RAISE EXCEPTION 'Failed to add compression policy';
	END IF;

	PERFORM alter_job(jobid, scheduled => true, next_start => now());

END $$;

UPDATE config SET compression_status=1;
