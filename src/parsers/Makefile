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

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS :=

maininstall:
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='parsers main' install

INSTALLHEADERS_HEADERS := \
	sparse-matrix.h \
	CassandraModel.h \
	CassandraParser.h \
	FastParser.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpPomdpParser.a
BUILDLIB_SRCS := pomdp_spec.tab.cc pomdp_spec.yy.cc \
  imm-reward.c decision-tree.c parse_err.c parse_hash.c \
  sparse-matrix.c mdp.c \
  CassandraModel.cc \
  CassandraParser.cc \
  FastParser.cc
include $(BUILD_DIR)/buildlib.mak

# use 'gmake TEST=1 install' to build the following stuff
ifneq (,$(TEST))

BUILDBIN_TARGET := testDecisionTree
BUILDBIN_SRCS := testDecisionTree.c decision-tree.c
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS :=
include $(BUILD_DIR)/buildbin.mak

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak
