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
