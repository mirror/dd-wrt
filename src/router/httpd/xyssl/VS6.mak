
CFLAGS=/O2 /W3 /MT /nologo /Fo$@ /DWIN32 /DSELF_TEST #/DHAVE_SSE2
LDFLAGS=xyssl.lib kernel32.lib shell32.lib user32.lib

LIB_OBJ=library/aes.obj           library/arc4.obj          \
        library/base64.obj        library/bignum.obj        \
        library/certs.obj         library/dhm.obj           \
        library/des.obj           library/havege.obj        \
        library/md2.obj           library/md4.obj           \
        library/md5.obj           library/net.obj           \
        library/rsa.obj           library/sha1.obj          \
        library/sha2.obj          library/ssl_cli.obj       \
        library/ssl_srv.obj       library/ssl_tls.obj       \
        library/timing.obj        library/x509_read.obj

PRG_OBJ=programs/benchmark.exe    programs/hello.exe        \
        programs/filecrypt.exe    programs/rsa_demo.exe     \
        programs/selftest.exe     programs/ssl_client1.exe  \
        programs/ssl_client2.exe  programs/ssl_server.exe

default: lib prg

lib:  $(LIB_OBJ) ; @echo.
	@lib /out:xyssl.lib $(LIB_OBJ)
	@del library\*.obj

prg:  $(PRG_OBJ) ; @echo.
	@del programs\*.exe

.c.obj: ; @$(CC) $(CFLAGS) /I"include/xyssl/"  /c $<

.c.exe: ; @$(CC) $(CFLAGS) /I"include" $(LDFLAGS) $<
