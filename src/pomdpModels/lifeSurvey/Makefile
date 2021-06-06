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
	$(MAKE) $(MFLAGS) -C ../.. -fMakefile SUBDIRS='pomdpModels/lifeSurvey main' install

INSTALLHEADERS_HEADERS := \
	LSModelFile.h \
	LifeSurvey.h \
	LSPathAndReactExec.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpLifeSurvey.a
BUILDLIB_SRCS := \
	LSModelFile.cc \
	LifeSurvey.cc \
	LSPathAndReactExec.cc
include $(BUILD_DIR)/buildlib.mak

BUILDBIN_TARGET := gen_LifeSurvey
BUILDBIN_SRCS := gen_LifeSurvey.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpLifeSurvey
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := genTargetLocations
BUILDBIN_SRCS := genTargetLocations.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpLifeSurvey
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := calcLSTrialStats
BUILDBIN_SRCS := calcLSTrialStats.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpLifeSurvey
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := dumpMap
BUILDBIN_SRCS := dumpMap.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpLifeSurvey
include $(BUILD_DIR)/buildbin.mak

ifneq (,$(TEST))

# add targets here to be built with 'make TEST=1 install'

BUILDBIN_TARGET := testLSModelFile
BUILDBIN_SRCS := testLSModelFile.cc LSModelFile.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS :=
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := testLSPathAndReact
BUILDBIN_SRCS := testLSPathAndReact.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := \
	-lzmdpLifeSurvey \
	-lzmdpExec \
	-lzmdpPomdpCore \
	-lzmdpPomdpBounds \
	-lzmdpPomdpParser \
	-lzmdpBounds \
	-lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak
