$ ver = f$verify(0)
$!  Makefile for VMS
$ Make = ""
$ ccopt="/include=[]"
$ cc = "CC"
$!
$! Check for MMK/MMS
$!
$ If F$Search ("Sys$System:MMS.EXE") .nes. "" Then Make = "MMS"
$ If F$Type (MMK) .eqs. "STRING" Then Make = "MMK"
$!
$! Look for the compiler used
$!
$ if p1.nes.""
$   then
$     if f$locate("G",p1).ne.f$length(p1) then goto gnuc
$     if f$locate("V",p1).ne.f$length(p1) then goto vaxc
$     if f$locate("D",p1).ne.f$length(p1) then goto decc
$ endif
$!
$!  Option file for test applications
$!
$ open/write optf client.opt
$ write optf "slang/library"
$!
$DECC:
$ if f$search("SYS$SYSTEM:DECC$COMPILER.EXE").nes.""
$ then
$   write sys$output "DECC compiler found"
$   defs = "VMS"
$   ccopt = "/decc/prefix=all/float=ieee/name=(as_is,short)/ieee=denorm"+ccopt
$!   ccopt = "/decc/prefix=all/float=ieee/name=(as_is,short)"+ccopt
$   goto compile
$ endif
$!
$VAXC:
$ if f$search("SYS$SYSTEM:VAXC.EXE").nes.""
$ then
$   write sys$output "VAXC compiler found"
$   defs = "FLOAT_TYPE"
$   write optf "sys$library:vaxcrtl.exe/share"
$   goto compile
$ endif
$!
$GNUC:
$ if f$trnlnm("GNU_CC").nes.""
$ then
$   write sys$output "GNUC compiler found"
$   defs = "FLOAT_TYPE"
$   cc="GCC"
$   ccopt = "/warnings/nocase_hack/nolist/optimize=4"+ccopt
$   write optf "gnu_cc:[000000]gcclib.olb/lib"
$   goto compile
$ endif
$!
$! No compiler found - Warning and Exit
$!
$ close optf
$ type sys$input
FATAL: No C-compiler found - Can't build Slang on this system.

$ goto The_exit
$!
$COMPILE:
$!
$ close optf
$ files = "slang,slparse,slmath,slstd,slarray,slregexp,slerr"
$ files = files + ",slrline,slgetkey,slvmstty,slkeymap,sltoken,slcurses"
$ files = files + ",slmemcpy,slmemcmp,slmemset,slmemchr,slmisc,slsig"
$ files = files + ",sldisply,slsmg,slsearch,slcmd,slmalloc,slclass"
$ files = files + ",slprepr,sltypes,slscroll,slsignal,slkeypad,slerrno"
$ files = files + ",slstruct,slcmplex,slarrfun,slimport,slpath,slarith,slassoc"
$ files = files + ",slcompat,slposdir,slstdio,slproc,sltime,slstrops"
$ files = files + ",slbstr,slpack,slintall,slistruc,slposio,slnspace,slarrmis"
$ files = files + ",slospath,slscanf,slstring,sllist,slexcept,slfpu,slboseos"
$ files = files + ",sllower,slupper,slischar,slutf8,slwcwidth,slwclut,slcommon"
$!
$!  simple make
$!
$  copy slconfig.h config.h
$  purge config.h
$  if (Make .nes. "")
$   then
$    'Make'/Macro = (cc="''CC'", cflags="''CCOPt'/define=(''defs')")
$  else
$    count = 0
$    olbf = f$search("slang.olb")
$    if (olbf .eqs. "") then library/object/create slang.olb
$    next_file:
$      f = f$element(count, ",", files)
$      count = count + 1
$      if (f .eqs. ",") then goto testfiles
$      objf = f$search("''f'.obj")
$      if (objf .eqs. "") then goto compile_it
$      tobj = f$file_attr(objf, "RDT")
$      tc  = f$file_attr("''f'.c", "RDT")
$      if (f .eqs. "sysdep")
$      then
$  	 if ( f$cvtime(tobj) .lts. f$cvtime(f$file_attr("vms.c","RDT"))) -
   	   then goto compile_it
$      endif
$      if (f$cvtime(tc) .lts. f$cvtime(tobj)) then goto next_file
$    compile_it:
$      write sys$output "''CC' ''CCOPT'/define=(''defs') ''f'.c"
$      'CC' 'CCOPT'/define=('defs') 'f'.c
$      library/replace slang.olb 'f'
$      goto next_file
$  testfiles:
$    purge slang.olb
$    write sys$output "''CC' ''CCOPT'/define=(''defs') calc.c"
$      'CC' 'CCOPT'/define=('defs') calc.c
$      write sys$output "link/exec=calc calc client.opt/opt"
$      link/exec=calc calc, client.opt/opt
$!    write sys$output "''CC' ''CCOPT'/define=(''defs') worm.c"
$!      'CC' 'CCOPT'/define=('defs') worm.c
$!      write sys$output "link/exec=worm worm client.opt/opt"
$!      link/exec=worm worm, client.opt/opt
$  endif
$The_exit:
$ x = f$verify(ver)
$ exit $status
