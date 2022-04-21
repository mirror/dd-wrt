CFLAGS_trace.o := -I$(src)


$(obj)/shipped-certs.c: $(wildcard $(src)/certs/*.hex)
	@$(kecho) "  GEN     $@"
	@(echo '#include "reg.h"'; \
	  echo 'const u8 shipped_regdb_certs[] = {'; \
	  echo | cat - $^ ; \
	  echo '};'; \
	  echo 'unsigned int shipped_regdb_certs_len = sizeof(shipped_regdb_certs);'; \
	 ) > $@

$(obj)/extra-certs.c: $(CPTCFG_CFG80211_EXTRA_REGDB_KEYDIR:"%"=%) \
		      $(wildcard $(CPTCFG_CFG80211_EXTRA_REGDB_KEYDIR:"%"=%)/*.x509)
	@$(kecho) "  GEN     $@"
	@(set -e; \
	  allf=""; \
	  for f in $^ ; do \
	      test -f $$f || continue;\
	      # similar to hexdump -v -e '1/1 "0x%.2x," "\n"' \
	      thisf=$$(od -An -v -tx1 < $$f | \
	                   sed -e 's/ /\n/g' | \
	                   sed -e 's/^[0-9a-f]\+$$/\0/;t;d' | \
	                   sed -e 's/^/0x/;s/$$/,/'); \
	      # file should not be empty - maybe command substitution failed? \
	      test ! -z "$$thisf";\
	      allf=$$allf$$thisf;\
	  done; \
	  ( \
	      echo '#include "reg.h"'; \
	      echo 'const u8 extra_regdb_certs[] = {'; \
	      echo "$$allf"; \
	      echo '};'; \
	      echo 'unsigned int extra_regdb_certs_len = sizeof(extra_regdb_certs);'; \
	  ) > $@)

clean-files += shipped-certs.c extra-certs.c
