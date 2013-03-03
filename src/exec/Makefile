#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.7 2007-04-19 22:17:35 trey Exp $
#
# Copyright (c) 2006, Trey Smith. All rights reserved.
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
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='exec main' install

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS := 

CFLAGS +=

INSTALLHEADERS_HEADERS := \
	MDPExec.h \
	BoundPairExec.h \
	PolicyEvaluator.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpExec.a
BUILDLIB_SRCS := \
	MDPExec.cc \
	BoundPairExec.cc \
	PolicyEvaluator.cc
include $(BUILD_DIR)/buildlib.mak

ifneq (,$(TEST))

# add targets here to be built with 'make TEST=1 install'

BUILDBIN_TARGET := testExec
BUILDBIN_SRCS := testExec.cc
BUILDBIN_INDEP_LIBS := 
BUILDBIN_DEP_LIBS := \
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
# Revision 1.6  2007/03/23 02:12:39  trey
# renamed libzmdpPomdpExec.a to libzmdpExec.a for consistency
#
# Revision 1.5  2007/03/23 01:05:06  trey
# added PolicyEvaluator
#
# Revision 1.4  2007/03/23 00:25:55  trey
# renamed testPomdpExec to testMDPExec
#
# Revision 1.3  2007/03/23 00:01:04  trey
# fixed to reflect migration from PomdpExec to MDPExec base class
#
# Revision 1.2  2006/06/27 18:20:31  trey
# added MaxPlanesLowerBoundExec
#
# Revision 1.1  2006/06/24 16:25:27  trey
# initial check-in
#
#
