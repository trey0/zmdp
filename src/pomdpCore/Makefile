#!gmake 	# Tell emacs about this file: -*- Makefile -*-
#
# Copyright (c) 2002-2005, Trey Smith. All rights reserved.
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
# DO NOT MODIFY THIS HEADER SECTION

CHECKOUT_DIR := $(shell perl -e '$$_ = `pwd`; chop; $$_ = scalar reverse("$$_/"); if (s:.*/crs/::) { print scalar reverse($$_); } else { print "<error>"; die "*** could not calculate CHECKOUT_DIR ***\n"; }')
include $(CHECKOUT_DIR)/src/build/header.mak

######################################################################
# PLACE MODIFICATIONS BELOW THIS LINE

include $(SRC_DIR)/common/options.mak

maininstall:
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='pomdpCore main' install

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS :=

INSTALLHEADERS_HEADERS := \
	Pomdp.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpPomdpCore.a
BUILDLIB_SRCS := \
	Pomdp.cc
include $(BUILD_DIR)/buildlib.mak

ifneq (,$(TEST))

# add targets here to be built with 'make TEST=1 install'

BUILDBIN_TARGET := testPomdpRead
BUILDBIN_SRCS := testPomdpRead.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpPomdpCore -lzmdpPomdpParser -lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak
