
#CC = gcc
#CFLAGS = -I. -fPIC -O -g -Wall -Werror -Wno-unused-variable -Wno-unused-parameter
CFLAGS += $(COPTS) -DRPCOVERRIDE -I. -Iinclude -Ilibrpc -fPIC -Wall -Wno-unused-variable -Wno-unused-parameter $(LTO)
CFLAGS += -D_GNU_SOURCE

COMMON_PROG_OBJS = \
	call_qcsapi.o	\
	qcsapi_driver.o	\
	qcsapi_output.o	\
	qcsapi_sem.o	\

SOCKET_PROG_OBJS = \
	$(COMMON_PROG_OBJS)				\
	qcsapi_rpc/client/socket/qcsapi_socket_rpc_client.o	\
	qcsapi_rpc_common/client/find_host_addr.o	\

SOCKET_C_SAMPLE_OBJS = \
	qcsapi_rpc_sample/c_rpc_qcsapi_sample.o	\
	qcsapi_rpc_common/client/find_host_addr.o	\

PCIE_PROG_OBJS = \
	$(COMMON_PROG_OBJS)				\
	qcsapi_rpc/client/pcie/qcsapi_pcie_rpc_client.o	\
	qcsapi_rpc_common/client/rpc_pci_clnt.o	\

LIB_OBJS = \
	qcsapi_rpc/generated/qcsapi_rpc_xdr.o	\
	qcsapi_rpc/generated/qcsapi_rpc_clnt_adapter.o	\

#TARGETS = c_rpc_qcsapi_sample qcsapi_sockrpc qcsapi_sockrpc_static qcsapi_pcie qcsapi_pcie_static $(LIB_REALNAME)


#$(LIB_REALNAME)



-include $(shell find . -name \*.d)

LIB_NAME = qcsapi_client
LIB_NAME_AR = qcsapi_client.a
LIB_LDNAME = lib$(LIB_NAME).so
LIB_SONAME = $(LIB_LDNAME).1
LIB_REALNAME = $(LIB_LDNAME).1.0.1

TARGETS = qcsapi_sockrpc $(LIB_NAME_AR)

all: rpc libqcsapi_client.a 
#rpc $(TARGETS)

c_rpc_qcsapi_sample: ${SOCKET_C_SAMPLE_OBJS:%=build/%} $(LIB_REALNAME)
	${CC} $(filter %.o, $^) -L. -l$(LIB_NAME) -o $@

qcsapi_pcie: ${PCIE_PROG_OBJS:%=build/%} $(LIB_REALNAME)
	${CC} $(filter %.o, $^) -L. -l$(LIB_NAME) -o $@

qcsapi_pcie_static: ${PCIE_PROG_OBJS:%=build/%} ${LIB_OBJS}
	${CC} $(filter %.o, $^) -o $@

qcsapi_sockrpc: ${SOCKET_PROG_OBJS:%=build/%} $(LIB_REALNAME)
	${CC} $(filter %.o, $^) -L. -l$(LIB_NAME) librpc/librpc.a -o $@

qcsapi_sockrpc_static: ${SOCKET_PROG_OBJS:%=build/%} ${LIB_OBJS}
	${CC} $(filter %.o, $^) -o $@

$(LIB_REALNAME): ${LIB_OBJS:%=build/%} qcsapi_rpc_common/client/find_host_addr.o
	${CC} -shared -s -o  $@ -Wl,-soname,$(LIB_SONAME) -lc $^ -Llibrpc -lrpc
	cd ${@D} ; ln -fs $(LIB_REALNAME) $(LIB_SONAME)
	cd ${@D} ; ln -fs $(LIB_SONAME) $(LIB_LDNAME)

libqcsapi_client.a: ${LIB_OBJS:%=build/%} qcsapi_rpc_common/client/find_host_addr.o
	$(AR) cr $@ $^

build/%.o: %.c
	@mkdir -p ${@D}
	${CC} ${CFLAGS} $< -c -o $@ -MD -MF $@.d

include _cmake.mk


rpc:
	$(call CMakeClean,librpc)
	$(call CMakeConfigure,librpc) 
	make -C librpc

clean:
	rm -rf build $(LIB_LDNAME)* libqcsapi_client.a $(TARGETS)
	-make -C librpc clean
	-$(call CMakeClean,librpc)
