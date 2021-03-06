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
# DESCRIPTION:  Enables backwards compatibility with some old variable names

# fold old distinct variables (if defined) into the new more generic variables

ifneq (,$(CPPFLAGS))
  CFLAGS += $(CPPFLAGS)
  CPPFLAGS :=
endif

ifneq (,$(INC_DIRS))
  CFLAGS += $(INC_DIRS)
  INC_DIRS :=
endif

ifneq (,$(LIB_DIRS))
  LDFLAGS += $(LIB_DIRS)
  LIB_DIRS :=
endif

ifneq (,$(COMFLAGS))
  CFLAGS += $(COMFLAGS)
  LDFLAGS += $(COMFLAGS)
  COM_FLAGS :=
endif
