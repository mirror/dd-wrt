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

MACHINE=$(shell uname -m)
LIBDIR_INSTALL = usr/lib
ARCH=
ifeq ($(strip $(MACHINE)),x86_64)
  ARCH=.$(MACHINE)
endif


RESOURCES_DIR = resources

BUILD_DIR = build

INC_DIR_WIREFORMAT = ../wireformat/include
INC_DIR_WIREFORMAT_FULL = $(INC_DIR_WIREFORMAT)/$(WIREFORMAT_PROJECT)

INC_DIR_JVM = /usr/lib/jvm/java-1.6.0-openjdk$(ARCH)/include
INC_DIR_JVM_OS = $(INC_DIR_JVM)/$(OS)

LIBDIR_BUILD = lib

SRC_DIR_C = src/main/c

INCLUDES = -I "$(INC_DIR_WIREFORMAT)" -I "$(INC_DIR_JVM)" -I "$(INC_DIR_JVM_OS)"
LIBRARIES = -L ../wireformat/lib -l$(WIREFORMAT_PROJECT)

DESTDIR ?=
JAVA_DIR_INSTALL = usr/share/java/olsrd

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
	@echo "[$@]"
	@echo "/$(BUILD_DIR)/" > $@
	@echo "/$(LIBDIR_BUILD)/" >> $@
	@echo "/build.xml" >> $@
	@echo "/$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.h" >> $@
	@echo "/$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_ClusterLeader.h" >> $@
	@echo "/$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_UplinkMessage.h" >> $@
	@echo "/$(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java" >> $@
	@$(MAKE) -C doc $@

$(LIBDIR_BUILD)/$(SONAME): $(BUILD_DIR)/$(JAVA_PKG_UNDER)_UplinkMessage.o \
                           $(BUILD_DIR)/$(JAVA_PKG_UNDER)_ClusterLeader.o \
                           $(BUILD_DIR)/$(JAVA_PKG_UNDER)_PositionUpdate.o
	@echo "[LD] $@"
	@$(CC) -shared -Wl,-soname=$(SONAME) $(LIBRARIES) -o "$@" \
      $(BUILD_DIR)/$(JAVA_PKG_UNDER)_UplinkMessage.o \
      $(BUILD_DIR)/$(JAVA_PKG_UNDER)_ClusterLeader.o \
      $(BUILD_DIR)/$(JAVA_PKG_UNDER)_PositionUpdate.o

# PositionUpdate
$(BUILD_DIR)/$(JAVA_PKG_UNDER)_PositionUpdate.o: $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.c \
                                                 $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/wireFormat.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/nodeIdConversion.h
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o "$@" "$<"

$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.h: $(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/PositionUpdate.class
	@echo "[JavaH] $<"
	@rm -f "$@"
	@javah -jni -classpath "$(BUILD_DIR_JNI)" -o "$@" "$(JAVA_PKG).PositionUpdate"

$(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/PositionUpdate.class: $(JAVASRC)/$(JAVA_PKG_DIR)/PositionUpdate.java \
                                                       $(JAVASRC)/$(JAVA_PKG_DIR)/LibraryLoader.java \
                                                       $(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java
	@echo "[JavaC] $<"
	@javac -bootclasspath $(JAVABOOTCLASSPATH) -classpath $(JAVASRC) -source 1.6 -target 1.6 -Xlint -d "$(BUILD_DIR_JNI)" "$<"

# ClusterLeader
$(BUILD_DIR)/$(JAVA_PKG_UNDER)_ClusterLeader.o: $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_ClusterLeader.c \
                                                 $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_ClusterLeader.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/wireFormat.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/nodeIdConversion.h
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o "$@" "$<"

$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_ClusterLeader.h: $(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/ClusterLeader.class
	@echo "[JavaH] $<"
	@rm -f "$@"
	@javah -jni -classpath "$(BUILD_DIR_JNI)" -o "$@" "$(JAVA_PKG).ClusterLeader"

$(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/ClusterLeader.class: $(JAVASRC)/$(JAVA_PKG_DIR)/ClusterLeader.java \
                                                      $(JAVASRC)/$(JAVA_PKG_DIR)/LibraryLoader.java \
                                                      $(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java
	@echo "[JavaC] $<"
	@javac -bootclasspath $(JAVABOOTCLASSPATH) -classpath $(JAVASRC) -source 1.6 -target 1.6 -Xlint -d "$(BUILD_DIR_JNI)" "$<"

# UplinkMessage
$(BUILD_DIR)/$(JAVA_PKG_UNDER)_UplinkMessage.o: $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_UplinkMessage.c \
                                                 $(SRC_DIR_C)/$(JAVA_PKG_UNDER)_UplinkMessage.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/wireFormat.h \
                                                 $(INC_DIR_WIREFORMAT_FULL)/nodeIdConversion.h
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o "$@" "$<"

$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_UplinkMessage.h: $(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/UplinkMessage.class
	@echo "[JavaH] $<"
	@rm -f "$@"
	@javah -jni -classpath "$(BUILD_DIR_JNI)" -o "$@" "$(JAVA_PKG).UplinkMessage"

$(BUILD_DIR_JNI)/$(JAVA_PKG_DIR)/UplinkMessage.class: $(JAVASRC)/$(JAVA_PKG_DIR)/UplinkMessage.java \
                                                      $(JAVASRC)/$(JAVA_PKG_DIR)/LibraryLoader.java \
                                                      $(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java
	@echo "[JavaC] $<"
	@javac -bootclasspath $(JAVABOOTCLASSPATH) -classpath $(JAVASRC) -source 1.6 -target 1.6 -Xlint -d "$(BUILD_DIR_JNI)" "$<"


#
# java
#

java: prepare \
     build.xml \
     $(JAVASRC)/$(JAVA_PKG_DIR)/PositionUpdate.java \
     $(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java
	@ant -f build.xml release

build.xml: $(RESOURCES_DIR)/build.xml
	@echo "[generate] $@"
	@sed -r -e "s#PLUGIN_VER#$(PLUGIN_VER)#g" \
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
	@echo "[generate] $@"
	@$(CPP) -I "$(BUILD_DIR_JNI)/c" -DPLUGIN_VER=\"$(PLUGIN_VER)\" -DGIT_SHA=\"$(GIT_SHA)\" "$<" | \
	  sed -r -e '/^[[:space:]]*#.*$$/ d' \
	         -e '/^[[:space:]]*$$/ d' \
	         > "$@"


#
# generate jni includes
#

$(BUILD_DIR_JNI)/c/library.h: Makefile.inc
	@echo "[generate] $@"
	@echo "#define PUD_WIRE_FORMAT_LIBRARYNAME \"$(PROJECT)\"" > "$@"

$(BUILD_DIR_JNI)/c/wireFormat.h: $(INC_DIR_WIREFORMAT_FULL)/wireFormat.h
	@echo "[generate] $@"
	@grep -E '^[[:space:]]*#define[[:space:]]+' "$<" > "$@"


#
# Phony Targets
#

prepare:
	@mkdir -p "$(BUILD_DIR)" \
	          "$(LIBDIR_BUILD)" \
	          "$(BUILD_DIR_JNI)/c"


clean: doc-clean
	@echo "[$@]"
	@rm -fr "$(BUILD_DIR)" "$(LIBDIR_BUILD)" \
	        build.xml \
	        "$(SRC_DIR_C)/$(JAVA_PKG_UNDER)_PositionUpdate.h" \
	        "$(JAVASRC)/$(JAVA_PKG_DIR)/WireFormatConstants.java"

doc: build.xml
	@$(MAKE) -C doc all
	@ant -f build.xml javadoc

doc-clean:
	@$(MAKE) -C doc clean

install: all
	mkdir -v -p "$(DESTDIR)/$(LIBDIR_INSTALL)"
	cp "$(LIBDIR_BUILD)/$(SONAME)" "$(DESTDIR)/$(LIBDIR_INSTALL)/$(SONAME).$(PLUGIN_VER)"
	$(STRIP) "$(DESTDIR)/$(LIBDIR_INSTALL)/$(SONAME).$(PLUGIN_VER)"
	ldconfig -n "$(DESTDIR)/$(LIBDIR_INSTALL)"
	mkdir -v -p "$(DESTDIR)/$(JAVA_DIR_INSTALL)"
	cp -t "$(DESTDIR)/$(JAVA_DIR_INSTALL)" \
      "$(LIBDIR_BUILD)/$(PROJECT).jar" \
      "$(LIBDIR_BUILD)/$(PROJECT)-src.zip" \
      "$(LIBDIR_BUILD)/$(PROJECT)-doc.zip"

uninstall:
	rm -f "$(DESTDIR)/$(LIBDIR_INSTALL)/$(SONAME)" \
	      "$(DESTDIR)/$(LIBDIR_INSTALL)/$(SONAME).$(PLUGIN_VER)"
	ldconfig -n "$(DESTDIR)/$(LIBDIR_INSTALL)"
	rmdir -v -p --ignore-fail-on-non-empty "$(DESTDIR)/$(LIBDIR_INSTALL)"
	rm -f "$(DESTDIR)/$(JAVA_DIR_INSTALL)/$(PROJECT).jar" \
          "$(DESTDIR)/$(JAVA_DIR_INSTALL)/$(PROJECT)-src.zip" \
          "$(DESTDIR)/$(JAVA_DIR_INSTALL)/$(PROJECT)-doc.zip"
	rmdir -v -p --ignore-fail-on-non-empty "$(DESTDIR)/$(JAVA_DIR_INSTALL)"

endif
