#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.6 2007-01-14 01:23:20 trey Exp $
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
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='bounds main' install

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS := 

INSTALLHEADERS_HEADERS := \
	AbstractBound.h \
	MDPCache.h \
	IncrementalLowerBound.h \
	IncrementalUpperBound.h \
	PointLowerBound.h \
	PointUpperBound.h \
	BoundPairCore.h \
	RelaxUBInitializer.h \
	BoundPair.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpBounds.a
BUILDLIB_SRCS := \
	MDPCache.cc \
	PointLowerBound.cc \
	PointUpperBound.cc \
	BoundPairCore.cc \
	RelaxUBInitializer.cc \
	BoundPair.cc
include $(BUILD_DIR)/buildlib.mak

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.5  2006/10/24 02:08:01  trey
# much refactoring
#
# Revision 1.4  2006/10/17 19:16:25  trey
# added MDPCache.cc
#
# Revision 1.3  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.2  2006/04/05 21:35:43  trey
# shuffled how source files fit into libraries
#
# Revision 1.1  2006/04/04 17:22:51  trey
# initial check-in
#
#
