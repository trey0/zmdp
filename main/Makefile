#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.39 2007-07-08 20:59:29 trey Exp $
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

maininstall: install

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS :=

INSTALLHEADERS_HEADERS := \
	zmdpMainConfig.h
include $(BUILD_DIR)/installheaders.mak

MAIN_LIBS := \
	-lzmdpSearch \
	-lzmdpPomdpCore \
	-lzmdpPomdpBounds \
	-lzmdpPomdpParser \
	-lzmdpBounds \
	-lzmdpRaceTrack \
	-lzmdpCommon

EMBEDFILES_TARGET := zmdpMainConfig.cc
EMBEDFILES_SRC := zmdpMainConfig.h
include $(BUILD_DIR)/embedfiles.mak

BUILDBIN_TARGET := zmdp
BUILDBIN_SRCS := zmdp.cc TestDriver.cc solverUtils.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpLifeSurvey -lzmdpExec $(MAIN_LIBS)
include $(BUILD_DIR)/buildbin.mak

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.38  2007/03/23 02:20:41  trey
# fixed to reflect name change -lzmdpPomdpExec -> -lzmdpExec
#
# Revision 1.37  2007/03/06 17:56:22  trey
# no longer need to create a link to generated file zmdpMainConfig.cc in the target include directory, because the Makefile system has been modified to put it there in the first place
#
# Revision 1.36  2007/03/05 08:56:59  trey
# modified how zmdpMainConfig is used so binaries built in other directories can embed the default config
#
# Revision 1.35  2006/10/16 17:32:17  trey
# switched zmdpEvaluate to use new config system
#
# Revision 1.34  2006/10/16 05:51:07  trey
# added zmdpMainConfig.cc
#
# Revision 1.33  2006/07/10 03:35:57  trey
# added zmdpEvaluate
#
# Revision 1.32  2006/04/28 22:33:36  trey
# renamed zmdpRtdp -> zmdpSearch
#
# Revision 1.31  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.30  2006/04/27 23:19:30  trey
# added TestDriver.cc to zmdpBenchmark sources
#
# Revision 1.29  2006/04/27 23:07:12  trey
# added zmdpSolve
#
# Revision 1.28  2006/04/27 20:20:51  trey
# renamed solveMDP to zmdpBenchmark; moved some code to solverUtils.cc
#
# Revision 1.27  2006/04/07 19:40:41  trey
# switched back to a unified binary for all algorithms, turns out it cuts down on code maintenance
#
# Revision 1.26  2006/04/05 21:38:27  trey
# temporarily stopped maintaining hsvi
#
# Revision 1.25  2006/04/04 17:28:38  trey
# added dependency on -lzmdpBounds; no longer maintaining ARTDP
#
# Revision 1.24  2006/03/17 20:18:16  trey
# added solvePomdpARTDP
#
# Revision 1.23  2006/03/17 20:07:57  trey
# added solveRaceTrackARTDP
#
# Revision 1.22  2006/02/27 20:16:10  trey
# removed references to obsolete experimental algorithms
#
# Revision 1.21  2006/02/19 22:20:44  trey
# added solvePomdpHdp
#
# Revision 1.20  2006/02/17 18:38:46  trey
# added several new binaries
#
# Revision 1.19  2006/02/13 21:49:14  trey
# added solveRaceTrackFrtdp
#
# Revision 1.18  2006/02/13 19:07:55  trey
# added solveRaceTrackLrtdp
#
# Revision 1.17  2006/02/09 21:58:29  trey
# added solveRaceTrackRtdp
#
# Revision 1.16  2006/02/08 20:29:03  trey
# now link against RelaxBound library
#
# Revision 1.15  2006/02/06 19:31:43  trey
# added more solveXXX builds
#
# Revision 1.14  2006/02/01 18:01:47  trey
# added solveRaceTrackPAOStar
#
# Revision 1.13  2006/01/31 20:26:21  trey
# refactored/renamed libs
#
# Revision 1.12  2006/01/31 18:14:04  trey
# added solvePomdpGhsvi
#
# Revision 1.11  2006/01/30 19:26:40  trey
# replaced generic testPomdp with separate binaries for different problem types and algorithms
#
# Revision 1.10  2005/11/02 21:03:47  trey
# removed cruft and added ghsvi
#
# Revision 1.9  2005/10/28 03:53:08  trey
# simplified license
#
# Revision 1.8  2005/10/28 02:59:30  trey
# added copyright header
#
# Revision 1.7  2005/10/21 20:23:55  trey
# renamed commonTime to avoid conflicts with other projects
#
# Revision 1.6  2005/02/06 16:42:44  trey
# made linkage against -lhsvi independent of USE_CPLEX flag
#
# Revision 1.5  2005/01/17 19:37:14  trey
# added include of options.mak
#
# Revision 1.4  2004/12/16 04:00:40  trey
# removed USER_CFLAGS, can now turn on cplex use with USE_CPLEX=1
#
# Revision 1.3  2004/11/15 23:43:03  trey
# there are now distinct binaries testPomdp and testPomdpWithCplex
#
# Revision 1.2  2004/11/14 00:31:47  trey
# now select algorithm to run from command line
#
# Revision 1.1  2004/11/13 23:48:54  trey
# rearranged testPomdp into the main directory
#
# Revision 1.3  2004/11/13 23:29:44  trey
# moved many files from hsvi to common
#
# Revision 1.2  2004/11/09 21:31:59  trey
# got pomdp source tree into a building state again
#
# Revision 1.1.1.1  2004/11/09 16:18:56  trey
# imported hsvi into new repository
#
# Revision 1.12  2003/09/22 18:48:13  trey
# made several algorithm configurations depend on makefile settings, added extra timing output
#
# Revision 1.11  2003/09/20 02:26:09  trey
# found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
#
# Revision 1.10  2003/09/17 18:50:35  trey
# added user quit command, which leaves a gprof profile, unlike Ctrl-C
#
# Revision 1.9  2003/09/16 00:57:01  trey
# lots of performance tuning, mostly fixed rising upper bound problem
#
# Revision 1.8  2003/09/11 01:46:42  trey
# completed conversion to compressed matrices
#
# Revision 1.7  2003/09/07 02:28:07  trey
# started to adapt for boost matrix library
#
# Revision 1.6  2003/09/06 21:51:53  trey
# many changes
#
# Revision 1.5  2003/07/17 18:50:51  trey
# cleaned out some cruft
#
# Revision 1.4  2003/07/16 16:08:51  trey
# added support for CPLEX and Interleave
#
# Revision 1.3  2003/06/26 15:41:19  trey
# C++ version of pomdp solver functional
#
# Revision 1.2  2003/01/08 19:57:39  trey
# seems to work on 4-state pomdp now
#
# Revision 1.1.1.1  2003/01/07 19:19:40  trey
# Imported sources
#
# Revision 1.12  2002/09/25 16:20:42  trey
# put in updated rules for DW version of genWorld and viewWorld
#
# Revision 1.11  2002/09/25 16:15:04  trey
# defined sr library and rules for SR version of genWorld and viewWorld
#
# Revision 1.10  2002/09/19 15:18:00  trey
# added SR stuff
#
# Revision 1.9  2002/08/26 20:47:04  trey
# added text display to SimpleImage (involved recoding almost from scratch)
#
# Revision 1.8  2002/08/16 17:44:24  trey
# started adding config file support
#
# Revision 1.7  2002/07/04 14:33:09  trey
# greedy2 overhauled to allow interleaved em and execution, new apps added
#
# Revision 1.6  2002/06/26 20:41:34  trey
# massive improvements to EM, first experiments run
#
# Revision 1.5  2002/05/30 18:07:56  trey
# added basic EM algorithm for inferring cluster parameters
#
# Revision 1.4  2002/05/22 20:23:40  trey
# discrete world now works
#
# Revision 1.3  2002/05/21 03:25:48  trey
# almost ready for discrete sim to work
#
# Revision 1.2  2002/05/16 23:27:15  trey
# major overhaul
#
# Revision 1.1  2002/04/30 14:26:21  trey
# initial check-in
#
# Revision 1.1  2001/10/31 21:43:24  trey
#
