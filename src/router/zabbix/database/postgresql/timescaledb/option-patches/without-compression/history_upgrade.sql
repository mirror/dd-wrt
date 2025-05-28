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


END $$;

UPDATE config SET compression_status=0;
