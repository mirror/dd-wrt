include ../wireformat/Makefile.inc
WIREFORMAT_PROJECT := $(PROJECT)

include Makefile.inc

include ../Makefile.inc
OLSRD_PLUGIN = false

TOPDIR = ../../..
include $(TOPDIR)/Makefile.inc

######################
#
# Settings
#
######################

ifeq ($(DEBUG),1)
DEBUGBOOLEAN = true
else
DEBUGBOOLEAN = false
endif


DESTDIR ?=
USRDIR ?= $(DESTDIR)/usr
LIBDIR ?= $(USRDIR)/lib
SHAREDIR ?= $(USRDIR)/share
SHAREJAVADIR ?= $(SHAREDIR)/java/$(EXENAME)


MACHINE=$(shell uname -m)
ARCH=
ifeq ($(strip $(MACHINE)),x86_64)
  ARCH=.$(MACHINE)
endif


RESOURCES_DIR = resources
BUILD_DIR = build
LIBDIR_BUILD = lib

INC_DIR_WIREFORMAT = ../wireformat/include
INC_DIR_WIREFORMAT_FULL = $(INC_DIR_WIREFORMAT)/$(WIREFORMAT_PROJECT)

INC_DIR_JVM = /usr/lib/jvm/java/include
INC_DIR_JVM_OS = $(INC_DIR_JVM)/$(OS)


SRC_DIR_C = src/main/c

INCLUDES = -I "$(INC_DIR_WIREFORMAT)" -I "$(INC_DIR_JVM)" -I "$(INC_DIR_JVM_OS)"
LIBRARIES = -L ../wireformat/lib -l$(WIREFORMAT_PROJECT)


BUILD_DIR_JNI = $(BUILD_DIR)/jni

JAVASRC = src/main/java
JAVAVERSIONSRC = 1.6
JAVAVERSIONDST = 1.6
JAVABOOTCLASSPATH = resources/ee.j2se/$(JAVAVERSIONSRC)/ee.j2se.jar
JAVA_PKG = org.olsr.plugin.pud
JAVA_PKG_DIR = $(subst .,/,$(JAVA_PKG))
JAVA_PKG_UNDER = $(subst .,_,$(JAVA_PKG))


ifneq ($(OS),linux)

.PHONY: all default_target install clean

all: default_target

default_target install clean:
	@echo "*** only supported on Linux"

else

GIT_SHA = $(shell git describe 2> /dev/null || echo "Unknown")

.PHONY: all default_target java prepare clean doc doc-clean install uninstall

#
# Targets
#

all: default_target

default_target: prepare $(LIBDIR_BUILD)/$(SONAME) java

.gitignore:
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)echo "/$(BUILD_DIR)/" > $@
	$(MAKECMDPREFIX)echo "/$(LIBDIR_BUILD)/" >> $@
	$(MAKECMDPREFIX)echo "/build.xml" >> $@
	$(MAKECMDPREFIX)echo "/$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.h" >> $@
	$(MAKECMDPREFIX)echo "/$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_ClusterLeader.h" >> $@
	$(MAKECMDPREFIX)echo "/$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_UplinkMessage.h" >> $@
	$(MAKECMDPREFIX)echo "/$(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java" >> $@
	$(MAKECMDPREFIX)$(MAKE) -C doc $@

$(LIBDIR_BUILD)/$(SONAME): $(BUILD_DIR)/$(JAVA_PKG_UNDER)_UplinkMessage.o \
                           $(BUILD_DIR)/$(JAVA_PKG_UNDER)_ClusterLeader.o \
                           $(BUILD_DIR)/$(JAVA_PKG_UNDER)_PositionUpdate.o
ifeq ($(VERBOSE),0)
	@echo "[LD] $@"
endif
	$(MAKECMDPREFIX)$(CC) -shared -Wl,-soname=$(SONAME) $(LIBRARIES) -o "$@" \
      $(BUILD_DIR)/$(JAVA_PKG_UNDER)_UplinkMessage.o \
      $(BUILD_DIR)/$(JAVA_PKG_UNDER)_ClusterLeader.o \
      $(BUILD_DIR)/$(JAVA_PKG_UNDER)_PositionUpdate.o

# PositionUpdate
$(BUILD_DIR)/$(JAVA_PKG_UNDER)_PositionUpdate.o: $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.c \
                                                 $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/wireFormat.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/nodeIdConversion.h
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o "$@" "$<"

$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.h: $(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/PositionUpdate.class
ifeq ($(VERBOSE),0)
	@echo "[JavaH] $<"
endif
	$(MAKECMDPREFIX)rm -f "$@"
	$(MAKECMDPREFIX)javah -jni -classpath "$(BUILD_DIR_JNI)" -o "$@" "$(JAVA_PKG).PositionUpdate"

$(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/PositionUpdate.class: $(JAVASRC)/$(JAVA_PKG_DIR)/PositionUpdate.java \
                                                       $(JAVASRC)/$(JAVA_PKG_DIR)/LibraryLoader.java \
                                                       $(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java
ifeq ($(VERBOSE),0)
	@echo "[JavaC] $<"
endif
	$(MAKECMDPREFIX)javac -bootclasspath $(JAVABOOTCLASSPATH) -classpath $(JAVASRC) -source 1.6 -target 1.6 -Xlint -d "$(BUILD_DIR_JNI)" "$<"

# ClusterLeader
$(BUILD_DIR)/$(JAVA_PKG_UNDER)_ClusterLeader.o: $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_ClusterLeader.c \
                                                 $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_ClusterLeader.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/wireFormat.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/nodeIdConversion.h
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o "$@" "$<"

$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_ClusterLeader.h: $(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/ClusterLeader.class
ifeq ($(VERBOSE),0)
	@echo "[JavaH] $<"
endif
	$(MAKECMDPREFIX)rm -f "$@"
	$(MAKECMDPREFIX)javah -jni -classpath "$(BUILD_DIR_JNI)" -o "$@" "$(JAVA_PKG).ClusterLeader"

$(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/ClusterLeader.class: $(JAVASRC)/$(JAVA_PKG_DIR)/ClusterLeader.java \
                                                      $(JAVASRC)/$(JAVA_PKG_DIR)/LibraryLoader.java \
                                                      $(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java
ifeq ($(VERBOSE),0)
	@echo "[JavaC] $<"
endif
	$(MAKECMDPREFIX)javac -bootclasspath $(JAVABOOTCLASSPATH) -classpath $(JAVASRC) -source 1.6 -target 1.6 -Xlint -d "$(BUILD_DIR_JNI)" "$<"

# UplinkMessage
$(BUILD_DIR)/$(JAVA_PKG_UNDER)_UplinkMessage.o: $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_UplinkMessage.c \
                                                 $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_UplinkMessage.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/wireFormat.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/nodeIdConversion.h
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o "$@" "$<"

$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_UplinkMessage.h: $(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/UplinkMessage.class
ifeq ($(VERBOSE),0)
	@echo "[JavaH] $<"
endif
	$(MAKECMDPREFIX)rm -f "$@"
	$(MAKECMDPREFIX)javah -jni -classpath "$(BUILD_DIR_JNI)" -o "$@" "$(JAVA_PKG).UplinkMessage"

$(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/UplinkMessage.class: $(JAVASRC)/$(JAVA_PKG_DIR)/UplinkMessage.java \
                                                      $(JAVASRC)/$(JAVA_PKG_DIR)/LibraryLoader.java \
                                                      $(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java
ifeq ($(VERBOSE),0)
	@echo "[JavaC] $<"
endif
	$(MAKECMDPREFIX)javac -bootclasspath $(JAVABOOTCLASSPATH) -classpath $(JAVASRC) -source 1.6 -target 1.6 -Xlint -d "$(BUILD_DIR_JNI)" "$<"


#
# java
#

java: prepare \
     build.xml \
     $(JAVASRC)/$(JAVA_PKG_DIR)/PositionUpdate.java \
     $(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java
	$(MAKECMDPREFIX)ant -f build.xml release

build.xml: $(RESOURCES_DIR)/build.xml
ifeq ($(VERBOSE),0)
	@echo "[generate] $@"
endif
	$(MAKECMDPREFIX)sed -r -e "s#PLUGIN_VER#$(PLUGIN_VER)#g" \
	        -e "s#PROJECT#$(PROJECT)#g" \
	        -e "s#BUILD_DIR#$(BUILD_DIR)#g" \
	        -e "s#LIBDIR_BUILD#$(LIBDIR_BUILD)#g" \
	        -e "s#JAVASRC#$(JAVASRC)#g" \
	        -e "s#JAVAVERSIONSRC#$(JAVAVERSIONSRC)#g" \
	        -e "s#JAVAVERSIONDST#$(JAVAVERSIONDST)#g" \
	        -e "s#DEBUG#$(DEBUGBOOLEAN)#g" \
	        "$<" > "$@"


#
# generate java classes
#

$(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java: \
      $(RESOURCES_DIR)/WireFormatConstants.java.h \
      $(BUILD_DIR_JNI)/c/library.h \
      $(BUILD_DIR_JNI)/c/wireFormat.h
ifeq ($(VERBOSE),0)
	@echo "[generate] $@"
endif
	$(MAKECMDPREFIX)$(CPP) -I "$(BUILD_DIR_JNI)/c" -DPLUGIN_VER=\"$(PLUGIN_VER)\" -DGIT_SHA=\"$(GIT_SHA)\" "$<" | \
	  sed -r -e '/^[[:space:]]*#.*$$/ d' \
	         -e '/^[[:space:]]*$$/ d' \
	         > "$@"


#
# generate jni includes
#

$(BUILD_DIR_JNI)/c/library.h: Makefile.inc
ifeq ($(VERBOSE),0)
	@echo "[generate] $@"
endif
	$(MAKECMDPREFIX)echo "#define PUD_WIRE_FORMAT_LIBRARYNAME \"$(PROJECT)\"" > "$@"

$(BUILD_DIR_JNI)/c/wireFormat.h: $(INC_DIR_WIREFORMAT_FULL)/wireFormat.h
ifeq ($(VERBOSE),0)
	@echo "[generate] $@"
endif
	$(MAKECMDPREFIX)grep -E '^[[:space:]]*#define[[:space:]]+' "$<" > "$@"


#
# Phony Targets
#

prepare:
	$(MAKECMDPREFIX)mkdir -p "$(BUILD_DIR)" \
	          "$(LIBDIR_BUILD)" \
	          "$(BUILD_DIR_JNI)/c"


clean: doc-clean
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)rm -fr "$(BUILD_DIR)" "$(LIBDIR_BUILD)" \
	        build.xml \
	        "$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.h" \
	        "$(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java"

doc: build.xml
	$(MAKECMDPREFIX)$(MAKE) -C doc all
	$(MAKECMDPREFIX)ant -f build.xml javadoc

doc-clean:
	$(MAKECMDPREFIX)$(MAKE) -C doc clean

install: all
	mkdir -v -p "$(LIBDIR)"
	cp "$(LIBDIR_BUILD)/$(SONAME)" "$(LIBDIR)/$(SONAME).$(PLUGIN_VER)"
	$(STRIP) "$(LIBDIR)/$(SONAME).$(PLUGIN_VER)"
	/sbin/ldconfig -n "$(LIBDIR)"
	mkdir -v -p "$(SHAREJAVADIR)"
	cp -t "$(SHAREJAVADIR)" \
      "$(LIBDIR_BUILD)/$(PROJECT).jar" \
      "$(LIBDIR_BUILD)/$(PROJECT)-src.zip" \
      "$(LIBDIR_BUILD)/$(PROJECT)-doc.zip"

uninstall:
	rm -f "$(LIBDIR)/$(SONAME)" \
	      "$(LIBDIR)/$(SONAME).$(PLUGIN_VER)"
	/sbin/ldconfig -n "$(LIBDIR)"
	rmdir -v -p --ignore-fail-on-non-empty "$(LIBDIR)"
	rm -f "$(SHAREJAVADIR)/$(PROJECT).jar" \
          "$(SHAREJAVADIR)/$(PROJECT)-src.zip" \
          "$(SHAREJAVADIR)/$(PROJECT)-doc.zip"
	rmdir -v -p --ignore-fail-on-non-empty "$(SHAREJAVADIR)"

endif
