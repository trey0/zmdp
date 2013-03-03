#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: buildbin.mak,v 1.4 2006-04-28 17:57:41 trey Exp $
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
# DESCRIPTION:  Include to build a binary.
#
# Input variables:
#
# BUILDBIN_TARGET
#    The name of the binary to build (e.g. ctr)
# BUILDBIN_SRCS
#    Source files to link into binary (e.g. ctr.cc)
# BUILDBIN_DEP_LIBS
#    Libraries to link against: specifically, ones that we should
#    include in dependency checking in case they are rebuilt
#    (e.g. -lctr)
# BUILDBIN_INDEP_LIBS
#    Libraries to link against: external libraries that are unlikely
#    to be rebuilt, so skip dependency checking (e.g. -lipc)
# BUILDBIN_LDFLAGS
#    If set to a non-null string, this overrides the value of
#    LDFLAGS just for this binary.
# BUILDBIN_WHOLE_DEP_LIBS
# BUILDBIN_WHOLE_INDEP_LIBS
#    These arguments are rarely used. They correspond to
#    BUILDBIN_DEP_LIBS and BUILDBIN_INDEP_LIBS, but the WHOLE specifies
#    that symbols in the library which are not used by preceding object and
#    library files should nonetheless be included in the binary: this is
#    important, for instance, if there are global variables in the library
#    that are not referenced outside the library, but whose constructors
#    have useful side effects.

include $(BUILD_DIR)/fixvars.mak # backwards compatibility

ifeq (,$(BUILDBIN_LDFLAGS))
  BUILDBIN_LDFLAGS := $(LDFLAGS)
endif

ifneq (,$(SKIP_LOCALS))
  BUILDBIN_BIN_DIR := $(TARGET_BIN_DIR)
else
  BUILDBIN_BIN_DIR := $(LOCAL_BIN_DIR)
endif

_BT := $(BUILDBIN_BIN_DIR)/$(BUILDBIN_TARGET)

BUILDBIN_OBJS := $(BUILDBIN_SRCS)
BUILDBIN_OBJS := $(BUILDBIN_OBJS:%.c=$(OBJ_DIR)/%.o)
BUILDBIN_OBJS := $(BUILDBIN_OBJS:%.cc=$(OBJ_DIR)/%.o)
BUILDBIN_OBJS := $(BUILDBIN_OBJS:%.cpp=$(OBJ_DIR)/%.o)

$(_BT)_OBJS := $(BUILDBIN_OBJS)
$(_BT)_DEP_LIBS := $(BUILDBIN_DEP_LIBS)
$(_BT)_INDEP_LIBS := $(BUILDBIN_INDEP_LIBS)
$(_BT)_WHOLE_DEP_LIBS := $(BUILDBIN_WHOLE_DEP_LIBS)
$(_BT)_WHOLE_INDEP_LIBS := $(BUILDBIN_WHOLE_INDEP_LIBS)
$(_BT)_LDFLAGS := $(BUILDBIN_LDFLAGS)

$(_BT)_WHOLE := $($(_BT)_WHOLE_DEP_LIBS)
ifneq (,$($(_BT)_WHOLE_INDEP_LIBS))
  $(_BT)_WHOLE += $($(_BT)_WHOLE_INDEP_LIBS)
endif
ifneq (,$($(_BT)_WHOLE))
  $(_BT)_WHOLE := $(LDFLAGS_START_WHOLE) $($(_BT)_WHOLE) $(LDFLAGS_END_WHOLE)
endif

BUILDBIN_EXP_DEP_LIBS := \
  $(shell $(PERL) $(BUILD_DIR)/findlibs.perl $(LDFLAGS) $(BUILDBIN_DEP_LIBS))

ifneq (,$(BUILDBIN_WHOLE_DEP_LIBS))
  BUILDBIN_EXP_WHOLE_DEP_LIBS := \
    $(shell $(PERL) $(BUILD_DIR)/findlibs.perl $(LDFLAGS) $(BUILDBIN_WHOLE_DEP_LIBS))
else
  BUILDBIN_EXP_WHOLE_DEP_LIBS :=
endif

$(_BT): $($(_BT)_OBJS) $(BUILDBIN_EXP_DEP_LIBS) $(BUILDBIN_EXP_WHOLE_DEP_LIBS) Makefile
	@[ -d $(BUILDBIN_BIN_DIR) ] || mkdir -p $(BUILDBIN_BIN_DIR)
	$(LD) $($@_LDFLAGS) $($@_OBJS) $($@_WHOLE) $($@_DEP_LIBS) $($@_INDEP_LIBS) -o $@.tmp
	mv -f $@.tmp $@

BUILDBIN_DEPS := $(BUILDBIN_OBJS:$(OBJ_DIR)/%.o=$(OBJ_DIR)/%.d)
BUILDBIN_LOCALS += $(_BT)
BUILDBIN_TARGETS += $(TARGET_BIN_DIR)/$(BUILDBIN_TARGET)

$(BUILDBIN_TARGET): $(_BT)
	ln -sf $< $@

ifeq (,$(SKIP_LOCALS))

# for Dom :)
ifneq (,$(PUT_MY_BINARIES_RIGHT_HERE_THANK_YOU_VERY_MUCH))
  BUILDBIN_LOCALS += $(BUILDBIN_TARGET)
endif

BUILDBIN_SYMLINKS_TO_CLEAN += $(BUILDBIN_TARGET)

$(TARGET_BIN_DIR)/$(BUILDBIN_TARGET): $(_BT)
	@[ -d $(TARGET_BIN_DIR) ] || mkdir -p $(TARGET_BIN_DIR)
	$(INSTALLBINS_COPY) $< $@

endif # not SKIP_LOCALS

ifdef DEPEND
ifneq ($(BUILDBIN_DEPS),)
-include $(BUILDBIN_DEPS)
endif
endif

# clear input variables
BUILDBIN_TARGET :=
BUILDBIN_SRCS :=
BUILDBIN_DEP_LIBS :=
BUILDBIN_INDEP_LIBS :=
BUILDBIN_LDFLAGS :=
BUILDBIN_WHOLE_DEP_LIBS :=
BUILDBIN_WHOLE_INDEP_LIBS :=

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.3  2005/10/28 03:34:41  trey
# switched to simpler license
#
# Revision 1.2  2005/10/28 02:24:42  trey
# added copyright headers
#
# Revision 1.1.1.1  2004/11/09 16:18:57  trey
# imported hsvi into new repository
#
# Revision 1.2  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.6  2004/03/19 20:39:26  trey
# added per-binary BUILDBIN_LDFLAGS variable
#
# Revision 1.5  2004/03/19 20:00:37  trey
# fixed issue with symlinks
#
# Revision 1.4  2004/03/19 18:26:33  trey
# cleaned up some variables
#
# Revision 1.3  2004/03/19 16:23:07  trey
# fixed some bugs i found with dom
#
# Revision 1.2  2004/03/19 14:50:40  trey
# major changes to support "gmake all" and other features atacama folks want
#
# Revision 1.1  2003/10/05 16:51:41  trey
# initial atacama check-in
#
# Revision 1.1  2003/10/05 02:59:35  trey
# initial microraptor check-in
#
# Revision 1.7  2002/05/23 15:23:15  trey
# fixed problem with multiple buildbin.mak includes using WHOLE_DEP flags in the same GNUmakefile
#
# Revision 1.6  2002/05/22 16:15:33  trey
# removed LDFLAGS_END variable and made --whole-archive flag not appear unless WHOLE_DEP_LIBS or WHOLE_INDEP_LIBS are specified
#
# Revision 1.5  2002/05/21 22:58:51  danig
# Changed BUILDBIN_WHOLE_LIBS to BUILDBIN_WHOLE_DEP_LIBS and
# BUILDBIN_WHOLE_INDEP_LIBS.  Libraries included under these headings
# have their entire contents included in the executable.  This can cause
# executable bloat, so it should be used appropriately.
#
# Revision 1.4  2002/05/21 22:51:54  danig
# Added BUILDBIN_WHOLE_LIBS plus rule for linking entire static
# libraries into the executable.
#
# Revision 1.3  2002/05/21 22:26:57  danig
# Added the LDFLAGS_END variable.  Also added the --whole-archive and
# --no-whole-archive link flags in order to allow automatic inclusion
# of static global variables.
#
# Revision 1.2  2002/05/13 19:43:31  trey
# suppressed some extra echos
#
# Revision 1.1.1.1  2001/08/27 15:46:51  trey
# initial check-in
#
# Revision 1.6  2000/09/30 23:25:37  trey
# modified makefile system to put generated source and header files in the gen/ directory
#
# Revision 1.5  2000/02/14 20:34:34  trey
# fixed afs-related problems
#
# Revision 1.4  2000/02/02 18:35:27  trey
# fixed unfortunate tendency to remake dependencies on gmake clean, some other minor fixes
#
# Revision 1.3  1999/11/03 20:51:43  trey
# added support for flex/bison, tweaked other stuff
#
# Revision 1.2  1999/10/27 18:40:52  trey
# Makefile system overhaul after talking to Reid
#
# Revision 1.1.1.1  1999/10/27 02:48:58  trey
# Imported sources
#
#
