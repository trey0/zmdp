#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.9 2007-04-22 22:42:33 trey Exp $
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

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.8  2007/03/23 02:18:29  trey
# fixed to reflect name change -lzmdpPomdpExec -> -lzmdpExec
#
# Revision 1.7  2006/09/12 19:46:52  trey
# added dumpMap
#
# Revision 1.6  2006/07/10 02:22:41  trey
# added calcLSTrialStats
#
# Revision 1.5  2006/06/29 21:39:04  trey
# added LSPathAndReact and corresponding test
#
# Revision 1.4  2006/06/27 16:04:40  trey
# refactored so outside code can access the LifeSurvey model using -lzmdpLifeSurvey
#
# Revision 1.4  2006/06/26 21:34:45  trey
# regrouped most source files into libzmdpLifeSurvey.a
#
# Revision 1.3  2006/06/16 14:45:20  trey
# added ability to generate target map
#
# Revision 1.2  2006/06/12 18:12:30  trey
# rename LSModel -> LSModelFile; uncommented gen_LifeSurvey build
#
# Revision 1.1  2006/06/11 14:37:39  trey
# initial check-in
#
# Revision 1.6  2006/05/27 19:01:43  trey
# added testPomdpRead
#
# Revision 1.5  2006/04/28 21:14:33  trey
# removed QMDP
#
# Revision 1.4  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.3  2006/04/06 04:13:20  trey
# removed several files now in other directories
#
# Revision 1.2  2006/01/31 19:18:12  trey
# added Pomdp{Lower,Upper}Bound
#
# Revision 1.1  2006/01/31 18:31:50  trey
# moved many files from common to pomdpCore
#
#
