# NMake Makefile portion for code generation and
# intermediate build directory creation
# Items in here should not need to be edited unless
# one is maintaining the NMake build files.

# Create the build directories
vs$(VSVER)\$(CFG)\$(PLAT)\libpsl	\
vs$(VSVER)\$(CFG)\$(PLAT)\psl	\
vs$(VSVER)\$(CFG)\$(PLAT)\tests:
	@-md $@

vs$(VSVER)\$(CFG)\$(PLAT)\libpsl\suffixes_dafsa.h: vs$(VSVER)\$(CFG)\$(PLAT)\libpsl $(PSL_FILE) ..\src\psl-make-dafsa
	@echo Generating $@
	$(PYTHON) ..\src\psl-make-dafsa --output-format=cxx+ "$(PSL_FILE_INPUT)" $@

vs$(VSVER)\$(CFG)\$(PLAT)\psl.dafsa: vs$(VSVER)\$(CFG)\$(PLAT)\tests
	@echo Generating $@
	$(PYTHON) ..\src\psl-make-dafsa --output-format=binary "$(PSL_FILE_INPUT)" $@

vs$(VSVER)\$(CFG)\$(PLAT)\psl_ascii.dafsa: vs$(VSVER)\$(CFG)\$(PLAT)\tests
	@echo Generating $@
	$(PYTHON) ..\src\psl-make-dafsa --output-format=binary --encoding=ascii "$(PSL_FILE_INPUT)" $@

libpsl.pc: ..\libpsl.pc.in
	@echo Generating $@
	$(PYTHON) libpsl-pc.py --name=$(PACKAGE_NAME)	\
	--version=$(PACKAGE_VERSION) --url=$(PACKAGE_URL) --prefix=$(PREFIX)

..\config.h: config.h.win32
	@echo Generating $@
	@copy $** $@
