#!/bin/sh

DB_ROOT="`dirname $0`/../.."
DB_ROOT="`cd $DB_ROOT && pwd`"
SQLITE="$DB_ROOT/lang/sql/sqlite"
OS="`uname -o | awk '{print tolower($1)}'`"

if [[ "$OS" == "cygwin" ]] ; then
	ARCH="`uname -m | grep '64'`"
	if [ -z "$ARCH" ] ; then
		ARCH="Win32"
	else
		ARCH="x64"
	fi
	DBSQL="$DB_ROOT/build_windows/$ARCH/Release/dbsql"
	TESTFIXTURE="$DB_ROOT/build_windows/$ARCH/Release/testfixture"
	OUT_DIR="$DB_ROOT/build_windows/sql"	
else
	DBSQL="$DB_ROOT/build_unix/dbsql"
	TESTFIXTURE="$DB_ROOT/build_unix/sql/testfixture"
	OUT_DIR="$DB_ROOT/build_unix/sql"
fi


# Check the "-no-clean" option
CLEAN_OUTPUT=1
if [[ "$1" == "-no-clean" ]] ; then
	CLEAN_OUTPUT=0
	echo "## Files generted by this script are not removed automatically."
	echo "## Generated files are in '$OUT_DIR'"
	echo "## and have a prefix 'gen_'."
fi


# Convert the given Unix-like path name to OS's native path name
function get_os_path_name() {
	if [[ "$OS" == "cygwin" ]] ; then
		echo "`cygpath -w $1`"
	else
		echo "$1"
	fi
}


# Check compile options
#   AUTH_ENABLED=1        If authentication is enabled
#   ENCRYPTION_ENABLED=1  If encryption is enabled
#   KEYSTORE_ENABLED=1    If key store is enabled
#
function is_option_enabled() {
	$DBSQL test.db "select sqlite_compileoption_used('$1')" | tr -d '\r'
}

AUTH_ENABLED=`is_option_enabled USER_AUTHENTICATION`
ENCRYPTION_ENABLED=`is_option_enabled HAS_CODEC`
KEYSTORE_ENABLED=`is_option_enabled BDBSQL_USER_AUTHENTICATION_KEYSTORE`

echo "AUTH_ENABLED: $AUTH_ENABLED"
echo "ENCRYPTION_ENABLED: $ENCRYPTION_ENABLED"
echo "KEYSTORE_ENABLED: $KEYSTORE_ENABLED"

if (( AUTH_ENABLED == 0 )) ; then
	echo "Authentication is disabled. Skip tests."
	exit 1
fi


# Generate tests which use C APIs (via Tcl) from SQLite tests
#   sqlite_to_bdbapi testname
#       $testname   - The name of the SQLite test without the '.test' suffix.
#                     For example, "userauth01" for "sqlite/test/userauth01.test".
function sqlite_to_bdbapi() {
	# 1. Adjust the path to $testdir
	# 2. Suffix test name with "api"
	# 3. Handle misspelled "authuser01" in userauth01.test
	# 4. Convert one-line SQL-execution expressions into multiple lines with the
	#    SQL statement on a separate line.
	SED_EXPR_API=(\
	-e "s/\[file dirname \$argv0\]/[file dirname \$argv0]\/..\/..\/lang\/sql\/sqlite\/test/" \
	-e "s/$1/$1-api/" \
	-e "s/authuser01/authuser01-api/" \
	-e "s/\(execsql\|catchsql\|eval\) {\([^}]*\)}/\1 {\n    \2;\n  }/" \
	)

	if (( ENCRYPTION_ENABLED == 1 )) ; then
		# Set the encryption key after initial database open
		SED_EXPR_API[8]="-e"
		SED_EXPR_API[9]="/source \$testdir\/tester.tcl/a\\do_not_use_codec\nsqlite3_key db key"
		if (( KEYSTORE_ENABLED == 1 )) ; then
			# If the database is not "test.db", set the encryption key after database open
			# This expression is used for userauth01.test. Improve it if more tests are added
			SED_EXPR_API[10]="-e"
			SED_EXPR_API[11]="s/sqlite3 \([^ ]\+\) test3.db/&\n  sqlite3_key \1 key/"
		else
			# Set the encryption key after every database open
			SED_EXPR_API[10]="-e"
			SED_EXPR_API[11]="s/sqlite3 \([^ ]\+\) \([^ ]\+\)/&\n  sqlite3_key \1 key/"
		fi
	fi

	sed "${SED_EXPR_API[@]}" "$SQLITE/test/$1.test" > "$OUT_DIR/gen_$1_api.test"
}

# Generate tests which use C APIs (via Tcl) from BDB tests
#   bdb_to_api testname
#       $testname   - The name of the BDB test.
#                     For example, "userauth_ext_common" for "bdb_userauth_ext_common.test".
function bdb_to_api() {
	TEST_PREFIX="${1//_/-}"
	SED_EXPR_API=(\
	-e "s/$TEST_PREFIX/$TEST_PREFIX-api/"
	)

	sed "${SED_EXPR_API[@]}" "$DB_ROOT/test/sql/bdb_$1.test" > "$OUT_DIR/gen_$1_api.test"
}

# Generate tests which use pragmas
#   api_to_pragma testname
#       $testname   - The name of the generated API test.
#                     For example, "userauth01" for "gen_userauth01_api.test".
function api_to_pragma() {
	# 1. Convert authentication calls and extract error messages
	# 2. Suffix test name with "pragma"
	# 3. Expected result for SQLITE_OK is an empty message
	# 4. Expected result for SQLITE_AUTH is an error message for authentication
	# 5. Specific rule to handle userauth01-1.64
	sed -e '/^[^#].*sqlite3_user_/ {
		s/sqlite3_user_authenticate \([^ ]\+\) \([^ ]\+\) \([^ ]\+\)/  PRAGMA bdbsql_user_login="\2:\3"\n  } \1]/
		s/sqlite3_user_add \([^ ]\+\) \([^ ]\+\) \([^ ]\+\) \([^ ]\+\)/  PRAGMA bdbsql_user_add="\2:\3:\4"\n  } \1]/
		s/sqlite3_user_change \([^ ]\+\) \([^ ]\+\) \([^ ]\+\) \([^ ]\+\)/  PRAGMA bdbsql_user_edit="\2:\3:\4"\n  } \1]/
		s/sqlite3_user_delete \([^ ]\+\) \([^ ]\+\)/  PRAGMA bdbsql_user_delete="\2"\n  } \1]/
		i\  set v [catchsql \{
		a\  lindex $v end
	}' -e "s/-api/-pragma/" \
	   -e "s/{SQLITE_OK}/{}/" \
	   -e "s/{SQLITE_AUTH}/{\/Error 23: authorization denied\/}/" \
	   -e "s/{SQLITE_ERROR}/{\/Error 1: SQL logic error or missing database\/}/" \
	   -e "s/{SQLITE_PERM}/{\/Error 3: access permission denied\/}/" \
	   -e "s/{\/SQLITE_AUTH|SQLITE_PERM\/}/{\/Error 23: authorization denied|Error 3: access permission denied\/}/" \
	   -e "/^do_test userauth01-pragma-1.64/,/^}/ s/{\/Error 23: authorization denied\/}/{SQLITE_AUTH}/" \
	"$OUT_DIR/gen_$1_api.test" > "$OUT_DIR/gen_$1_pragma.test"
}

# Generate tests which use SQL shell scripts
#   api_to_shell testname
#       $testname   - The name of the generated API test.
#                     For example, "userauth01" for "gen_userauth01_api.test".
function api_to_shell() {
	# For userauth01.test, the 2.1 part is not applicable. Remove it.
	KEY_REQUIRED=0
	if (( ENCRYPTION_ENABLED == 1 )) && (( KEYSTORE_ENABLED == 0 )) ; then
		KEY_REQUIRED=1
	fi
	sed -e "/authuser01-api-2.1 {/,/}/d" "$OUT_DIR/gen_$1_api.test" | \
	awk -f "$DB_ROOT/test/sql/api_to_shell.awk" -v "KEY_REQUIRED=$KEY_REQUIRED" \
	    > "$OUT_DIR/gen_$1_shell.dbsql"
}

# Generate tests for all interfaces
#   generate_all testname
#       $testname - The name of the test (without the common prefix and suffix).
#                   For example, "userauth01" for SQLite "userauth01.test" or BDB "bdb_userauth01.test".
function generate_all() {
	if [[ -f "$SQLITE/test/$1.test" ]] ; then
		sqlite_to_bdbapi "$1"
	else
		bdb_to_api "$1"
	fi
	api_to_pragma "$1"
	api_to_shell "$1"
}


# Run a single test using all interfaces. The tests must be generated before.
#   run_test testname
#       $testname - The name of the test
function run_test() {
	# Run API and PRAGMA tests
	for api in "api" "pragma"
	do
		rm -rf test*db*
		TEST_NAME="`get_os_path_name $OUT_DIR/gen_$1_$api`"
		printf "Running $TEST_NAME.test... \t"
		$TESTFIXTURE "$TEST_NAME.test" &> "$TEST_NAME.out"
		result=`grep "errors out of" "$TEST_NAME.out" || echo "fail"`
		leak=`grep "Unfreed memory:" "$TEST_NAME.out" || echo "no memleaks"`
		printf "$result\t$leak\n"
	done

	# Run SQL Shell tests
	rm -rf test*db*
	TEST_NAME="`get_os_path_name $OUT_DIR/gen_$1_shell`"
	printf "Running $TEST_NAME.dbsql... \t"
	$DBSQL < "$TEST_NAME.dbsql" &> "$TEST_NAME.tmp"
	# Turn absolute paths into relative paths, so file paths stay same across machines
	CUR_DIR="`pwd -P`"
	CUR_DIR="`get_os_path_name $CUR_DIR`"
	sed -e "s|${CUR_DIR//\\/\\\\}[/\\]||" \
	    -e "s/\(Error: near line\) [0-9]\+:/\1 xx:/" "$TEST_NAME.tmp" > "$TEST_NAME.out"
	# Compare with the expected output
	DIFF=`diff -wu "$DB_ROOT/test/sql/bdb_$1_shell.expected" "$TEST_NAME.out"`
	if (( "$?" == 0 )) ; then
		echo "ok"
	else
		echo "fail"
		echo "$DIFF"
	fi
}

# For a single test, generate scripts for all interfaces and run them.
#   generate_and_run testname
#       $testname - The name of the test
function generate_and_run() {
	generate_all $1
	run_test $1
	if (( CLEAN_OUTPUT == 1 )) ; then
		rm -rf $OUT_DIR/gen_$1_*.* test*db*
	fi
}

generate_and_run "userauth01"
generate_and_run "userauth_ext_common"

if (( KEYSTORE_ENABLED == 1 )) ; then
	generate_and_run "userauth_keystore"
else
	generate_and_run "userauth_no_keystore"
fi
