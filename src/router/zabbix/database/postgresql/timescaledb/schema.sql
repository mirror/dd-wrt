DO $$
DECLARE
	minimum_postgres_version_major		INTEGER;
	minimum_postgres_version_minor		INTEGER;
	current_postgres_version_major		INTEGER;
	current_postgres_version_minor		INTEGER;
	current_postgres_version_full		VARCHAR;

	minimum_timescaledb_version_major	INTEGER;
	minimum_timescaledb_version_minor	INTEGER;
	current_timescaledb_version_major	INTEGER;
	current_timescaledb_version_minor	INTEGER;
	current_timescaledb_version_full	VARCHAR;
BEGIN
	SELECT 10 INTO minimum_postgres_version_major;
	SELECT 2 INTO minimum_postgres_version_minor;
	SELECT 1 INTO minimum_timescaledb_version_major;
	SELECT 5 INTO minimum_timescaledb_version_minor;

	SHOW server_version INTO current_postgres_version_full;

	IF NOT found THEN
		RAISE EXCEPTION 'Cannot determine PostgreSQL version, aborting';
	END IF;

	SELECT substring(current_postgres_version_full, '^(\d+).') INTO current_postgres_version_major;
	SELECT substring(current_postgres_version_full, '^\d+.(\d+)') INTO current_postgres_version_minor;

	IF (current_postgres_version_major < minimum_postgres_version_major OR
			(current_postgres_version_major = minimum_postgres_version_major AND
			current_postgres_version_minor < minimum_postgres_version_minor)) THEN
			RAISE EXCEPTION 'PostgreSQL version % is NOT SUPPORTED (with TimescaleDB)! Minimum is %.%.0 !',
					current_postgres_version_full, minimum_postgres_version_major,
					minimum_postgres_version_minor;
	ELSE
		RAISE NOTICE 'PostgreSQL version % is valid', current_postgres_version_full;
	END IF;

	SELECT extversion INTO current_timescaledb_version_full FROM pg_extension WHERE extname = 'timescaledb';

	IF NOT found THEN
		RAISE EXCEPTION 'TimescaleDB extension is not installed';
	ELSE
		RAISE NOTICE 'TimescaleDB extension is detected';
	END IF;

	SELECT substring(current_timescaledb_version_full, '^(\d+).') INTO current_timescaledb_version_major;
	SELECT substring(current_timescaledb_version_full, '^\d+.(\d+)') INTO current_timescaledb_version_minor;

	IF (current_timescaledb_version_major < minimum_timescaledb_version_major OR
			(current_timescaledb_version_major = minimum_timescaledb_version_major AND
			current_timescaledb_version_minor < minimum_timescaledb_version_minor)) THEN
		RAISE EXCEPTION 'TimescaleDB version % is UNSUPPORTED! Minimum is %.%.0!',
				current_timescaledb_version_full, minimum_timescaledb_version_major,
				minimum_timescaledb_version_minor;
	ELSE
		RAISE NOTICE 'TimescaleDB version % is valid', current_timescaledb_version_full;
	END IF;
	PERFORM create_hypertable('history', 'clock', chunk_time_interval => 86400, migrate_data => true);
	PERFORM create_hypertable('history_uint', 'clock', chunk_time_interval => 86400, migrate_data => true);
	PERFORM create_hypertable('history_log', 'clock', chunk_time_interval => 86400, migrate_data => true);
	PERFORM create_hypertable('history_text', 'clock', chunk_time_interval => 86400, migrate_data => true);
	PERFORM create_hypertable('history_str', 'clock', chunk_time_interval => 86400, migrate_data => true);
	PERFORM create_hypertable('trends', 'clock', chunk_time_interval => 2592000, migrate_data => true);
	PERFORM create_hypertable('trends_uint', 'clock', chunk_time_interval => 2592000, migrate_data => true);
	UPDATE config SET db_extension='timescaledb',hk_history_global=1,hk_trends_global=1;
	UPDATE config SET compression_status=1,compress_older='7d';
	RAISE NOTICE 'TimescaleDB is configured successfully';
END $$;
