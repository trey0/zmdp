#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.20 2006-02-17 18:38:46 trey Exp $
#
# Copyright (c) 2002-2005, Trey Smith
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * The software may not be sold or incorporated into a commercial
#   product without specific prior written permission.
# * The above copyright notice and this permission notice shall be
#   included in all copies or substantial portions of the software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

######################################################################
# DO NOT MODIFY THIS HEADER SECTION

CHECKOUT_DIR := $(shell perl -e '$$_ = `pwd`; chop; $$_ = scalar reverse("$$_/"); if (s:.*/crs/::) { print scalar reverse($$_); } else { print "<error>"; die "*** could not calculate CHECKOUT_DIR ***\n"; }')
include $(CHECKOUT_DIR)/src/build/header.mak

######################################################################
# PLACE MODIFICATIONS BELOW THIS LINE

include $(SRC_DIR)/common/options.mak

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS :=

BUILDBIN_TARGET := solvePomdpHsvi
BUILDBIN_SRCS := solvePomdpHsvi.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpHsvi -lzmdpPomdpCore -lzmdpPomdpParser -lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := solvePomdpLrtdp
BUILDBIN_SRCS := solvePomdpLrtdp.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpLrtdp -lzmdpPomdpCore -lzmdpPomdpParser -lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := solvePomdpFrtdp
BUILDBIN_SRCS := solvePomdpFrtdp.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpFrtdp -lzmdpPomdpCore -lzmdpPomdpParser -lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

#BUILDBIN_TARGET := solvePomdpGhsvi
#BUILDBIN_SRCS := solvePomdpGhsvi.cc
#BUILDBIN_INDEP_LIBS :=
#BUILDBIN_DEP_LIBS := -lzmdpGhsvi -lzmdpPomdpCore -lzmdpPomdpParser -lzmdpCommon
#include $(BUILD_DIR)/buildbin.mak

#BUILDBIN_TARGET := solvePomdpPAOStar
#BUILDBIN_SRCS := solvePomdpPAOStar.cc
#BUILDBIN_INDEP_LIBS :=
#BUILDBIN_DEP_LIBS := -lzmdpPAOStar -lzmdpPomdpCore -lzmdpPomdpParser -lzmdpCommon
#include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := solveRaceTrackRtdp
BUILDBIN_SRCS := solveRaceTrackRtdp.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpRtdp -lzmdpRelaxBound -lzmdpRaceTrack -lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := solveRaceTrackLrtdp
BUILDBIN_SRCS := solveRaceTrackLrtdp.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpLrtdp -lzmdpRelaxBound -lzmdpRaceTrack -lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := solveRaceTrackHdp
BUILDBIN_SRCS := solveRaceTrackHdp.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpHdp -lzmdpRelaxBound -lzmdpRaceTrack -lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

BUILDBIN_TARGET := solveRaceTrackFrtdp
BUILDBIN_SRCS := solveRaceTrackFrtdp.cc
BUILDBIN_INDEP_LIBS :=
BUILDBIN_DEP_LIBS := -lzmdpFrtdp -lzmdpRelaxBound -lzmdpRaceTrack -lzmdpCommon
include $(BUILD_DIR)/buildbin.mak

#BUILDBIN_TARGET := solveRaceTrackPAOStar
#BUILDBIN_SRCS := solveRaceTrackPAOStar.cc
#BUILDBIN_INDEP_LIBS :=
#BUILDBIN_DEP_LIBS := -lzmdpPAOStar -lzmdpRelaxBound -lzmdpRaceTrack -lzmdpCommon
#include $(BUILD_DIR)/buildbin.mak

#BUILDBIN_TARGET := solveRaceTrackGhsvi
#BUILDBIN_SRCS := solveRaceTrackGhsvi.cc
#BUILDBIN_INDEP_LIBS :=
#BUILDBIN_DEP_LIBS := -lzmdpGhsvi -lzmdpRelaxBound -lzmdpRaceTrack -lzmdpCommon
#include $(BUILD_DIR)/buildbin.mak

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
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
