ATM_OBJS=text2atm.o atm2text.o atmequal.o sdu2cell.o text2qos.o qos2text.o \
  qosequal.o sap2text.o text2sap.o sapequal.o misc.o
ifeq ($(LIB_PATH)/libresolv.a,$(wildcard $(LIB_PATH)/libresolv.a))
ATM_OBJS += ans_l.o
else
ATM_OBJS += ans.o
endif
ATMD_OBJS=common.o diag.o kptr.o text2ip.o timer.o unix.o
PGMS=#test
GENLIBS=libatm.a libatmd.a
SYSHDR=atm.h atmd.h atmsap.h
OPTSYSHDR=stdint.h

do_all:			libatm.a libatmd.a

include ../Rules.make

../lib/libatm.a:	libatm.a

libatm.a:		$(ATM_OBJS)
			$(AR) rcs libatm.a $(ATM_OBJS)

../lib/libatmd.a:	libatmd.a

libatmd.a:		$(ATMD_OBJS)
			$(AR) rcs libatmd.a $(ATMD_OBJS)

ans_l.o:		ans.o
			$(LD) -r -o ans_l.o ans.o -L$(LIB_PATH) -lresolv
clean:
			rm -f *.a *.o
