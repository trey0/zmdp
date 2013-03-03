#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: header.mak,v 1.10 2006-11-29 21:01:31 trey Exp $
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

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.9  2006/05/20 03:53:22  trey
# changed "make" command to build "install" target by default, less confusing for new users
#
# Revision 1.8  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.7  2005/11/28 20:55:26  trey
# replaced "gmake" with "make" in help text, now infer operating system instead of always using linux2
#
# Revision 1.6  2005/11/03 20:24:32  trey
# added options.mak as a dependency of all *.o files
#
# Revision 1.5  2005/10/28 03:34:41  trey
# switched to simpler license
#
# Revision 1.4  2005/10/28 02:24:42  trey
# added copyright headers
#
# Revision 1.3  2004/11/09 21:31:59  trey
# got pomdp source tree into a building state again
#
# Revision 1.2  2004/11/09 18:33:06  trey
# adjusted to new project name
#
# Revision 1.1.1.1  2004/11/09 16:18:57  trey
# imported hsvi into new repository
#
# Revision 1.4  2004/05/21 19:16:59  trey
# branched example-buildrc.mak into two files, one for atacama and one for microraptor; added COMPILER_DIRECTORY_WITH_SLASH configuration variable
#
# Revision 1.3  2004/05/13 20:52:50  trey
# fixed so that a build works even if you do not have a buildrc in $HOME
#
# Revision 1.2  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.6  2004/03/19 18:26:33  trey
# cleaned up some variables
#
# Revision 1.5  2004/03/19 16:23:07  trey
# fixed some bugs i found with dom
#
# Revision 1.4  2004/03/19 14:59:40  trey
# changed buildrc install location
#
# Revision 1.3  2004/03/19 14:50:40  trey
# major changes to support "gmake all" and other features atacama folks want
#
# Revision 1.2  2003/10/07 16:01:17  trey
# changed directory names to reflect move
#
# Revision 1.1  2003/10/05 16:51:41  trey
# initial atacama check-in
#
# Revision 1.1  2003/10/05 02:59:35  trey
# initial microraptor check-in
#
# Revision 1.11  2003/04/25 18:42:14  danig
# Modified to allow compilation on a system with linux kernel version
# 2.4 and gcc 2.95.3.
#
# Revision 1.10  2003/02/17 22:57:55  trey
# got rid of obsolete GRP setting
#
# Revision 1.9  2003/02/17 20:54:40  trey
# added a variable used for python/swig compilation
#
# Revision 1.8  2002/04/25 22:13:40  trey
# added java rules; removed xdrgen target now that the system can auto-detect when xdrgen needs to be run
#
# Revision 1.7  2002/04/25 01:37:17  trey
# turned back on suppression of echoing dependency generation commands
#
# Revision 1.6  2002/04/25 01:24:56  trey
# fixed bug that *.tdl.h files were not being automatically generated if another *.tdl file tried to include them
#
# Revision 1.5  2002/02/12 15:49:59  trey
# fixed *.c++ to be *.cpp
#
# Revision 1.4  2002/02/12 15:43:28  trey
# added a *.c++ dependency generation rule
#
# Revision 1.3  2001/08/27 19:12:41  trey
# fixed SRC_DIR problem
#
# Revision 1.2  2001/08/27 18:01:06  trey
# Makefile
#
# Revision 1.21  2001/03/21 19:25:02  trey
# fixed so that we have corresponding dependency-generating rules for .c and .cc files
#
# Revision 1.20  2001/03/06 17:48:22  trey
# set up auto-generation of hierarchical directory of docs
#
# Revision 1.19  2001/03/06 04:01:39  trey
# added rules for documentation generation, gmake doc
#
# Revision 1.18  2001/02/13 17:09:46  trey
# added to advanced help output
#
# Revision 1.17  2001/02/13 16:33:35  trey
# switched include of headers under src to use "dir/foo.h" instead of <dir/foo.h>; dependency checking is now easier. made some corresponding makefile changes
#
# Revision 1.16  2001/02/07 20:35:36  trey
# minor Build system updates to support cleaning up after xdrgen
#
# Revision 1.15  2001/02/06 02:03:12  trey
# added rules for generating header files from XDR specs
#
# Revision 1.14  2000/10/01 00:44:02  trey
# added separate CPPFLAGS variable for things that also need to be passed to makedepend; fixed help target to display properly under linux
#
# Revision 1.13  2000/09/30 23:25:37  trey
# modified makefile system to put generated source and header files in the gen/ directory
#
# Revision 1.12  2000/09/20 22:37:30  trey
# added . to the include path and moved generic.mak up in the file so that $(PERL) gets defined before we need it
#
# Revision 1.11  2000/09/20 18:27:07  trey
# fixed to avoid putting another file in Build just for the linux-major-version issue
#
# Revision 1.10  2000/09/17 19:26:41  hersh
# Changed OS_RELEASE to be only major and minor number, no patch numbers.
#
# Revision 1.9  2000/08/28 16:48:46  trey
# fixed help msg
#
# Revision 1.8  2000/02/25 22:11:35  trey
# added support for scripts and Logs directory
#
# Revision 1.7  2000/02/14 20:34:36  trey
# fixed afs-related problems
#
# Revision 1.6  2000/02/02 18:35:29  trey
# fixed unfortunate tendency to remake dependencies on gmake clean, some other minor fixes
#
# Revision 1.5  1999/11/11 15:08:23  trey
# now uses diraenv environment variables; using installdata is also streamlined
#
# Revision 1.4  1999/11/08 15:42:31  trey
# updated to reflect use of diraenv file
#
# Revision 1.3  1999/11/03 20:51:44  trey
# added support for flex/bison, tweaked other stuff
#
# Revision 1.2  1999/10/27 18:41:04  trey
# Makefile system overhaul after talking to Reid
#
# Revision 1.1.1.1  1999/10/27 02:48:58  trey
# Imported sources
#
#
