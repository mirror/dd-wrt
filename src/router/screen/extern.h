/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 * $Id$ GNU
 */

#if !defined(__GNUC__) || __GNUC__ < 2
#undef __attribute__
#define __attribute__(x)
#endif

#include <stdbool.h>

/* screen.c */
extern int   main __P((int, char **));
extern sigret_t SigHup __P(SIGPROTOARG);
extern void  eexit __P((int)) __attribute__((__noreturn__));
extern void  Detach __P((int));
extern void  Hangup __P((void));
extern void  Kill __P((int, int));
#ifdef USEVARARGS
extern void  Msg __P((int, const char *, ...)) __attribute__((format(printf, 2, 3)));
extern void  Panic __P((int, const char *, ...)) __attribute__((format(printf, 2, 3))) __attribute__((__noreturn__));
extern void  QueryMsg __P((int, const char *, ...)) __attribute__((format(printf, 2, 3)));
extern void  Dummy __P((int, const char *, ...)) __attribute__((format(printf, 2, 3)));
#else
extern void  Msg __P(());
extern void  Panic __P(());
extern void  QueryMsg __P(());
extern void  Dummy __P(());
#endif
extern void  Finit __P((int));
extern void  MakeNewEnv __P((void));
extern char *MakeWinMsg __P((char *, struct win *, int));
extern char *MakeWinMsgEv __P((char *, struct win *, int, int, struct event *, int));
extern int   AddWinMsgRend __P((const char *, int));
extern void  PutWinMsg __P((char *, int, int));
#ifdef BSDWAIT
extern void  WindowDied __P((struct win *, union wait, int));
#else
extern void  WindowDied __P((struct win *, int, int));
#endif
extern void  setbacktick __P((int, int, int, char **));

/* ansi.c */
extern void  ResetAnsiState __P((struct win *));
extern void  ResetWindow __P((struct win *));
extern void  ResetCharsets __P((struct win *));
extern void  WriteString __P((struct win *, char *, int));
extern void  ChangeAKA __P((struct win *, char *, int));
extern void  SetCharsets __P((struct win *, char *));
extern int   GetAnsiStatus __P((struct win *, char *));
extern void  WNewAutoFlow __P((struct win *, int));
extern void  WBell __P((struct win *, int));
extern void  WMsg __P((struct win *, int, char *));
extern void  WChangeSize __P((struct win *, int, int));
extern void  WindowChanged __P((struct win *, int));
extern int   MFindUsedLine __P((struct win *, int, int));

/* fileio.c */
extern int   StartRc __P((char *, int));
extern void  FinishRc __P((char *));
extern void  RcLine __P((char *, int));
extern FILE *secfopen __P((char *, char *));
extern int   secopen __P((char *, int, int));
extern void  WriteFile __P((struct acluser *, char *, int));
extern char *ReadFile __P((char *, int *));
extern void  KillBuffers __P((void));
extern int   printpipe __P((struct win *, char *));
extern int   readpipe __P((char **));
extern void  RunBlanker __P((char **));
extern void  do_source __P((char *));

/* tty.c */
extern int   OpenTTY __P((char *, char *));
extern void  InitTTY __P((struct mode *, int));
extern void  GetTTY __P((int, struct mode *));
extern void  SetTTY __P((int, struct mode *));
extern void  SetMode __P((struct mode *, struct mode *, int, int));
extern void  SetFlow __P((int));
extern void  SendBreak __P((struct win *, int, int));
extern int   TtyGrabConsole __P((int, int, char *));
extern char *TtyGetModemStatus __P((int, char *));
#ifdef DEBUG
extern void  DebugTTY __P((struct mode *));
#endif /* DEBUG */
extern int   fgtty __P((int));
extern void  brktty __P((int));
extern struct baud_values *lookup_baud __P((int bps));
extern int   SetBaud __P((struct mode *, int, int));
extern int   SttyMode __P((struct mode *, char *));
extern int   CheckTtyname __P((char *));
extern char  *GetPtsPathOrSymlink __P((int));

/* mark.c */
extern int   GetHistory __P((void));
extern void  MarkRoutine __P((void));
extern void  revto_line __P((int, int, int));
extern void  revto __P((int, int));
extern int   InMark __P((void));
extern void  MakePaster __P((struct paster *, char *, int, int));
extern void  FreePaster __P((struct paster *));

/* search.c */
extern void  Search __P((int));
extern void  ISearch __P((int));

/* input.c */
extern void  inp_setprompt __P((char *, char *));
extern void  Input __P((char *, int, int, void (*)(char *, int, char *), char *, int));
extern int   InInput __P((void));

/* help.c */
extern void  exit_with_usage __P((char *, char *, char *));
extern void  display_help __P((char *, struct action *));
extern void  display_copyright __P((void));
extern void  display_displays __P((void));
extern void  display_bindkey __P((char *, struct action *));
extern int   InWList __P((void));
extern void  WListUpdatecv __P((struct canvas *, struct win *));
extern void  WListLinkChanged __P((void));
#ifdef ZMODEM
extern void  ZmodemPage __P((void));
#endif

/* window.c */
extern int   MakeWindow __P((struct NewWindow *));
extern int   RemakeWindow __P((struct win *));
extern void  FreeWindow __P((struct win *));
#ifdef PSEUDOS
extern int   winexec __P((char **));
extern void  FreePseudowin __P((struct win *));
#endif
extern void  nwin_compose __P((struct NewWindow *, struct NewWindow *, struct NewWindow *));
extern int   DoStartLog __P((struct win *, char *, int));
extern int   ReleaseAutoWritelock __P((struct display *, struct win *));
extern int   ObtainAutoWritelock __P((struct display *, struct win *));
extern void  CloseDevice __P((struct win *));
#ifdef ZMODEM
extern void  zmodem_abort __P((struct win *, struct display *));
#endif
#ifndef HAVE_EXECVPE
extern void  execvpe __P((char *, char **, char **));
#endif

/* utmp.c */
#ifdef UTMPOK
extern void  InitUtmp __P((void));
extern void  RemoveLoginSlot __P((void));
extern void  RestoreLoginSlot __P((void));
extern int   SetUtmp __P((struct win *));
extern int   RemoveUtmp __P((struct win *));
#endif /* UTMPOK */
extern void  SlotToggle __P((int));
#ifdef USRLIMIT
extern int   CountUsers __P((void));
#endif
#ifdef CAREFULUTMP
extern void   CarefulUtmp __P((void));
#else
# define CarefulUtmp()  /* nothing */
#endif /* CAREFULUTMP */


/* loadav.c */
#ifdef LOADAV
extern void  InitLoadav __P((void));
extern void  AddLoadav __P((char *));
#endif

/* pty.c */
extern int   OpenPTY __P((char **));
extern void  InitPTY __P((int));

/* process.c */
extern void  InitKeytab __P((void));
extern void  ProcessInput __P((char *, int));
#ifdef MAPKEYS
extern void  ProcessInput2 __P((char *, int));
#endif
extern void  DoProcess __P((struct win *, char **, int *, struct paster *));
extern void  DoAction  __P((struct action *, int));
extern int   FindCommnr __P((const char *));
extern void  DoCommand __P((char **, int *));
extern void  Activate __P((int));
extern void  KillWindow __P((struct win *));
extern void  SetForeWindow __P((struct win *));
extern int   Parse __P((char *, int, char **, int *));
extern void  SetEscape __P((struct acluser *, int, int));
extern void  DoScreen __P((char *, char **));
extern int   IsNumColon __P((char *, int, char *, int));
extern void  ShowWindows __P((int));
extern char *AddWindows __P((char *, int, int, int));
extern char *AddWindowFlags __P((char *, int, struct win *));
extern char *AddOtherUsers __P((char *, int, struct win *));
extern int   WindowByNoN __P((char *));
extern struct win *FindNiceWindow __P((struct win *, char *));
#ifdef COPY_PASTE
extern int   CompileKeys __P((char *, int, unsigned char *));
#endif
#ifdef RXVT_OSC
extern void  RefreshXtermOSC __P((void));
#endif
extern int   ParseSaveStr __P((struct action *act, char **));
extern int   ParseNum __P((struct action *act, int *));
extern int   ParseSwitch __P((struct action *, int *));
extern int   ParseAttrColor __P((char *, char *, int));
extern void  ApplyAttrColor __P((int, struct mchar *));
extern void  SwitchWindow __P((int));
extern int   StuffKey __P((int));

/* termcap.c */
extern int   InitTermcap __P((int, int));
extern char *MakeTermcap __P((int));
extern void  DumpTermcap __P((int, FILE *));
extern char *gettermcapstring __P((char *));
#ifdef MAPKEYS
extern int   remap __P((int, int));
extern void  CheckEscape __P((void));
#endif
extern int   CreateTransTable __P((char *));
extern void  FreeTransTable __P((void));

/* attacher.c */
extern int   Attach __P((int));
extern void  Attacher __P((void));
extern sigret_t AttacherFinit __P(SIGPROTOARG);
extern void  SendCmdMessage __P((char *, char *, char **, int));

/* display.c */
extern struct display *MakeDisplay __P((char *, char *, char *, int, int, struct mode *));
extern void  FreeDisplay __P((void));
extern void  DefProcess __P((char **, int *));
extern void  DefRedisplayLine __P((int, int, int, int));
extern void  DefClearLine __P((int, int, int, int));
extern int   DefRewrite __P((int, int, int, struct mchar *, int));
extern int   DefResize __P((int, int));
extern void  DefRestore __P((void));
extern void  AddCStr __P((char *));
extern void  AddCStr2 __P((char *, int));
extern void  InitTerm __P((int));
extern void  FinitTerm __P((void));
extern void  PUTCHAR __P((int));
extern void  PUTCHARLP __P((int));
extern void  ClearAll __P((void));
extern void  ClearArea __P((int, int, int, int, int, int, int, int));
extern void  ClearLine __P((struct mline *, int, int, int, int));
extern void  RefreshAll __P((int));
extern void  RefreshArea __P((int, int, int, int, int));
extern void  RefreshLine __P((int, int, int, int));
extern void  Redisplay __P((int));
extern void  RedisplayDisplays __P((int));
extern void  ShowHStatus __P((char *));
extern void  RefreshHStatus __P((void));
extern void  DisplayLine __P((struct mline *, struct mline *, int, int, int));
extern void  GotoPos __P((int, int));
extern int   CalcCost __P((char *));
extern void  ScrollH __P((int, int, int, int, int, struct mline *));
extern void  ScrollV __P((int, int, int, int, int, int));
extern void  PutChar __P((struct mchar *, int, int));
extern void  InsChar __P((struct mchar *, int, int, int, struct mline *));
extern void  WrapChar __P((struct mchar *, int, int, int, int, int, int, int));
extern void  ChangeScrollRegion __P((int, int));
extern void  InsertMode __P((int));
extern void  KeypadMode __P((int));
extern void  CursorkeysMode __P((int));
extern void  ReverseVideo __P((int));
extern void  CursorVisibility __P((int));
extern void  MouseMode __P((int));
extern void  ExtMouseMode __P((int));
extern void  SetFont __P((int));
extern void  SetAttr __P((int));
extern void  SetColor __P((int, int));
extern void  SetRendition __P((struct mchar *));
extern void  SetRenditionMline __P((struct mline *, int));
extern void  MakeStatus __P((char *));
extern void  RemoveStatus __P((void));
extern int   ResizeDisplay __P((int, int));
extern void  AddStr __P((char *));
extern void  AddStrn __P((char *, int));
extern void  Flush __P((int));
extern void  freetty __P((void));
extern void  Resize_obuf __P((void));
#ifdef AUTO_NUKE
extern void  NukePending __P((void));
#endif
#ifdef RXVT_OSC
extern void  ClearAllXtermOSC __P((void));
extern void  SetXtermOSC __P((int, char *, char *));
#endif
#ifdef COLOR
extern int   color256to16 __P((int));
# ifdef COLORS256
extern int   color256to88 __P((int));
# endif
#endif
extern void  ResetIdle __P((void));
extern void  KillBlanker __P((void));
extern void  DisplaySleep1000 __P((int, int));

/* resize.c */
extern int   ChangeWindowSize __P((struct win *, int, int, int));
extern void  ChangeScreenSize __P((int, int, int));
extern void  CheckScreenSize __P((int));
extern char *xrealloc __P((char *, int));
extern void  ResizeLayersToCanvases __P((void));
extern void  ResizeLayer __P((struct layer *, int, int, struct display *));
extern int   MayResizeLayer __P((struct layer *));
extern void  FreeAltScreen __P((struct win *));
extern void  EnterAltScreen __P((struct win *));
extern void  LeaveAltScreen __P((struct win *));

/* sched.c */
extern void  evenq __P((struct event *));
extern void  evdeq __P((struct event *));
extern void  SetTimeout __P((struct event *, int));
extern void  sched __P((void));

/* socket.c */
extern int   FindSocket __P((int *, int *, int *, char *, bool *));
extern int   MakeClientSocket __P((int, bool));
extern int   MakeServerSocket __P((bool));
extern int   RecoverSocket __P((void));
extern int   chsock __P((void));
extern void  ReceiveMsg __P((void));
extern void  SendCreateMsg __P((char *, struct NewWindow *));
extern int   SendErrorMsg __P((char *, char *));
extern int   SendAttachMsg __P((int, struct msg *, int));
extern void  ReceiveRaw __P((int));
extern bool  IsSocket __P((const char *));

/* misc.c */
extern char *SaveStr __P((const char *));
extern char *SaveStrn __P((const char *, int));
extern char *InStr __P((char *, const char *));
#ifndef HAVE_STRERROR
extern char *strerror __P((int));
#endif
extern void  centerline __P((char *, int));
extern void  leftline __P((char *, int, struct mchar *));
extern char *Filename __P((char *));
extern char *stripdev __P((char *));
#ifdef NEED_OWN_BCOPY
extern void  xbcopy __P((char *, char *, int));
#endif
extern void  bclear __P((char *, int));
extern void  closeallfiles __P((int));
extern int   UserContext __P((void));
extern void  UserReturn __P((int));
extern int   UserStatus __P((void));
#if defined(POSIX) || defined(hpux)
extern void (*xsignal __P((int, void (*)SIGPROTOARG))) __P(SIGPROTOARG);
#endif
#ifndef HAVE_RENAME
extern int   rename __P((char *, char *));
#endif
#if defined(HAVE_SETEUID) || defined(HAVE_SETREUID)
extern void  xseteuid  __P((int));
extern void  xsetegid  __P((int));
#endif
extern int   AddXChar __P((char *, int));
extern int   AddXChars __P((char *, int, char *));
extern void  xsetenv  __P((char *, char *));
extern void  sleep1000 __P((int));
#ifdef DEBUG
extern void  opendebug __P((int, int));
#endif
#ifdef USEVARARGS
# ifndef HAVE_VSNPRINTF
extern int   xvsnprintf __P((char *, int, char *, va_list));
# endif
#else
extern int   xsnprintf __P(());
#endif


/* acl.c */
#ifdef MULTIUSER
extern int   AclCheckPermWin __P((struct acluser *, int, struct win *));
extern int   AclCheckPermCmd __P((struct acluser *, int, struct comm *));
extern int   AclSetPerm __P((struct acluser *, struct acluser *, char *, char *));
extern int   AclUmask __P((struct acluser *, char *, char **));
extern int   UsersAcl __P((struct acluser *, int, char **));
extern void  AclWinSwap __P((int, int));
extern int   NewWindowAcl __P((struct win *, struct acluser *));
extern void  FreeWindowAcl __P((struct win *));
extern char *DoSu __P((struct acluser **, char *, char *, char *));
extern int   AclLinkUser __P((char *, char *));
#endif /* MULTIUSER */
extern int   UserFreeCopyBuffer __P((struct acluser *));
extern struct acluser **FindUserPtr __P((char *));
extern int   UserAdd __P((char *, char *, struct acluser **));
extern int   UserDel __P((char *, struct acluser **));


/* braile.c */
#ifdef HAVE_BRAILLE
extern void  InitBraille __P((void));
extern void  RefreshBraille __P((void));
extern void  DoBrailleAction __P((struct action *, int));
extern void  BGotoPos __P((struct layer *, int, int));
extern void  BPutChar __P((struct layer *, struct mchar *, int, int));
extern void  BPutStr __P((struct layer *, char *, int, struct mchar *, int, int));
extern void  BCDisplayLine __P((struct layer *, struct mline *, int, int, int, int));
#endif




/* layer.c */
extern void  LGotoPos __P((struct layer *, int, int));
extern void  LPutChar __P((struct layer *, struct mchar *, int, int));
extern void  LInsChar __P((struct layer *, struct mchar *, int, int, struct mline *));
extern void  LPutStr __P((struct layer *, char *, int, struct mchar *, int, int));
extern void  LPutWinMsg __P((struct layer *, char *, int, struct mchar *, int, int));
extern void  LScrollH __P((struct layer *, int, int, int, int, int, struct mline *));
extern void  LScrollV __P((struct layer *, int, int, int, int));
extern void  LClearAll __P((struct layer *, int));
extern void  LClearArea __P((struct layer *, int, int, int, int, int, int));
extern void  LClearLine __P((struct layer *, int, int, int, int, struct mline *));
extern void  LRefreshAll __P((struct layer *, int));
extern void  LCDisplayLine __P((struct layer *, struct mline *, int, int, int, int));
extern void  LCDisplayLineWrap __P((struct layer *, struct mline *, int, int, int, int));
extern void  LSetRendition __P((struct layer *, struct mchar *));
extern void  LWrapChar  __P((struct layer *, struct mchar *, int, int, int, int));
extern void  LCursorVisibility __P((struct layer *, int));
extern void  LSetFlow __P((struct layer *, int));
extern void  LKeypadMode __P((struct layer *, int));
extern void  LCursorkeysMode __P((struct layer *, int));
extern void  LMouseMode __P((struct layer *, int));
extern void  LExtMouseMode __P((struct layer *, int));
#if defined(USEVARARGS)
extern void  LMsg __P((int, const char *, ...)) __attribute__((format(printf, 2, 3)));
#else
extern void  LMsg __P(());
#endif
extern void  KillLayerChain __P((struct layer *));
extern int   InitOverlayPage __P((int, struct LayFuncs *, int));
extern void  ExitOverlayPage __P((void));
extern int   LayProcessMouse __P((struct layer *, unsigned char));
extern void  LayProcessMouseSwitch __P((struct layer *, int));

/* teln.c */
#ifdef BUILTIN_TELNET
extern int   TelOpenAndConnect __P((struct win *));
extern int   TelIsline __P((struct win *p));
extern void  TelProcessLine __P((char **, int *));
extern int   DoTelnet __P((char *, int *, int));
extern int   TelIn __P((struct win *, char *, int, int));
extern void  TelBreak __P((struct win *));
extern void  TelWindowSize __P((struct win *));
extern void  TelStatus __P((struct win *, char *, int));
#endif

/* nethack.c */
extern const char *DoNLS __P((const char *));

/* encoding.c */
#ifdef ENCODINGS
# ifdef UTF8
extern void  InitBuiltinTabs __P((void));
extern struct mchar *recode_mchar __P((struct mchar *, int, int));
extern struct mline *recode_mline __P((struct mline *, int, int, int));
extern int   FromUtf8 __P((int, int *));
extern void  AddUtf8 __P((int));
extern int   ToUtf8 __P((char *, int));
extern int   ToUtf8_comb __P((char *, int));
extern int   utf8_isdouble __P((int));
extern int   utf8_iscomb __P((int));
extern void  utf8_handle_comb __P((int, struct mchar *));
extern int   ContainsSpecialDeffont __P((struct mline *, int, int, int));
extern int   LoadFontTranslation __P((int, char *));
extern void  LoadFontTranslationsForEncoding __P((int));
# endif	/* UTF8 */
extern void  WinSwitchEncoding __P((struct win *, int));
extern int   FindEncoding __P((char *));
extern char *EncodingName __P((int));
extern int   EncodingDefFont __P((int));
extern void  ResetEncoding __P((struct win *));
extern int   CanEncodeFont __P((int, int));
extern int   DecodeChar __P((int, int, int *));
extern int   RecodeBuf __P((unsigned char *, int, int, int, unsigned char *));
# ifdef DW_CHARS
extern int   PrepareEncodedChar __P((int));
# endif
#endif
extern int   EncodeChar __P((char *, int, int, int *));

/* layout.c */
extern void  RemoveLayout __P((struct layout *));
extern int   LayoutDumpCanvas __P((struct canvas *, char *));
