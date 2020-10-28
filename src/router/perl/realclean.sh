cd cpan/Archive-Tar
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Attribute/Handlers.pm
rm -f lib/AutoLoader.pm
rm -f lib/AutoSplit.pm
rm -f lib/B/Debug.pm
cd cpan/CPAN
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd cpan/CPAN-Meta
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/CPAN/Meta/Requirements.pm
rm -f lib/CPAN/Meta/YAML.pm
rm -f lib/Carp.pm
rm -f lib/Carp/Heavy.pm
rm -f lib/Config/Perl/V.pm
rm -f lib/Devel/SelfStubber.pm
rm -f lib/Digest.pm
rm -f lib/Digest/base.pm
rm -f lib/Digest/file.pm
rm -f lib/Dumpvalue.pm
rm -f lib/Env.pm
cd ext/Errno
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Exporter.pm
rm -f lib/Exporter/Heavy.pm
rm -f lib/ExtUtils/CBuilder.pm
rm -f lib/ExtUtils/CBuilder/Base.pm
rm -f lib/ExtUtils/CBuilder/Platform/Unix.pm
rm -f lib/ExtUtils/CBuilder/Platform/VMS.pm
rm -f lib/ExtUtils/CBuilder/Platform/Windows.pm
rm -f lib/ExtUtils/CBuilder/Platform/Windows/BCC.pm
rm -f lib/ExtUtils/CBuilder/Platform/Windows/GCC.pm
rm -f lib/ExtUtils/CBuilder/Platform/Windows/MSVC.pm
rm -f lib/ExtUtils/CBuilder/Platform/aix.pm
rm -f lib/ExtUtils/CBuilder/Platform/android.pm
rm -f lib/ExtUtils/CBuilder/Platform/cygwin.pm
rm -f lib/ExtUtils/CBuilder/Platform/darwin.pm
rm -f lib/ExtUtils/CBuilder/Platform/dec_osf.pm
rm -f lib/ExtUtils/CBuilder/Platform/os2.pm
rm -f lib/ExtUtils/Constant.pm
rm -f lib/ExtUtils/Constant/Base.pm
rm -f lib/ExtUtils/Constant/ProxySubs.pm
rm -f lib/ExtUtils/Constant/Utils.pm
rm -f lib/ExtUtils/Constant/XS.pm
rm -f lib/ExtUtils/Install.pm
rm -f lib/ExtUtils/Installed.pm
rm -f lib/ExtUtils/Packlist.pm
cd cpan/ExtUtils-MakeMaker
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd cpan/ExtUtils-Manifest
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/ExtUtils/Miniperl.pm
cd dist/ExtUtils-ParseXS
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/File/Fetch.pm
rm -f lib/File/Find.pm
rm -f lib/File/Path.pm
rm -f lib/File/Temp.pm
rm -f lib/FileCache.pm
rm -f lib/Filter/Simple.pm
rm -f lib/Getopt/Long.pm
cd cpan/HTTP-Tiny
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/I18N/Collate.pm
rm -f lib/I18N/LangTags.pm
rm -f lib/I18N/LangTags/Detect.pm
rm -f lib/I18N/LangTags/List.pm
cd dist/lib
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd cpan/IO-Compress
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/IO/Socket/IP.pm
rm -f lib/IO/Zlib.pm
rm -f lib/IPC/Cmd.pm
rm -f lib/IPC/Open2.pm
rm -f lib/IPC/Open3.pm
cd cpan/JSON-PP
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Locale/Codes.pm
rm -f lib/Locale/Codes.pod
rm -f lib/Locale/Codes/Changes.pod
rm -f lib/Locale/Codes/Constants.pm
rm -f lib/Locale/Codes/Country.pm
rm -f lib/Locale/Codes/Country.pod
rm -f lib/Locale/Codes/Country_Codes.pm
rm -f lib/Locale/Codes/Country_Retired.pm
rm -f lib/Locale/Codes/Currency.pm
rm -f lib/Locale/Codes/Currency.pod
rm -f lib/Locale/Codes/Currency_Codes.pm
rm -f lib/Locale/Codes/Currency_Retired.pm
rm -f lib/Locale/Codes/LangExt.pm
rm -f lib/Locale/Codes/LangExt.pod
rm -f lib/Locale/Codes/LangExt_Codes.pm
rm -f lib/Locale/Codes/LangExt_Retired.pm
rm -f lib/Locale/Codes/LangFam.pm
rm -f lib/Locale/Codes/LangFam.pod
rm -f lib/Locale/Codes/LangFam_Codes.pm
rm -f lib/Locale/Codes/LangFam_Retired.pm
rm -f lib/Locale/Codes/LangVar.pm
rm -f lib/Locale/Codes/LangVar.pod
rm -f lib/Locale/Codes/LangVar_Codes.pm
rm -f lib/Locale/Codes/LangVar_Retired.pm
rm -f lib/Locale/Codes/Language.pm
rm -f lib/Locale/Codes/Language.pod
rm -f lib/Locale/Codes/Language_Codes.pm
rm -f lib/Locale/Codes/Language_Retired.pm
rm -f lib/Locale/Codes/Script.pm
rm -f lib/Locale/Codes/Script.pod
rm -f lib/Locale/Codes/Script_Codes.pm
rm -f lib/Locale/Codes/Script_Retired.pm
rm -f lib/Locale/Codes/Types.pod
rm -f lib/Locale/Country.pm
rm -f lib/Locale/Country.pod
rm -f lib/Locale/Currency.pm
rm -f lib/Locale/Currency.pod
rm -f lib/Locale/Language.pm
rm -f lib/Locale/Language.pod
rm -f lib/Locale/Script.pm
rm -f lib/Locale/Script.pod
rm -f lib/Locale/Maketext.pm
rm -f lib/Locale/Maketext.pod
rm -f lib/Locale/Maketext/Cookbook.pod
rm -f lib/Locale/Maketext/Guts.pm
rm -f lib/Locale/Maketext/GutsLoader.pm
rm -f lib/Locale/Maketext/TPJ13.pod
rm -f lib/Locale/Maketext/Simple.pm
rm -f lib/Math/BigFloat.pm
rm -f lib/Math/BigInt.pm
rm -f lib/Math/BigInt/Calc.pm
rm -f lib/Math/BigInt/CalcEmu.pm
rm -f lib/Math/BigInt/Lib.pm
rm -f lib/Math/BigRat.pm
rm -f lib/Math/Complex.pm
rm -f lib/Math/Trig.pm
rm -f lib/Memoize.pm
rm -f lib/Memoize/AnyDBM_File.pm
rm -f lib/Memoize/Expire.pm
rm -f lib/Memoize/ExpireFile.pm
rm -f lib/Memoize/ExpireTest.pm
rm -f lib/Memoize/NDBM_File.pm
rm -f lib/Memoize/SDBM_File.pm
rm -f lib/Memoize/Storable.pm
cd dist/Module-CoreList
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Module/Load.pm
rm -f lib/Module/Load/Conditional.pm
rm -f lib/Module/Loaded.pm
cd cpan/Module-Metadata
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/NEXT.pm
rm -f lib/Net/Ping.pm
rm -f lib/Params/Check.pm
rm -f lib/Perl/OSType.pm
rm -f lib/PerlIO/via/QuotedPrint.pm
cd cpan/Pod-Checker
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Pod/Escapes.pm
rm -f lib/Pod/Simple.pm
rm -f lib/Pod/Simple.pod
rm -f lib/Pod/Simple/BlackBox.pm
rm -f lib/Pod/Simple/Checker.pm
rm -f lib/Pod/Simple/Debug.pm
rm -f lib/Pod/Simple/DumpAsText.pm
rm -f lib/Pod/Simple/DumpAsXML.pm
rm -f lib/Pod/Simple/HTML.pm
rm -f lib/Pod/Simple/HTMLBatch.pm
rm -f lib/Pod/Simple/HTMLLegacy.pm
rm -f lib/Pod/Simple/LinkSection.pm
rm -f lib/Pod/Simple/Methody.pm
rm -f lib/Pod/Simple/Progress.pm
rm -f lib/Pod/Simple/PullParser.pm
rm -f lib/Pod/Simple/PullParserEndToken.pm
rm -f lib/Pod/Simple/PullParserStartToken.pm
rm -f lib/Pod/Simple/PullParserTextToken.pm
rm -f lib/Pod/Simple/PullParserToken.pm
rm -f lib/Pod/Simple/RTF.pm
rm -f lib/Pod/Simple/Search.pm
rm -f lib/Pod/Simple/SimpleTree.pm
rm -f lib/Pod/Simple/Subclassing.pod
rm -f lib/Pod/Simple/Text.pm
rm -f lib/Pod/Simple/TextContent.pm
rm -f lib/Pod/Simple/TiedOutFH.pm
rm -f lib/Pod/Simple/Transcode.pm
rm -f lib/Pod/Simple/TranscodeDumb.pm
rm -f lib/Pod/Simple/TranscodeSmart.pm
rm -f lib/Pod/Simple/XHTML.pm
rm -f lib/Pod/Simple/XMLOutStream.pm
cd ext/Pod-Functions
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd ext/Pod-Html
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd cpan/Pod-Parser
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd cpan/Pod-Perldoc
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd cpan/Pod-Usage
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Safe.pm
rm -f lib/Search/Dict.pm
rm -f lib/SelfLoader.pm
rm -f lib/Term/ANSIColor.pm
cd cpan/Term-Cap
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Term/Complete.pm
rm -f lib/Term/ReadLine.pm
rm -f lib/Test.pm
cd cpan/Test-Harness
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Test/Builder.pm
rm -f lib/Test/Builder/Formatter.pm
rm -f lib/Test/Builder/IO/Scalar.pm
rm -f lib/Test/Builder/Module.pm
rm -f lib/Test/Builder/Tester.pm
rm -f lib/Test/Builder/Tester/Color.pm
rm -f lib/Test/Builder/TodoDiag.pm
rm -f lib/Test/More.pm
rm -f lib/Test/Simple.pm
rm -f lib/Test/Tester.pm
rm -f lib/Test/Tester/Capture.pm
rm -f lib/Test/Tester/CaptureRunner.pm
rm -f lib/Test/Tester/Delegate.pm
rm -f lib/Test/Tutorial.pod
rm -f lib/Test/use/ok.pm
rm -f lib/Test2.pm
rm -f lib/Test2/API.pm
rm -f lib/Test2/API/Breakage.pm
rm -f lib/Test2/API/Context.pm
rm -f lib/Test2/API/Instance.pm
rm -f lib/Test2/API/Stack.pm
rm -f lib/Test2/Event.pm
rm -f lib/Test2/Event/Bail.pm
rm -f lib/Test2/Event/Diag.pm
rm -f lib/Test2/Event/Encoding.pm
rm -f lib/Test2/Event/Exception.pm
rm -f lib/Test2/Event/Fail.pm
rm -f lib/Test2/Event/Generic.pm
rm -f lib/Test2/Event/Note.pm
rm -f lib/Test2/Event/Ok.pm
rm -f lib/Test2/Event/Pass.pm
rm -f lib/Test2/Event/Plan.pm
rm -f lib/Test2/Event/Skip.pm
rm -f lib/Test2/Event/Subtest.pm
rm -f lib/Test2/Event/TAP/Version.pm
rm -f lib/Test2/Event/V2.pm
rm -f lib/Test2/Event/Waiting.pm
rm -f lib/Test2/EventFacet.pm
rm -f lib/Test2/EventFacet/About.pm
rm -f lib/Test2/EventFacet/Amnesty.pm
rm -f lib/Test2/EventFacet/Assert.pm
rm -f lib/Test2/EventFacet/Control.pm
rm -f lib/Test2/EventFacet/Error.pm
rm -f lib/Test2/EventFacet/Hub.pm
rm -f lib/Test2/EventFacet/Info.pm
rm -f lib/Test2/EventFacet/Meta.pm
rm -f lib/Test2/EventFacet/Parent.pm
rm -f lib/Test2/EventFacet/Plan.pm
rm -f lib/Test2/EventFacet/Render.pm
rm -f lib/Test2/EventFacet/Trace.pm
rm -f lib/Test2/Formatter.pm
rm -f lib/Test2/Formatter/TAP.pm
rm -f lib/Test2/Hub.pm
rm -f lib/Test2/Hub/Interceptor.pm
rm -f lib/Test2/Hub/Interceptor/Terminator.pm
rm -f lib/Test2/Hub/Subtest.pm
rm -f lib/Test2/IPC.pm
rm -f lib/Test2/IPC/Driver.pm
rm -f lib/Test2/IPC/Driver/Files.pm
rm -f lib/Test2/Tools/Tiny.pm
rm -f lib/Test2/Transition.pod
rm -f lib/Test2/Util.pm
rm -f lib/Test2/Util/ExternalMeta.pm
rm -f lib/Test2/Util/Facets2Legacy.pm
rm -f lib/Test2/Util/HashBase.pm
rm -f lib/Test2/Util/Trace.pm
rm -f lib/ok.pm
rm -f lib/Text/Abbrev.pm
rm -f lib/Text/Balanced.pm
rm -f lib/Text/ParseWords.pm
rm -f lib/Text/Tabs.pm
rm -f lib/Text/Wrap.pm
rm -f lib/Thread/Queue.pm
rm -f lib/Thread/Semaphore.pm
rm -f lib/Tie/File.pm
rm -f lib/Tie/Memoize.pm
rm -f lib/Tie/RefHash.pm
rm -f lib/Time/Local.pm
cd dist/XSLoader
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Fatal.pm
rm -f lib/autodie.pm
rm -f lib/autodie/Scope/Guard.pm
rm -f lib/autodie/Scope/GuardStack.pm
rm -f lib/autodie/Util.pm
rm -f lib/autodie/exception.pm
rm -f lib/autodie/exception/system.pm
rm -f lib/autodie/hints.pm
rm -f lib/autodie/skip.pm
rm -f lib/autouse.pm
cd dist/base
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/Math/BigFloat/Trace.pm
rm -f lib/Math/BigInt/Trace.pm
rm -f lib/bigint.pm
rm -f lib/bignum.pm
rm -f lib/bigrat.pm
rm -f lib/constant.pm
rm -f lib/encoding/warnings.pm
rm -f lib/experimental.pm
cd dist/if
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd cpan/libnet
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/parent.pm
rm -f lib/perlfaq.pm
rm -f lib/perlfaq.pod
rm -f lib/perlfaq1.pod
rm -f lib/perlfaq2.pod
rm -f lib/perlfaq3.pod
rm -f lib/perlfaq4.pod
rm -f lib/perlfaq5.pod
rm -f lib/perlfaq6.pod
rm -f lib/perlfaq7.pod
rm -f lib/perlfaq8.pod
rm -f lib/perlfaq9.pod
rm -f lib/perlglossary.pod
cd cpan/podlators
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
cd dist/Unicode-Normalize
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so
fi
cd ../..
rm -f lib/version.pm
rm -f lib/version.pod
rm -f lib/version/Internals.pod
rm -f lib/version/regex.pm
cd ext/DynaLoader
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=static
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=static
fi
cd ../..
cd ext/B
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Compress-Raw-Bzip2
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Compress-Raw-Zlib
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd dist/PathTools
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/DB_File
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd dist/Data-Dumper
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd dist/Devel-PPPort
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/Devel-Peek
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Digest-MD5
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Digest-SHA
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Encode
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/Fcntl
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/File-DosGlob
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/File-Glob
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Filter-Util-Call
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/GDBM_File
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/Hash-Util
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/Hash-Util-FieldHash
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/I18N-Langinfo
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd dist/IO
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/IPC-SysV
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Scalar-List-Utils
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/MIME-Base64
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Math-BigInt-FastCalc
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/Opcode
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/POSIX
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/PerlIO-encoding
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/PerlIO-mmap
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/PerlIO-scalar
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/PerlIO-via
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/SDBM_File
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Socket
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd dist/Storable
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/Sys-Hostname
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Sys-Syslog
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/Tie-Hash-NamedCapture
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd dist/Time-HiRes
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Time-Piece
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd cpan/Unicode-Collate
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/XS-APItest
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/XS-Typemap
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/arybase
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/attributes
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/mro
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/re
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd dist/threads
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd dist/threads-shared
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1 LIBPERL_A=libperl.so LINKTYPE=dynamic
fi
cd ../..
cd ext/DynaLoader
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/B
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Compress-Raw-Bzip2
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Compress-Raw-Zlib
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/PathTools
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/DB_File
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/Data-Dumper
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/Devel-PPPort
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/Devel-Peek
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Digest-MD5
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Digest-SHA
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Encode
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/Fcntl
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/File-DosGlob
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/File-Glob
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Filter-Util-Call
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/GDBM_File
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/Hash-Util
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/Hash-Util-FieldHash
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/I18N-Langinfo
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/IO
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/IPC-SysV
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Scalar-List-Utils
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/MIME-Base64
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Math-BigInt-FastCalc
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/Opcode
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/POSIX
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/PerlIO-encoding
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/PerlIO-mmap
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/PerlIO-scalar
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/PerlIO-via
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/SDBM_File
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Socket
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/Storable
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/Sys-Hostname
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Sys-Syslog
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/Tie-Hash-NamedCapture
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/Time-HiRes
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Time-Piece
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/Unicode-Collate
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/XS-APItest
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/XS-Typemap
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/arybase
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/attributes
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/mro
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/re
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/threads
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/threads-shared
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/CPAN-Meta
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd ext/Errno
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/ExtUtils-MakeMaker
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/ExtUtils-Manifest
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd dist/ExtUtils-ParseXS
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
cd cpan/HTTP-Tiny
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old realclean MAKE='make' PERL_CORE=1
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    make realclean MAKE='make' PERL_CORE=1
fi
cd ../..
