#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.31 2007-07-08 20:56:56 trey Exp $
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
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='common main' install

INSTALLHEADERS_HEADERS := \
	zmdpCommonDefs.h \
	zmdpCommonTime.h \
	zmdpConfig.h \
	sla.h \
	sla_mask.h \
	zmdpCommonTypes.h \
	slaMatrixUtils.h \
	MatrixUtils.h \
	MDPModel.h \
	MDPSim.h \
	Solver.h \
	embedFiles.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpCommon.a
BUILDLIB_SRCS := \
	zmdpCommonTypes.cc \
	zmdpCommonTime.cc \
	zmdpConfig.cc \
	MDPSim.cc
include $(BUILD_DIR)/buildlib.mak

ifneq (,$(TEST))

BUILDBIN_TARGET := test_sla
BUILDBIN_SRCS := test_sla.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS :=
include $(BUILD_DIR)/buildbin.mak

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.30  2007/03/24 22:38:31  trey
# removed AbstractSim.h
#
# Revision 1.29  2007/01/14 01:24:17  trey
# removed obsolete reference to boost
#
# Revision 1.28  2006/11/08 16:33:14  trey
# renamed MDP.h to MDPModel.h to avoid confusion with mdp.h, stupid case insensitive Mac OS X
#
# Revision 1.27  2006/10/15 23:44:35  trey
# now install zmdpConfig.{h,cc} and embedFiles.h
#
# Revision 1.26  2006/04/28 21:11:15  trey
# removed PQueue
#
# Revision 1.25  2006/04/28 20:33:11  trey
# moved common/Interleave -> main/TestDriver
#
# Revision 1.24  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.23  2006/04/04 17:27:38  trey
# removed build rules for files moved to src/bounds
#
# Revision 1.22  2006/02/08 19:22:01  trey
# added MDPCache.h and RelaxBound
#
# Revision 1.21  2006/02/01 01:09:37  trey
# renamed pomdp namespace -> zmdp
#
# Revision 1.20  2006/01/31 19:13:30  trey
# added AbstractBound.h
#
# Revision 1.19  2006/01/31 18:36:23  trey
# moved many files from common to pomdpCore
#
# Revision 1.18  2006/01/28 03:05:00  trey
# added MDP, MDPSim, AbstractSim
#
# Revision 1.17  2005/11/29 04:44:53  trey
# added install of PQueue.h
#
# Revision 1.16  2005/11/03 17:46:28  trey
# added BeliefMDP.h
#
# Revision 1.15  2005/10/28 03:50:32  trey
# simplified license
#
# Revision 1.14  2005/10/28 02:51:40  trey
# added copyright headers
#
# Revision 1.13  2005/10/27 22:32:24  trey
# removed pomdpSmartRef.h
#
# Revision 1.12  2005/10/27 22:09:53  trey
# renamed and removed some files
#
# Revision 1.11  2005/10/21 20:07:57  trey
# renamed some source files to avoid potential conflicts with other projects
#
# Revision 1.10  2005/03/25 16:09:12  trey
# added install of QMDP.h and MatrixUtils.h
#
# Revision 1.9  2005/03/10 21:12:04  trey
# commented out test_aadd
#
# Revision 1.8  2005/03/09 18:52:05  trey
# added build of test_aadd
#
# Revision 1.7  2005/01/26 04:17:15  trey
# added test_sla and testPomdpM, removed ublasMatrixUtils.cc from lib
#
# Revision 1.6  2005/01/21 18:07:02  trey
# preparing for transition to sla matrix types
#
# Revision 1.5  2005/01/21 16:26:17  trey
# added ublasMatrixTypes.h
#
# Revision 1.4  2005/01/17 19:36:48  trey
# added include of options.mak
#
# Revision 1.3  2004/11/24 20:48:29  trey
# added MDPValueFunction and ValueFunction
#
# Revision 1.2  2004/11/13 23:29:44  trey
# moved many files from hsvi to common
#
# Revision 1.1  2004/11/09 21:28:24  trey
# check-in to pomdp repository
#
# Revision 1.13  2002/10/09 00:53:00  trey
# added testIPCTimer
#
# Revision 1.12  2002/05/19 21:13:31  trey
# factored out duplicate code in templates
#
# Revision 1.11  2002/05/16 23:28:58  trey
# added optional build of test binary for IPCFileIO
#
# Revision 1.10  2002/05/15 21:01:00  danig
# Changed old MergeIntoMsgName.h, putting function definitions into
# MergeIntoMsgName.cc
# Added MergeIntoMsgName.cc to the common library.
#
# Revision 1.9  2002/05/15 18:41:37  vincent
# Uncommented the xdrgen related lines.  Although it should be obsolete,
# according to comment in makefile, I get build errors without this.
#
# Revision 1.8  2002/05/09 21:37:08  danig
# Added (and commented out) flags for profiling.
#
# Revision 1.7  2002/04/25 22:14:17  trey
# added example build of a java class
#
# Revision 1.6  2002/03/17 00:46:28  danig
# Added ltoa to the common library.
#
# Revision 1.5  2001/10/31 02:33:52  trey
# overhaul of signals and connections
#
# Revision 1.4  2001/10/30 05:34:36  trey
# added new test code
#
# Revision 1.3  2001/10/25 21:07:17  trey
# initial check-in
#
# Revision 1.2  2001/08/27 19:22:05  trey
# foo
#
# Revision 1.1  2001/08/27 18:49:39  trey
# initial check-in
#
#
