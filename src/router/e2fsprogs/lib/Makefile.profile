all:: profiled $(LIBRARY)_p.a

subdirs:: profiled

profiled:
	@echo "	MKDIR $@"
	@mkdir profiled

clean::
	$(RM) -rf profiled
	$(RM) -f $(LIBRARY)_p.a ../$(LIBRARY)_p.a

$(LIBRARY)_p.a: $(OBJS)
	@echo "	GEN_PROFILED_LIB $(ELF_LIB)"
	@(if test -r $@; then $(RM) -f $@.bak && $(MV) $@ $@.bak; fi)
	@(cd profiled; $(ARUPD) ../$@ $(OBJS))
	-@$(RANLIB) $@
	@$(RM) -f ../$@
	@$(LN) $@ ../$@

install:: $(LIBRARY)_p.a installdirs
	@echo "	INSTALL_DATA $(libdir)/$(LIBRARY)_p.a"
	@$(INSTALL_DATA) $(LIBRARY)_p.a $(DESTDIR)$(libdir)/$(LIBRARY)_p.a
	@-$(RANLIB) $(DESTDIR)$(libdir)/$(LIBRARY)_p.a
	@$(CHMOD) $(LIBMODE) $(DESTDIR)$(libdir)/$(LIBRARY)_p.a

uninstall::
	$(RM) -f $(DESTDIR)$(libdir)/$(LIBRARY)_p.a
