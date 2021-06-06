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
# DESCRIPTION:  Include to install a file in the data directory.
#
# Input variables:
#
# INSTALLDATA_FILES
#   Files to install (e.g. foo.config)
# INSTALLDATA_DIR
#   Directory to put files in (e.g. $(HOME))

INSTALLDATA_SRCS := $(INSTALLDATA_FILES:%=$(THIS_DIR)/%)
INSTALLDATA_TARGETS := $(INSTALLDATA_FILES:%=$(INSTALLDATA_DIR)/%)

# the installdata target depends on $(INSTALL_DATA_TARGETS)
INSTALL_DATA_TARGETS += $(INSTALLDATA_TARGETS)

_LT := $(word 1,$(INSTALLDATA_SRCS))
$(_LT)_DIR := $(INSTALLDATA_DIR)

$(INSTALLDATA_TARGETS): $(INSTALLDATA_SRCS)
	@[ -d $($<_DIR) ] || mkdir -p $($<_DIR)
	$(INSTALLDATA_COPY) $? $($<_DIR)

# clear input variables
INSTALLDATA_FILES :=
INSTALLDATA_DIR :=
