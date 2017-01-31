## Process this file with automake to produce Makefile.in

sysconf_DATA = chilli.conf 

chillidir = $(sysconfdir)/chilli
chilli_SCRIPTS = functions up.sh down.sh newmulti.sh wpad.dat
chilli_DATA = defaults gui-config-default.ini

pythondir = $(libdir)/python
python_SCRIPTS = CoovaChilliLib.py

initdir = $(sysconfdir)/init.d
init_SCRIPTS = chilli

EXTRA_DIST = $(init_SCRIPTS:=.in) \
$(chilli_SCRIPTS:=.in) chilli.conf.in defaults.in \
CoovaChilliLib.py.in gui-config-default.ini 

edit = sed \
	-e 's,@ETCDIR\@,$(sysconfdir),g' \
	-e 's,@SBINDIR\@,$(sbindir),g' \
	-e 's,@INITDIR\@,$(initrddir),g' \
	-e 's,@VARTMP\@,$(localstatedir)/tmp,g' \
	-e 's,@VARRUN\@,$(localstatedir)/run,g' \
	-e 's,@ETCCHILLI\@,$(sysconfdir)/chilli,g' \
	-e 's,@SYSCONFDIR\@,$(sysconfdir),g' \
	-e 's,@PREFIX\@,$(prefix),g'

CoovaChilliLib.py: Makefile CoovaChilliLib.py.in
	rm -f CoovaChilliLib.py CoovaChilliLib.py.tmp
	$(edit) CoovaChilliLib.py.in >CoovaChilliLib.py.tmp
	mv CoovaChilliLib.py.tmp CoovaChilliLib.py

functions: Makefile functions.in
	rm -f functions functions.tmp
	$(edit) functions.in >functions.tmp
	mv functions.tmp functions

up.sh: Makefile up.sh.in
	rm -f up.sh up.sh.tmp
	$(edit) up.sh.in >up.sh.tmp
	chmod +x up.sh.tmp
	mv up.sh.tmp up.sh

down.sh: Makefile down.sh.in
	rm -f down.sh down.sh.tmp
	$(edit) down.sh.in >down.sh.tmp
	chmod +x down.sh.tmp
	mv down.sh.tmp down.sh

newmulti.sh: Makefile newmulti.sh.in
	rm -f newmulti.sh newmulti.sh.tmp
	$(edit) newmulti.sh.in >newmulti.sh.tmp
	chmod +x newmulti.sh.tmp
	mv newmulti.sh.tmp newmulti.sh

chilli: Makefile chilli.in
	rm -f chilli chilli.tmp
	$(edit) chilli.in >chilli.tmp
	chmod +x chilli.tmp
	mv chilli.tmp chilli

chilli.conf: Makefile chilli.conf.in
	rm -f chilli.conf chilli.conf.tmp
	$(edit) chilli.conf.in >chilli.conf.tmp
	mv chilli.conf.tmp chilli.conf

defaults: Makefile defaults.in
	rm -f defaults defaults.tmp
	$(edit) defaults.in >defaults.tmp
	mv defaults.tmp defaults

wpad.dat: Makefile wpad.dat.in
	rm -f wpad.dat wpad.dat.tmp
	$(edit) wpad.dat.in >wpad.dat.tmp
	mv wpad.dat.tmp wpad.dat
	chmod a+x wpad.dat

