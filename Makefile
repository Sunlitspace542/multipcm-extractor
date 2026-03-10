#
# Build m2me with make
# Generated with makeprojects.makefile
#

#
# Custom output files
#

EXTRA_OBJS:=

#
# Project to build without a goal
#

.DEFAULT_GOAL := all

#
# Default executable name
#

EXENAME ?= m2me

#
# List the names of all of the final binaries to build and clean
#

.PHONY: all
all: Release ;

.PHONY: clean
clean: clean_Debug clean_Internal clean_Release ;

#
# Configurations
#

.PHONY: Debug
Debug: Debuglnx ;

.PHONY: clean_Debug
clean_Debug: clean_Debuglnx ;

.PHONY: Internal
Internal: Internallnx ;

.PHONY: clean_Internal
clean_Internal: clean_Internallnx ;

.PHONY: Release
Release: Releaselnx ;

.PHONY: clean_Release
clean_Release: clean_Releaselnx ;

#
# Platforms
#

.PHONY: lnx
lnx: Debuglnx Internallnx Releaselnx ;

.PHONY: clean_lnx
clean_lnx: clean_Debuglnx clean_Internallnx clean_Releaselnx ;

#
# List of binaries to build or clean
#

.PHONY: Debuglnx
Debuglnx:
	@$(MAKE) -e --no-print-directory CONFIG=Debug TARGET=lnx -f Makefile bin/$(EXENAME)_dbg

.PHONY: clean_Debuglnx
clean_Debuglnx:
	@-rm -f $(EXTRA_OBJS)
	@-rm -rf temp/$(EXENAME)_dbg
	@-rm -f bin/$(EXENAME)_dbg
	@if [ -d bin ] && files=$$(ls -qAL -- bin) && [ -z "$$files" ]; then rm -fd bin; fi
	@if [ -d temp ] && files=$$(ls -qAL -- temp) && [ -z "$$files" ]; then rm -fd temp; fi

.PHONY: Internallnx
Internallnx:
	@$(MAKE) -e --no-print-directory CONFIG=Internal TARGET=lnx -f Makefile bin/$(EXENAME)_int

.PHONY: clean_Internallnx
clean_Internallnx:
	@-rm -f $(EXTRA_OBJS)
	@-rm -rf temp/$(EXENAME)_int
	@-rm -f bin/$(EXENAME)_int
	@if [ -d bin ] && files=$$(ls -qAL -- bin) && [ -z "$$files" ]; then rm -fd bin; fi
	@if [ -d temp ] && files=$$(ls -qAL -- temp) && [ -z "$$files" ]; then rm -fd temp; fi

.PHONY: Releaselnx
Releaselnx:
	@$(MAKE) -e --no-print-directory CONFIG=Release TARGET=lnx -f Makefile bin/$(EXENAME)

.PHONY: clean_Releaselnx
clean_Releaselnx:
	@-rm -f $(EXTRA_OBJS)
	@-rm -rf temp/$(EXENAME)
	@-rm -f bin/$(EXENAME)
	@if [ -d bin ] && files=$$(ls -qAL -- bin) && [ -z "$$files" ]; then rm -fd bin; fi
	@if [ -d temp ] && files=$$(ls -qAL -- temp) && [ -z "$$files" ]; then rm -fd temp; fi

#
# Create the folder for the binary output
#

bin:
	@-mkdir -p bin

temp:
	@-mkdir -p temp

#
# Disable building this make file
#

Makefile: ;

#
# Code below can only be invoked indirectly
#

ifneq (0,$(MAKELEVEL))

#
# Default configuration
#

CONFIG ?= Release

#
# Default target
#

TARGET ?= lnx

#
# Directory name fragments
#

TARGET_SUFFIX_lnx := lnx

CONFIG_SUFFIX_Debug := dbg
CONFIG_SUFFIX_Internal := int
CONFIG_SUFFIX_Release := rel

#
# Base name of the temp directory
#

BASE_SUFFIX := mak$(TARGET_SUFFIX_$(TARGET))$(CONFIG_SUFFIX_$(CONFIG))
TEMP_DIR := temp/m2me$(BASE_SUFFIX)

#
# SOURCE_DIRS = Work directories for the source code
#

SOURCE_DIRS :=.

#
# INCLUDE_DIRS = Header includes
#

INCLUDE_DIRS = $(SOURCE_DIRS)

#
# Set the compiler flags for each of the build types
#

CFlagsDebuglnx:= -g -Og -D_DEBUG -D__LINUX__
CFlagsInternallnx:= -g -O3 -D_DEBUG -D__LINUX__
CFlagsReleaselnx:= -O3 -DNDEBUG -D__LINUX__

#
# Set the assembler flags for each of the build types
#

AFlagsDebuglnx:= --defsym _DEBUG=1 --defsym __LINUX__=1
AFlagsInternallnx:= --defsym _DEBUG=1 --defsym __LINUX__=1
AFlagsReleaselnx:= --defsym NDEBUG=1 --defsym __LINUX__=1

#
# Set the Linker flags for each of the build types
#

LFlagsDebuglnx:= -g
LFlagsInternallnx:= -g
LFlagsReleaselnx:=

# Now, set the compiler flags

C_INCLUDES:=$(addprefix -I,$(INCLUDE_DIRS))
CL:=$(CC) -c -Wall -x c $(C_INCLUDES)
CP:=$(CXX) -c -Wall -x c++ $(C_INCLUDES)
ASM:=$(AS)
LINK:=$(CXX)

#
# Default build recipes
#

define BUILD_C=
@echo $(<F) / $(CONFIG) / $(TARGET); \
$(CL) $(CFlags$(CONFIG)$(TARGET)) $< -o $@ -MT '$@' -MMD -MF '$*.d'
endef

define BUILD_CPP=
@echo $(<F) / $(CONFIG) / $(TARGET); \
$(CP) $(CFlags$(CONFIG)$(TARGET)) $< -o $@ -MT '$@' -MMD -MF '$*.d'
endef

define BUILD_ASMX86=
@echo $(<F) / $(CONFIG) / $(TARGET); \
$(AS) --defsym __i386__=1 $(AFlags$(CONFIG)$(TARGET)) $< -o $@ -MD '$*.d'
endef

define BUILD_ASMX64=
@echo $(<F) / $(CONFIG) / $(TARGET); \
$(AS) --defsym __amd64__=1 $(AFlags$(CONFIG)$(TARGET)) $< -o $@ -MD '$*.d'
endef

#
# Object files to work with for the library
#

OBJS:= $(TEMP_DIR)/byte_order.o \
	$(TEMP_DIR)/m2me.o \
	$(TEMP_DIR)/wave.o

DEPS:= $(TEMP_DIR)/byte_order.d \
	$(TEMP_DIR)/m2me.d \
	$(TEMP_DIR)/wave.d

#
# Disable building the source files
#

byte_order.c \
m2me.c \
wave.c \
	: ;

#
# Build the object file folder
#

$(OBJS): | $(TEMP_DIR)

$(TEMP_DIR):
	@-mkdir -p $(TEMP_DIR)

#
# Build the object files
#

$(TEMP_DIR)/byte_order.o: byte_order.c ; $(BUILD_C)

$(TEMP_DIR)/m2me.o: m2me.c ; $(BUILD_C)

$(TEMP_DIR)/wave.o: wave.c ; $(BUILD_C)

#
# Create final binaries
#

bin/$(EXENAME)_dbg: $(EXTRA_OBJS) $(OBJS) Makefile | bin
	@echo Performing link...
	@$(LINK) -o bin/$(EXENAME)_dbg $(OBJS) $(LFlags$(CONFIG)$(TARGET))

bin/$(EXENAME)_int: $(EXTRA_OBJS) $(OBJS) Makefile | bin
	@echo Performing link...
	@$(LINK) -o bin/$(EXENAME)_int $(OBJS) $(LFlags$(CONFIG)$(TARGET))

bin/$(EXENAME): $(EXTRA_OBJS) $(OBJS) Makefile | bin
	@echo Performing link...
	@$(LINK) -o bin/$(EXENAME) $(OBJS) $(LFlags$(CONFIG)$(TARGET))

#
# Speed up make by telling it to ignore these extensions
#

%.d: ;

%: %,v

%: RCS/%,v

%: RCS/%

%: s.%

%: SCCS/s.%

%.h: ;

#
# Include the generated dependencies
#

-include $(DEPS)

endif
