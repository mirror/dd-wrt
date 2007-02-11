top_srcdir	:= ../../../..

CC	:= gcc
CFLAGS	:= -g -DNETSNMP_TCPCONN_TEST \
	-I$(top_srcdir)/agent/mibgroup/ip-mib/data_access \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/agent/mibgroup

OBJS = tcpConn_common.o tcpConn_linux.o

NETSNMPDIR=$(top_srcdir)
NETSNMPCONFIG=$(NETSNMPDIR)/net-snmp-config
NETSNMPBASELIBS := $(shell $(NETSNMPCONFIG) --base-agent-libs)
NETSNMPEXTLIBS := $(shell $(NETSNMPCONFIG) --external-agent-libs)
NETSNMPLIBDIRS := $(shell $(NETSNMPCONFIG) --build-lib-dirs $(NETSNMPDIR))
NETSNMPLIBDEPS := $(shell $(NETSNMPCONFIG) --build-lib-deps $(NETSNMPDIR))
LIB_DEPS=$(NETSNMPLIBDEPS)
LIBS=$(NETSNMPLIBDIRS) -Wl,-Bstatic $(NETSNMPBASELIBS) -Wl,-Bdynamic $(NETSNMPEXTLIBS)

TARGET	:= tcpConn_test

test: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

clean:
	rm $(OBJS) $(TARGET)
