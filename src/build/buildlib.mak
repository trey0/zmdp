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
# DESCRIPTION:  Used to build a library.
#
# Input variables:
#
# BUILDLIB_TARGET
#    The name of the library to build (e.g. libctr.a)
# BUILDLIB_SRCS
#    Source files to link into library (e.g. ctr.cc)

include $(BUILD_DIR)/fixvars.mak # backwards compatibility

ifneq (,$(SKIP_LOCALS))
  BUILDLIB_LIB_DIR := $(TARGET_LIB_DIR)
else
  BUILDLIB_LIB_DIR := $(LOCAL_LIB_DIR)
endif

_LT := $(BUILDLIB_LIB_DIR)/$(BUILDLIB_TARGET)

BUILDLIB_OBJS := $(BUILDLIB_SRCS)
BUILDLIB_OBJS := $(BUILDLIB_OBJS:%.c=$(OBJ_DIR)/%.o)
BUILDLIB_OBJS := $(BUILDLIB_OBJS:%.cc=$(OBJ_DIR)/%.o)
BUILDLIB_OBJS := $(BUILDLIB_OBJS:%.cpp=$(OBJ_DIR)/%.o)

$(_LT)_OBJS := $(BUILDLIB_OBJS)

$(_LT): $($(_LT)_OBJS) Makefile
	@[ -d $(BUILDLIB_LIB_DIR) ] || mkdir -p $(BUILDLIB_LIB_DIR)
	$(AR) $(ARFLAGS) $@.tmp $($@_OBJS)
ifneq ($(RANLIB),)
	$(RANLIB) $@.tmp
endif
	mv -f $@.tmp $@

BUILDLIB_DEPS := $(BUILDLIB_OBJS:$(OBJ_DIR)/%.o=$(OBJ_DIR)/%.d)
BUILDLIB_TARGETS += $(TARGET_LIB_DIR)/$(BUILDLIB_TARGET)


ifeq (,$(SKIP_LOCALS))

BUILDLIB_LOCALS += $(_LT)

$(TARGET_LIB_DIR)/$(BUILDLIB_TARGET): $(_LT)
	@[ -d $(TARGET_LIB_DIR) ] || mkdir -p $(TARGET_LIB_DIR)
	$(INSTALLLIBS_COPY) $< $@

endif # not SKIP_LOCALS

ifdef DEPEND
ifneq ($(BUILDLIB_DEPS),)
-include $(BUILDLIB_DEPS)
endif
endif

# clear input variables
BUILDLIB_TARGET :=
BUILDLIB_SRCS :=
