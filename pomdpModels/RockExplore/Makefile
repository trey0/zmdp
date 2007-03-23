#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.5 2007-03-23 02:19:19 trey Exp $
#
# Copyright (c) 2008, Trey Smith.
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
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='pomdpBounds main' install

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS := 

RE_LIBS := \
	-lzmdpExec \
	-lzmdpSearch \
	-lzmdpPomdpCore \
	-lzmdpPomdpBounds \
	-lzmdpPomdpParser \
	-lzmdpBounds \
	-lzmdpRaceTrack \
	-lzmdpCommon

BUILDBIN_TARGET := zmdpRockExplore
BUILDBIN_SRCS := zmdpRockExplore.cc REBasicPomdp.cc RockExplore.cc RockExplorePolicy.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := $(RE_LIBS)
include $(BUILD_DIR)/buildbin.mak

ifneq (,$(TEST))

# add targets here to be built with 'make TEST=1 install'

endif


######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.4  2007/03/07 08:12:27  trey
# refactored things
#
# Revision 1.3  2007/03/06 08:46:56  trey
# many tweaks
#
# Revision 1.2  2007/03/06 04:32:47  trey
# working towards heuristic policies
#
# Revision 1.1  2007/03/05 08:58:26  trey
# initial check-in
#
#
