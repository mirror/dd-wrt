CREATE OR REPLACE FUNCTION cuid_timestamp(cuid varchar(25)) RETURNS integer AS $$
DECLARE
	base36 varchar; 
	a char[];
	ret bigint;
	i int;
	val int;
	chars varchar;
BEGIN
	base36 := substring(cuid FROM 2 FOR 8);

	chars := '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ';

	FOR i IN REVERSE char_length(base36)..1 LOOP
		a := a || substring(upper(base36) FROM i FOR 1)::char;
	END LOOP;
	i := 0;
	ret := 0;
	WHILE i < (array_length(a, 1)) LOOP
		val := position(a[i + 1] IN chars) - 1;
		ret := ret + (val * (36 ^ i));
		i := i + 1;
	END LOOP;

	RETURN CAST(ret/1000 AS integer);
END;
$$ LANGUAGE 'plpgsql' IMMUTABLE;
DROP FUNCTION IF EXISTS base36_decode(character varying);

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

	current_db_extension			VARCHAR;
BEGIN
	SELECT 10 INTO minimum_postgres_version_major;
	SELECT 9 INTO minimum_postgres_version_minor;
	SELECT 2 INTO minimum_timescaledb_version_major;
	SELECT 0 INTO minimum_timescaledb_version_minor;

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

	SELECT value_str FROM settings WHERE name='db_extension' INTO current_db_extension;

	PERFORM create_hypertable('history', 'clock', chunk_time_interval => 86400, migrate_data => true, if_not_exists => true);
	PERFORM create_hypertable('history_uint', 'clock', chunk_time_interval => 86400, migrate_data => true, if_not_exists => true);
	PERFORM create_hypertable('history_log', 'clock', chunk_time_interval => 86400, migrate_data => true, if_not_exists => true);
	PERFORM create_hypertable('history_text', 'clock', chunk_time_interval => 86400, migrate_data => true, if_not_exists => true);
	PERFORM create_hypertable('history_str', 'clock', chunk_time_interval => 86400, migrate_data => true, if_not_exists => true);
	PERFORM create_hypertable('history_bin', 'clock', chunk_time_interval => 86400, migrate_data => true, if_not_exists => true);
	PERFORM create_hypertable('auditlog', 'auditid', chunk_time_interval => 604800,
			time_partitioning_func => 'cuid_timestamp', migrate_data => true, if_not_exists => true);
	PERFORM create_hypertable('trends', 'clock', chunk_time_interval => 2592000, migrate_data => true, if_not_exists => true);
	PERFORM create_hypertable('trends_uint', 'clock', chunk_time_interval => 2592000, migrate_data => true, if_not_exists => true);

	IF (current_db_extension = 'timescaledb') THEN
		RAISE NOTICE 'TimescaleDB extension is already installed; not changing configuration';
	ELSE
		UPDATE settings SET value_str='timescaledb' WHERE name='db_extension';
		UPDATE settings SET value_int=1 WHERE name='hk_history_global';
		UPDATE settings SET value_int=1 WHERE name='hk_trends_global';
		UPDATE settings SET value_int=1 WHERE name='compression_status';
		UPDATE settings SET value_str='7d' WHERE name='compress_older';
	END IF;

	RAISE NOTICE 'TimescaleDB is configured successfully';
END $$;
