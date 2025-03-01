@echo off
:: This script upgrades SQL databases from BDB 5.0 to early 6.1
:: to late 6.1 and up by reindexing them.
::

FOR %%V IN (%*) DO (
    @echo Recovering database %%V
    db_recover -f -h %%V-journal
    @echo Reindexing database %%V
    @echo .quit | dbsql.exe -cmd REINDEX  %%V
)
