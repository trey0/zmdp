#!gmake 	# Tell emacs about this file: -*- Makefile -*-
#
# Copyright (c) 1996-2005, Carnegie Mellon University. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You may
# obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

######################################################################
# DESCRIPTION:  Included at the header of a Makefile.  Mostly defines
#               variables, including OS- and site-specific stuff.

######################################################################
# USER HELP

.PHONY: defaultbuildrule
defaultbuildrule: install

# avoid using multiple @echo lines (it keeps invoking the shell. slow...)
.PHONY: help advanced
help:
	@echo -e "\nUse one of the following:" \
		 "\n" \
		 "\nmake all		   Build all files (default)" \
		 "\nmake install          Build/install all files" \
		 "\nmake clean            Remove object, dependency files" \
                 "\nmake uninstall        Remove installed libs, bins, data files" \
		 "\nmake realclean        realclean = clean + uninstall" \
		 "\nmake doc              Generate documentation using doxygen (only at top level)\n" \
		 "\nmake advanced         Get more usage help" \
		 "\n"

advanced:
	@echo -e "\nAdvanced targets:" \
		 "\n" \
		 "\nmake foo-local        'make foo' without recursion" \
		 "\nmake <filename>       Builds file according to make rules" \
		 "\nmake localdoc         Build docs for this dir only in gen/html" \
		 "\n" \
		 "\nVariables:" \
		 "\n" \
		 "\nmake OS=VxWorks       Cross-compile (not currently set up)" \
		 "\nmake NO_DEPEND=1      Do not update dependencies" \
		 "\nmake TEST=1           Many individual makefiles build extra" \
		 "\n                         test binaries if TEST=1" \
		 "\nmake echo-foo         Echo value of the make variable 'foo'" \
		 "\nmake vars             Echo value of all make variables" \
		 "\n"

######################################################################
# INITIAL DEFINITIONS

# these variables are declared here so they can be modified
#   by the OS-specific section, and added to later in this file
CFLAGS :=
LDFLAGS :=

######################################################################
# PICK UP OS-SPECIFIC STUFF

# set up default names of binaries and flags for compilers
SRC_DIR := $(CHECKOUT_DIR)/src
BUILD_DIR := $(SRC_DIR)/build
include $(BUILD_DIR)/Platforms/generic.mak

# Determine what operating system we are trying to build for
# Override this on the command line for cross compilation (e.g. OS=m68kVx5.1)
#SHELL = /bin/sh
OS_SYSNAME := $(shell uname -s | perl -ple 'tr/A-Z/a-z/;')
OS_RELEASE := $(shell uname -r | perl -ple 's/\..*$$//;')

OS := $(OS_SYSNAME)$(OS_RELEASE)

-include $(BUILD_DIR)/Platforms/$(OS).mak

######################################################################
# PICK UP USER-SPECIFIC CONFIGURATION

PROJECT := pomdp

BUILDRC_FILE := $(HOME)/.$(PROJECT).buildrc
ifeq (,$(shell ls $(BUILDRC_FILE) 2> /dev/null))
  # use this as a fall-back if there is no buildrc in $HOME
  BUILDRC_FILE := $(BUILD_DIR)/example-$(PROJECT)-buildrc.mak
endif
# use '-include' instead of 'include' so we silently fail if the file
# doesn't exist
-include $(BUILDRC_FILE)

######################################################################
# DIRECTORY DEFINES

THIS_DIR := $(shell pwd)

OBJ_DIR := obj/$(OS)

LOCAL_INC_DIR := include
LOCAL_LIB_DIR := lib/$(OS)
LOCAL_BIN_DIR := bin/$(OS)

TARGET_DIR := $(CHECKOUT_DIR)
TARGET_INC_DIR := $(TARGET_DIR)/include
TARGET_LIB_DIR := $(TARGET_DIR)/lib/$(OS)
TARGET_BIN_DIR := $(TARGET_DIR)/bin/$(OS)
TARGET_COMMON_BIN_DIR := $(TARGET_DIR)/bin/common
TARGET_PYTHON_DIR := $(TARGET_DIR)/lib/python
TARGET_JAVA_DIR := $(TARGET_DIR)/java
TARGET_CLEAN_DIRS := $(TARGET_DIR)/bin $(TARGET_DIR)/lib \
   $(TARGET_DATA_DIR) $(TARGET_INC_DIR)

EXTERNAL_DIR := $(CHECKOUT_DIR)/external
EXTERNAL_LIB_DIR := $(EXTERNAL_DIR)/lib/$(OS)
EXTERNAL_BIN_DIR := $(EXTERNAL_DIR)/bin/$(OS)
EXTERNAL_INC_DIR := $(EXTERNAL_DIR)/include

LDFLAGS += -L$(LOCAL_LIB_DIR) -L$(TARGET_LIB_DIR) -L$(EXTERNAL_LIB_DIR)
CFLAGS += -I. -I$(LOCAL_INC_DIR) -I$(TARGET_INC_DIR) -I$(EXTERNAL_INC_DIR)

VPATH := .:$(LOCAL_INC_DIR)

######################################################################
# INITIALIZE SOME RULES/VARIABLES THAT WILL BE ADDED TO LATER

BUILDLIB_TARGETS :=
BUILDLIB_LOCALS :=
BUILDBIN_TARGETS :=
BUILDBIN_LOCALS :=
INSTALL_DATA_TARGETS :=
INSTALLHEADERS_TARGETS :=
XDRGEN_TARGETS :=
BUILDBIN_SYMLINKS_TO_CLEAN :=

######################################################################
# DEPEND RULES

# bit of a hack to add this
OPTIONS_FILE := $(SRC_DIR)/common/options.mak

# i forget why, but these rules can't go in footer.mak

# need to add this rule explicitly in order to correctly infer that
# a tdl file with the line #include "foo.tdl.h" means that we need to
# generate foo.tdl.h.
$(OBJ_DIR)/%.tdl.d: $(LOCAL_INC_DIR)/%.tdl.cc
	@echo "Updating dependencies for $<..."
	@[ -d $(OBJ_DIR)/$(LOCAL_INC_DIR) ] || mkdir -p $(OBJ_DIR)/$(LOCAL_INC_DIR)
	@$(SHELL) -ec '$(MAKEDEP) $(CFLAGS) $< \
		| $(SED) '\''s|\($(*:%=%.tdl)\)\.o[ :]*|$(OBJ_DIR)/\1.o $@ : $(OPTIONS_FILE) |'\'' > $@; \
		[ -s $@ ] || rm -f $@'

$(OBJ_DIR)/%.d: %.c
	@echo "Updating dependencies for $<..."
	@[ -d $(OBJ_DIR)/$(LOCAL_INC_DIR) ] || mkdir -p $(OBJ_DIR)/$(LOCAL_INC_DIR)
	@$(SHELL) -ec '$(MAKEDEP) $(CFLAGS) $< \
		| $(SED) '\''s|\($*\)\.o[ :]*|$(OBJ_DIR)/\1.o $@ : $(OPTIONS_FILE) |'\'' > $@; \
		[ -s $@ ] || rm -f $@'

$(OBJ_DIR)/%.d: %.cc
	@echo "Updating dependencies for $<..."
	@[ -d $(OBJ_DIR)/$(LOCAL_INC_DIR) ] || mkdir -p $(OBJ_DIR)/$(LOCAL_INC_DIR)
	@$(SHELL) -ec '$(MAKEDEP) $(CFLAGS) $< \
		| $(SED) '\''s|\($*\)\.o[ :]*|$(OBJ_DIR)/\1.o $@ : $(OPTIONS_FILE) |'\'' > $@; \
		[ -s $@ ] || rm -f $@'

$(OBJ_DIR)/%.d: %.cpp
	@echo "Updating dependencies for $<..."
	@[ -d $(OBJ_DIR)/$(LOCAL_INC_DIR) ] || mkdir -p $(OBJ_DIR)/$(LOCAL_INC_DIR)
	@$(SHELL) -ec '$(MAKEDEP) $(CFLAGS) $< \
		| $(SED) '\''s|\($*\)\.o[ :]*|$(OBJ_DIR)/\1.o $@ : $(OPTIONS_FILE) |'\'' > $@; \
		[ -s $@ ] || rm -f $@'
