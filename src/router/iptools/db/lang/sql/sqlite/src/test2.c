/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the pager.c module in SQLite.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
*/
#include "sqliteInt.h"
#include "tcl.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern const char *sqlite3ErrName(int);

#ifndef SQLITE_OMIT_DISKIO

/*
** Page size and reserved size used for testing.
*/
static int test_pagesize = 1024;

/*
** Usage:   fake_big_file  N  FILENAME
**
** Write a few bytes at the N megabyte point of FILENAME.  This will
** create a large file.  If the file was a valid SQLite database, then
** the next time the database is opened, SQLite will begin allocating
** new pages after N.  If N is 2096 or bigger, this will test the
** ability of SQLite to write to large files.
*/
static int fake_big_file(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  int argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  sqlite3_vfs *pVfs;
  sqlite3_file *fd = 0;
  int rc;
  int n;
  i64 offset;
  char *zFile;
  int nFile;
  if( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
       " N-MEGABYTES FILE\"", 0);
    return TCL_ERROR;
  }
  if( Tcl_GetInt(interp, argv[1], &n) ) return TCL_ERROR;

  /*
   * This does not work with Berkeley DB. Making a large file does not cause
   * DB to skip the existing pages.
   */
  return TCL_ERROR;

  pVfs = sqlite3_vfs_find(0);
  nFile = (int)strlen(argv[2]);
  zFile = sqlite3_malloc( nFile+2 );
  if( zFile==0 ) return TCL_ERROR;
  memcpy(zFile, argv[2], nFile+1);
  zFile[nFile+1] = 0;
  rc = sqlite3OsOpenMalloc(pVfs, zFile, &fd, 
      (SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_MAIN_DB), 0
  );
  if( rc ){
    Tcl_AppendResult(interp, "open failed: ", sqlite3ErrName(rc), 0);
    sqlite3_free(zFile);
    return TCL_ERROR;
  }
  offset = n;
  offset *= 1024*1024;
  rc = sqlite3OsWrite(fd, "Hello, World!", 14, offset);
  sqlite3OsCloseFree(fd);
  sqlite3_free(zFile);
  if( rc ){
    Tcl_AppendResult(interp, "write failed: ", sqlite3ErrName(rc), 0);
    return TCL_ERROR;
  }
  return TCL_OK;
}
#endif


/*
** The sqlite3FaultSim() callback:
*/
static Tcl_Interp *faultSimInterp = 0;
static int faultSimScriptSize = 0;
static char *faultSimScript;
static int faultSimCallback(int x){
  char zInt[30];
  int i;
  int isNeg;
  int rc;
  if( x==0 ){
    memcpy(faultSimScript+faultSimScriptSize, "0", 2);
  }else{
    /* Convert x to text without using any sqlite3 routines */
    if( x<0 ){
      isNeg = 1;
      x = -x;
    }else{
      isNeg = 0;
    }
    zInt[sizeof(zInt)-1] = 0;
    for(i=sizeof(zInt)-2; i>0 && x>0; i--, x /= 10){
      zInt[i] = (x%10) + '0';
    }
    if( isNeg ) zInt[i--] = '-';
    memcpy(faultSimScript+faultSimScriptSize, zInt+i+1, sizeof(zInt)-i);
  }
  rc = Tcl_Eval(faultSimInterp, faultSimScript);
  if( rc ){
    fprintf(stderr, "fault simulator script failed: [%s]", faultSimScript);
    rc = SQLITE_ERROR;
  }else{
    rc = atoi(Tcl_GetStringResult(faultSimInterp));
  }
  Tcl_ResetResult(faultSimInterp);
  return rc;
}

/*
** sqlite3_test_control_fault_install SCRIPT
**
** Arrange to invoke SCRIPT with the integer argument to sqlite3FaultSim()
** appended, whenever sqlite3FaultSim() is called.  Or, if SCRIPT is the
** empty string, cancel the sqlite3FaultSim() callback.
*/
static int faultInstallCmd(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  int argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  const char *zScript;
  int nScript;
  int rc;
  if( argc!=1 && argc!=2 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SCRIPT\"", (void*)0);
  }
  zScript = argc==2 ? argv[1] : "";
  nScript = (int)strlen(zScript);
  if( faultSimScript ){
    free(faultSimScript);
    faultSimScript = 0;
  }
  if( nScript==0 ){
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, 0);
  }else{
    faultSimScript = malloc( nScript+100 );
    if( faultSimScript==0 ){
      Tcl_AppendResult(interp, "out of memory", (void*)0);
      return SQLITE_ERROR;
    }
    memcpy(faultSimScript, zScript, nScript);
    faultSimScript[nScript] = ' ';
    faultSimScriptSize = nScript+1;
    faultSimInterp = interp;
    rc = sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, faultSimCallback);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  return SQLITE_OK;
}

/*
** sqlite3BitvecBuiltinTest SIZE PROGRAM
**
** Invoke the SQLITE_TESTCTRL_BITVEC_TEST operator on test_control.
** See comments on sqlite3BitvecBuiltinTest() for additional information.
*/
static int testBitvecBuiltinTest(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  int argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  int sz, rc;
  int nProg = 0;
  int aProg[100];
  const char *z;
  if( argc!=3 ){
    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                     " SIZE PROGRAM\"", (void*)0);
  }
  if( Tcl_GetInt(interp, argv[1], &sz) ) return TCL_ERROR;
  z = argv[2];
  while( nProg<99 && *z ){
    while( *z && !sqlite3Isdigit(*z) ){ z++; }
    if( *z==0 ) break;
    aProg[nProg++] = atoi(z);
    while( sqlite3Isdigit(*z) ){ z++; }
  }
  aProg[nProg] = 0;
  rc = sqlite3_test_control(SQLITE_TESTCTRL_BITVEC_TEST, sz, aProg);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  return TCL_OK;
}  

static int t2_tcl_function_stub(
  void *NotUsed,
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  int argc,              /* Number of arguments */
  const char **argv      /* Text of each argument */
){
  return TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
int Sqlitetest2_Init(Tcl_Interp *interp){
  static struct {
    char *zName;
    Tcl_CmdProc *xProc;
  } aCmd[] = {
    { "pager_open",              (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_close",             (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_commit",            (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_rollback",          (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_stmt_begin",        (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_stmt_commit",       (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_stmt_rollback",     (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_stats",             (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_pagecount",         (Tcl_CmdProc*)t2_tcl_function_stub },
    { "page_get",                (Tcl_CmdProc*)t2_tcl_function_stub },
    { "page_lookup",             (Tcl_CmdProc*)t2_tcl_function_stub },
    { "page_unref",              (Tcl_CmdProc*)t2_tcl_function_stub },
    { "page_read",               (Tcl_CmdProc*)t2_tcl_function_stub },
    { "page_write",              (Tcl_CmdProc*)t2_tcl_function_stub },
    { "page_number",             (Tcl_CmdProc*)t2_tcl_function_stub },
    { "pager_truncate",          (Tcl_CmdProc*)t2_tcl_function_stub },
#ifndef SQLITE_OMIT_DISKIO
    { "fake_big_file",           (Tcl_CmdProc*)fake_big_file       },
#endif
    { "sqlite3BitvecBuiltinTest",(Tcl_CmdProc*)testBitvecBuiltinTest     },
    { "sqlite3_test_control_pending_byte", (Tcl_CmdProc*)t2_tcl_function_stub },
    { "sqlite3_test_control_fault_install", (Tcl_CmdProc*)faultInstallCmd },
  };
  int i;
  for(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }
#ifndef SQLITE_OMIT_WSD
  Tcl_LinkVar(interp, "sqlite_pending_byte",
     (char*)&sqlite3PendingByte, TCL_LINK_INT | TCL_LINK_READ_ONLY);
#endif
  return TCL_OK;
}
