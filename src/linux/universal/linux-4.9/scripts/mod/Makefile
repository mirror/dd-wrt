OBJECT_FILES_NON_STANDARD := y

hostprogs-y	:= modpost mk_elfconfig
always		:= $(hostprogs-y) empty.o

modpost-objs	:= modpost.o file2alias.o sumversion.o

devicetable-offsets-file := devicetable-offsets.h

$(obj)/devicetable-offsets.s: KBUILD_CFLAGS += $(DISABLE_LTO)
$(obj)/empty.c: KBUILD_CFLAGS += $(DISABLE_LTO)

$(obj)/$(devicetable-offsets-file): $(obj)/devicetable-offsets.s FORCE
	$(call filechk,offsets,__DEVICETABLE_OFFSETS_H__)

targets += $(devicetable-offsets-file) devicetable-offsets.s

# dependencies on generated files need to be listed explicitly

$(obj)/modpost.o $(obj)/file2alias.o $(obj)/sumversion.o: $(obj)/elfconfig.h
$(obj)/file2alias.o: $(obj)/$(devicetable-offsets-file)

quiet_cmd_elfconfig = MKELF   $@
      cmd_elfconfig = $(obj)/mk_elfconfig < $< > $@

$(obj)/elfconfig.h: $(obj)/empty.o $(obj)/mk_elfconfig FORCE
	$(call if_changed,elfconfig)

targets += elfconfig.h
