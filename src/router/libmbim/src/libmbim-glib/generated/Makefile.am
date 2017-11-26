noinst_LTLIBRARIES = libmbim-glib-generated.la

GENERATED_H = \
	mbim-error-types.h \
	mbim-enum-types.h \
	mbim-basic-connect.h \
	mbim-sms.h \
	mbim-ussd.h \
	mbim-auth.h \
	mbim-phonebook.h \
	mbim-stk.h \
	mbim-dss.h \
	mbim-ms-firmware-id.h \
	mbim-ms-host-shutdown.h \
	mbim-proxy-control.h \
	mbim-qmi.h \
	mbim-atds.h

GENERATED_C = \
	mbim-error-types.c \
	mbim-error-quarks.c \
	mbim-enum-types.c \
	mbim-basic-connect.c \
	mbim-sms.c \
	mbim-ussd.c \
	mbim-auth.c \
	mbim-phonebook.c \
	mbim-stk.c \
	mbim-dss.c \
	mbim-ms-firmware-id.c \
	mbim-ms-host-shutdown.c \
	mbim-proxy-control.c \
	mbim-qmi.c \
	mbim-atds.c

GENERATED_SECTIONS = \
	mbim-basic-connect.sections \
	mbim-sms.sections \
	mbim-ussd.sections \
	mbim-auth.sections \
	mbim-phonebook.sections \
	mbim-stk.sections \
	mbim-dss.sections \
	mbim-ms-firmware-id.sections \
	mbim-ms-host-shutdown.sections \
	mbim-proxy-control.sections \
	mbim-qmi.sections \
	mbim-atds.sections

# Error types
mbim-error-types.h: $(top_srcdir)/src/libmbim-glib/mbim-errors.h $(top_srcdir)/build-aux/templates/mbim-error-types-template.h
	$(AM_V_GEN) $(GLIB_MKENUMS) \
		--fhead "#ifndef __LIBMBIM_GLIB_ERROR_TYPES_H__\n#define __LIBMBIM_GLIB_ERROR_TYPES_H__\n#include \"mbim-errors.h\"\n" \
		--template $(top_srcdir)/build-aux/templates/mbim-error-types-template.h \
		--ftail "#endif /* __LIBMBIM_GLIB_ERROR_TYPES_H__ */\n" \
		$(top_srcdir)/src/libmbim-glib/mbim-errors.h > $@

mbim-error-types.c: $(top_srcdir)/src/libmbim-glib/mbim-errors.h mbim-error-types.h $(top_srcdir)/build-aux/templates/mbim-error-types-template.c
	$(AM_V_GEN) $(GLIB_MKENUMS) \
		--fhead "#include \"mbim-errors.h\"\n#include \"mbim-error-types.h\"\n" \
		--template $(top_srcdir)/build-aux/templates/mbim-error-types-template.c \
		$(top_srcdir)/src/libmbim-glib/mbim-errors.h > $@

mbim-error-quarks.c: $(top_srcdir)/src/libmbim-glib/mbim-errors.h mbim-error-types.h $(top_srcdir)/build-aux/templates/mbim-error-quarks-template.c
	$(AM_V_GEN) $(GLIB_MKENUMS) \
		--fhead "#include \"mbim-errors.h\"\n#include \"mbim-error-types.h\"\n" \
		--template $(top_srcdir)/build-aux/templates/mbim-error-quarks-template.c \
		$(top_srcdir)/src/libmbim-glib/mbim-errors.h > $@

# Enum/Flag types
ENUMS = \
	$(top_srcdir)/src/libmbim-glib/mbim-uuid.h \
	$(top_srcdir)/src/libmbim-glib/mbim-cid.h \
	$(top_srcdir)/src/libmbim-glib/mbim-message.h \
	$(top_srcdir)/src/libmbim-glib/mbim-enums.h
mbim-enum-types.h:  $(ENUMS) $(top_srcdir)/build-aux/templates/mbim-enum-types-template.h
	$(AM_V_GEN) $(GLIB_MKENUMS) \
		--fhead "#ifndef __LIBMBIM_GLIB_ENUM_TYPES_H__\n#define __LIBMBIM_GLIB_ENUM_TYPES_H__\n#include \"mbim-uuid.h\"\n#include \"mbim-cid.h\"\n#include \"mbim-message.h\"\n#include \"mbim-enums.h\"\n" \
		--template $(top_srcdir)/build-aux/templates/mbim-enum-types-template.h \
		--ftail "#endif /* __LIBMBIM_GLIB_ENUM_TYPES_H__ */\n" \
		$(ENUMS) > $@

mbim-enum-types.c: $(ENUMS) mbim-enum-types.h $(top_srcdir)/build-aux/templates/mbim-enum-types-template.c
	$(AM_V_GEN) $(GLIB_MKENUMS) \
		--fhead "#include \"mbim-enum-types.h\"\n" \
		--template $(top_srcdir)/build-aux/templates/mbim-enum-types-template.c \
		$(ENUMS) > $@

# Basic Connect service
mbim-basic-connect.h mbim-basic-connect.c mbim-basic-connect.sections: $(top_srcdir)/data/mbim-service-basic-connect.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-basic-connect.h && \
		rm -f mbim-basic-connect.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-basic-connect.json \
			--output mbim-basic-connect

# SMS service
mbim-sms.h mbim-sms.c mbim-sms.sections: $(top_srcdir)/data/mbim-service-sms.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-sms.h && \
		rm -f mbim-sms.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-sms.json \
			--output mbim-sms

# USSD service
mbim-ussd.h mbim-ussd.c mbim-ussd.sections: $(top_srcdir)/data/mbim-service-ussd.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-ussd.h && \
		rm -f mbim-ussd.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-ussd.json \
			--output mbim-ussd

# Auth service
mbim-auth.h mbim-auth.c mbim-auth.sections: $(top_srcdir)/data/mbim-service-auth.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-auth.h && \
		rm -f mbim-auth.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-auth.json \
			--output mbim-auth

# Phonebook service
mbim-phonebook.h mbim-phonebook.c mbim-phonebook.sections: $(top_srcdir)/data/mbim-service-phonebook.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-phonebook.h && \
		rm -f mbim-phonebook.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-phonebook.json \
			--output mbim-phonebook

# STK service
mbim-stk.h mbim-stk.c mbim-stk.sections: $(top_srcdir)/data/mbim-service-stk.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-stk.h && \
		rm -f mbim-stk.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-stk.json \
			--output mbim-stk

# DSS service
mbim-dss.h mbim-dss.c mbim-dss.sections: $(top_srcdir)/data/mbim-service-dss.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-dss.h && \
		rm -f mbim-dss.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-dss.json \
			--output mbim-dss

# Microsoft Firmware ID service
mbim-ms-firmware-id.h mbim-ms-firmware-id.c mbim-ms-firmware-id.sections: $(top_srcdir)/data/mbim-service-ms-firmware-id.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-ms-firmware-id.h && \
		rm -f mbim-ms-firmware-id.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-ms-firmware-id.json \
			--output mbim-ms-firmware-id

# Microsoft Host Shutdown service
mbim-ms-host-shutdown.h mbim-ms-host-shutdown.c mbim-ms-host-shutdown.sections: $(top_srcdir)/data/mbim-service-ms-host-shutdown.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-ms-host-shutdown.h && \
		rm -f mbim-ms-host-shutdown.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-ms-host-shutdown.json \
			--output mbim-ms-host-shutdown

# Proxy Control service
mbim-proxy-control.h mbim-proxy-control.c mbim-proxy-control.sections: $(top_srcdir)/data/mbim-service-proxy-control.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-proxy-control.h && \
		rm -f mbim-proxy-control.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-proxy-control.json \
			--output mbim-proxy-control

# QMI-over-MBIM service
mbim-qmi.h mbim-qmi.c mbim-qmi.sections: $(top_srcdir)/data/mbim-service-qmi.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-qmi.h && \
		rm -f mbim-qmi.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-qmi.json \
			--output mbim-qmi

# ATT Device service
mbim-atds.h mbim-atds.c mbim-atds.sections: $(top_srcdir)/data/mbim-service-atds.json $(top_srcdir)/build-aux/mbim-codegen/*.py $(top_srcdir)/build-aux/mbim-codegen/mbim-codegen
	$(AM_V_GEN)  \
		rm -f mbim-atds.h && \
		rm -f mbim-atds.c && \
		$(top_srcdir)/build-aux/mbim-codegen/mbim-codegen \
			--input $(top_srcdir)/data/mbim-service-atds.json \
			--output mbim-atds

BUILT_SOURCES = $(GENERATED_H) $(GENERATED_C)

nodist_libmbim_glib_generated_la_SOURCES = \
	$(GENERATED_H) \
	$(GENERATED_C)

libmbim_glib_generated_la_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-DLIBMBIM_GLIB_COMPILATION \
	-DG_LOG_DOMAIN=\"Mbim\" \
	-Wno-unused-function

libmbim_glib_generated_la_LIBADD = \
	$(LIBMBIM_GLIB_LIBS)

includedir = @includedir@/libmbim-glib
nodist_include_HEADERS = $(GENERATED_H)

CLEANFILES = $(GENERATED_H) $(GENERATED_C) $(GENERATED_SECTIONS)
