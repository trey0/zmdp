#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.16 2006-10-20 04:56:49 trey Exp $
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
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='search main' install

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS :=

INSTALLHEADERS_HEADERS := \
	RTDPCore.h \
	FRTDP.h \
	HSVI.h \
	RTDP.h \
	LRTDP.h \
	HDP.h \
	ScriptedUpdater.h \
	StateLog.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpSearch.a
BUILDLIB_SRCS := \
	RTDPCore.cc \
	FRTDP.cc \
	HSVI.cc \
	RTDP.cc \
	LRTDP.cc \
	HDP.cc \
	ScriptedUpdater.cc \
	StateLog.cc
include $(BUILD_DIR)/buildlib.mak

ifneq (,$(TEST))

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.15  2006/10/19 19:32:17  trey
# added support for backup logging
#
# Revision 1.14  2006/06/03 10:57:13  trey
# changed directory name rtdp -> search in maininstall rule
#
# Revision 1.13  2006/04/28 22:33:41  trey
# renamed zmdpRtdp -> zmdpSearch
#
# Revision 1.12  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.11  2006/04/27 20:18:27  trey
# renamed WRTDP to HSVI, maybe less confusing this way
#
# Revision 1.10  2006/04/12 19:22:51  trey
# added WRTDP
#
# Revision 1.9  2006/04/07 19:44:45  trey
# permanently removed ARTDP
#
# Revision 1.8  2006/04/06 04:15:27  trey
# combined separate libraries into single library for easier use
#
# Revision 1.7  2006/04/04 17:25:50  trey
# no longer maintaining ARTDP
#
# Revision 1.6  2006/03/17 20:06:57  trey
# added ARTDP
#
# Revision 1.5  2006/02/17 18:35:31  trey
# added HDP
#
# Revision 1.4  2006/02/13 21:46:58  trey
# added FRTDP
#
# Revision 1.3  2006/02/13 19:09:16  trey
# added LRTDP
#
# Revision 1.2  2006/02/11 22:38:19  trey
# added RTDPCore
#
# Revision 1.1  2006/02/09 21:59:04  trey
# initial check-in
#
#
