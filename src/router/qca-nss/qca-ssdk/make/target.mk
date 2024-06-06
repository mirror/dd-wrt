
include $(PRJ_PATH)/make/$(OS)_opt.mk

include $(PRJ_PATH)/make/tools.mk

####################################################################
# 			LNX Modules-Style Makefile
####################################################################
src_list_loop: src_list
	$(foreach i, $(SUB_DIR), $(MAKE) -C $(i) src_list_loop || exit 1;)

src_list:
	echo "$(LOC_SRC_FILE) " >> $(PRJ_PATH)/src_list.dep
	awk 'NF' $(PRJ_PATH)/src_list.dep > $(PRJ_PATH)/src_list.dep2
	rm -rf $(PRJ_PATH)/src_list.dep; mv $(PRJ_PATH)/src_list.dep2 $(PRJ_PATH)/src_list.dep

####################################################################
# 			SSDK-Style Makefile
####################################################################
obj: $(OBJ_LIST)
	$(OBJ_LOOP)

dep: build_dir $(DEP_LIST)
	$(DEP_LOOP)

$(OBJ_LIST): %.o : %.c %.d
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $< -o $(DST_DIR)/$@

$(DEP_LIST) : %.d : %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -MM $< > $(DST_DIR)/$@.tmp
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $(DST_DIR)/$@.tmp > $(DST_DIR)/$@
	$(RM) -f $(DST_DIR)/$@.tmp;

build_dir: $(DST_DIR)

$(DST_DIR):
	$(MKDIR) -p $(DST_DIR)

.PHONY: clean
clean: clean_o clean_d
	$(CLEAN_LOOP)

.PHONY: clean_o
clean_o: clean_obj
	$(CLEAN_OBJ_LOOP)

.PHONY: clean_d
clean_d: clean_dep
	$(CLEAN_DEP_LOOP)

clean_obj:
ifneq (,$(word 1, $(OBJ_FILE)))
	$(RM) -f $(OBJ_FILE)
endif

clean_dep:
ifneq (,$(word 1, $(DEP_FILE)))
	$(RM) -f $(DEP_FILE)
endif

ifneq (,$(word 1, $(DEP_FILE)))
  sinclude $(DEP_FILE)
endif	
