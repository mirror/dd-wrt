function open_db(db) {
	print ".open " DB_FILE[db]
	CUR_DB=db
}

function switch_db(db) {
	if (db != CUR_DB) {
		open_db(db)
		if (KEY_REQUIRED == 1) {
			print "PRAGMA key=\"" DB_KEY[db] "\";"
		}
		print ".user login " CUR_USER[CUR_DB]
	}
}

BEGIN {
	print ".shell rm -rf test.db test.db-journal"
	print ".shell rm -rf test3.db test3.db-journal"

	DB_FILE["db"]="test.db"
	DB_FILE["dummy"]="test-dummy.db"
	open_db("db")
}

# Preserve empty lines.
/^$/

# Preserve test numbers
/-api-/ {
	print ".print " $2
}

# Copy SQL statements.
/do_execsql_test/,/}/ {
	if ($1 != "do_execsql_test" && $1 != "}") {
		print $0
	}
}

/(execsql|catchsql) {/,/}/ {
	if ($2 == "{") {
		SQL_STMT=""
	} else if ($1 == "}") {
		if ($2 != "") {
			switch_db($2)
		} else {
			switch_db("db")
		}
		print SQL_STMT
	} else {
		SQL_STMT=SQL_STMT "\n" $0
	}
}

/eval {/,/}/ {
	if ($2 == "eval") {
		switch_db($1)
	} else if ($1 != "}") {
		print $0
	}
}

# Convert sqlite3 API calls.
/sqlite3 / {
	open_db("dummy")
	DB_FILE[$2]=$3
	open_db($2)
}

/sqlite3_key/ {
	if ($2 != CUR_DB) {
		open_db($2)
	}
	DB_KEY[$2]=$3
	print "PRAGMA key=\"" DB_KEY[$2] "\";"
}

/sqlite3_user_authenticate / {
	switch_db($2)
	print ".user login " $3 " " $4
	if (REQ_AUTH[DB_FILE[CUR_DB]] == 1) {
		CUR_USER[CUR_DB]=$3 " " $4
	}
}

/sqlite3_user_add / {
	switch_db($2)
	print ".user add " $3 " " $4 " " $5
	if ($5 == 1) {
		REQ_AUTH[DB_FILE[CUR_DB]]=1
	}
	if (REQ_AUTH[DB_FILE[CUR_DB]] == 1 && CUR_USER[CUR_DB] == "") {
		CUR_USER[CUR_DB]=$3 " " $4
	}
}

/sqlite3_user_change / {
	switch_db($2)
	print ".user edit " $3 " " $4 " " $5
}

/sqlite3_user_delete / {
	switch_db($2)
	print ".user delete " $3
}

# Convert file operations to shell commands.
/forcedelete / {
	sub(/ *forcedelete/, ".shell rm -rf")
	print $0
}

/file exists / {
	sub(/ *file exists/, ".shell test ! -e")
	print $0 "; echo $?"
}

/compare_file / {
	sub(/ *compare_file/, ".shell diff")
	print $0 "; echo $?"
}

/set fh \[open .* w\]/ {
	sub(/ *set fh \[open/, ".shell touch")
	sub(/w\]/, "")
	print $0
}

/set fh \[open .* a\]/ {
	sub(/ *set fh \[open/, ".shell echo a >>")
	sub(/a\]/, "")
	print $0
}

# Convert reset_db and close.
/reset_db/ {
	open_db("dummy")
	print ".shell rm -rf test.db test.db-journal"
	open_db("db")
}

/ close$/ {
	open_db("dummy")
}
