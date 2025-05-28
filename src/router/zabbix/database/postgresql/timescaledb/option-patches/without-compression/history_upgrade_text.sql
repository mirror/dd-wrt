\set ON_ERROR_STOP on

\copy (select * from history_text_old) TO '/tmp/history_text.csv' DELIMITER ',' CSV;

CREATE TEMP TABLE temp_history_text (
	itemid                   bigint                                    NOT NULL,
	clock                    integer         DEFAULT '0'               NOT NULL,
	value                    text            DEFAULT ''                NOT NULL,
	ns                       integer         DEFAULT '0'               NOT NULL
);

\copy temp_history_text FROM '/tmp/history_text.csv' DELIMITER ',' CSV

DO $$
DECLARE
	chunk_tm_interval	INTEGER;
	jobid			INTEGER;
BEGIN
	PERFORM create_hypertable('history_text', 'clock', chunk_time_interval => (
		SELECT integer_interval FROM timescaledb_information.dimensions WHERE hypertable_name='history_text_old'
	), migrate_data => true);

	INSERT INTO history_text SELECT * FROM temp_history_text ON CONFLICT (itemid,clock,ns) DO NOTHING;


END $$;

UPDATE config SET compression_status=0;
