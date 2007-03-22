#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.9 2007-03-22 18:25:49 trey Exp $
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

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.8  2007/01/14 01:24:40  trey
# removed obsolete reference to boost
#
# Revision 1.7  2006/11/05 03:25:25  trey
# added maininstall target for convenience
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
