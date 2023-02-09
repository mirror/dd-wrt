# NMake Makefile snippet for copying the built libraries, utilities and headers to
# a path under $(PREFIX).

install: all libpsl.pc build-info
	@if not exist $(PREFIX)\bin\ mkdir $(PREFIX)\bin
	@if not exist $(PREFIX)\lib\pkgconfig\ mkdir $(PREFIX)\lib\pkgconfig
	@if not exist $(PREFIX)\include @mkdir $(PREFIX)\include
	@copy /b vs$(VSVER)\$(CFG)\$(PLAT)\psl.exe $(PREFIX)\bin
	@copy /b vs$(VSVER)\$(CFG)\$(PLAT)\psl-tool.pdb $(PREFIX)\bin
	@if "$(PSL_LIBTYPE)" == "DLL" copy /b $(PSL_DLL) $(PREFIX)\bin
	@if "$(PSL_LIBTYPE)" == "DLL" copy /b vs$(VSVER)\$(CFG)\$(PLAT)\psl.pdb $(PREFIX)\bin
	@copy /b vs$(VSVER)\$(CFG)\$(PLAT)\psl.lib $(PREFIX)\lib
	@copy libpsl.pc $(PREFIX)\lib\pkgconfig
	@for %h in (..\include\*.h) do @copy %h $(PREFIX)\include\%~nxh
