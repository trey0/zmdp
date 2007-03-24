#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.8 2007-03-24 22:44:45 trey Exp $
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
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='mdps main' install

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS := 

INSTALLHEADERS_HEADERS := \
	GenericDiscreteMDP.h \
	RaceTrack.h \
	CacheMDP.h \
	CustomMDP.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpRaceTrack.a
BUILDLIB_SRCS := \
	GenericDiscreteMDP.cc \
	RaceTrack.cc \
	CacheMDP.cc \
	CustomMDP.cc
include $(BUILD_DIR)/buildlib.mak

ifneq (,$(TEST))

# add targets here to be built with 'make TEST=1 install'

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.7  2007/01/14 01:24:21  trey
# removed obsolete reference to boost
#
# Revision 1.6  2006/11/09 20:46:41  trey
# added GenericDiscreteMDP.cc
#
# Revision 1.5  2006/11/08 17:59:05  trey
# added GenericDiscreteMDP.h
#
# Revision 1.4  2006/11/07 20:09:56  trey
# added CustomMDP.{h,cc}
#
# Revision 1.3  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.2  2006/02/06 19:27:59  trey
# added maininstall target for convenience
#
# Revision 1.1  2006/02/01 18:02:07  trey
# initial check-in
#
# Revision 1.2  2006/01/31 19:18:12  trey
# added Pomdp{Lower,Upper}Bound
#
# Revision 1.1  2006/01/31 18:31:50  trey
# moved many files from common to pomdpCore
#
#
