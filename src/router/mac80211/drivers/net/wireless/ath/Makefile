obj-$(CPTCFG_ATH5K)		+= ath5k/
obj-$(CPTCFG_ATH9K_HW)		+= ath9k/
obj-$(CPTCFG_CARL9170)		+= carl9170/
obj-$(CPTCFG_ATH6KL)		+= ath6kl/
obj-$(CPTCFG_AR5523)		+= ar5523/
obj-$(CPTCFG_WIL6210)		+= wil6210/
obj-$(CPTCFG_ATH10K)		+= ath10k/
obj-$(CPTCFG_WCN36XX)		+= wcn36xx/

obj-$(CPTCFG_ATH_COMMON)	+= ath.o

ath-objs :=	main.o \
		regd.o \
		hw.o \
		key.o \
		debug.o \
		dfs_pattern_detector.o \
		dfs_pri_detector.o

ath-$(CPTCFG_ATH_TRACEPOINTS) += trace.o

CFLAGS_trace.o := -I$(src)
