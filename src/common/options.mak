#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: options.mak,v 1.73 2007-04-19 22:22:15 trey Exp $
#
# Copyright (c) 2002-2006, Trey Smith. All rights reserved.
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

CFLAGS += -DZMDP_VERSION=1.1.7

# algorithm configuration options (most of the options that used to be
# in this file are now run-time configuration parameters; see
# src/main/zmdp.config)

#CFLAGS += -DUSE_HSVI_ADAPTIVE_DEPTH=1

# debug/optimization options

USER_CFLAGS := -O3
#USER_CFLAGS := -g

XCFLAGS := $(CFLAGS)
CFLAGS += -DCFLAGS="\"$(XCFLAGS)\""

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.72  2007/04/09 00:19:12  trey
# updated version number and release notes
#
# Revision 1.71  2007/03/25 18:36:46  trey
# updated version number and release notes
#
# Revision 1.70  2007/03/24 23:17:51  trey
# updated version number and release notes
#
# Revision 1.69  2007/02/22 22:06:54  trey
# updated release notes for v1.0,2
#
# Revision 1.68  2006/11/12 21:24:55  trey
# updated version and release notes
#
# Revision 1.67  2006/11/09 21:15:43  trey
# updated version number
#
# Revision 1.66  2006/11/07 20:17:17  trey
# updated version number
#
# Revision 1.65  2006/11/01 00:21:10  trey
# udpated version number
#
# Revision 1.64  2006/10/30 20:00:15  trey
# USE_DEBUG_PRINT replaced with a run-time config parameter "debugLevel"
#
# Revision 1.63  2006/10/27 19:42:00  trey
# updated version number
#
# Revision 1.62  2006/10/25 19:12:01  trey
# moved some flags to zmdp.config
#
# Revision 1.61  2006/10/25 02:44:45  trey
# updated version number
#
# Revision 1.60  2006/10/18 18:56:44  trey
# USE_CONVEX_SUPPORT_LIST is now a run-time config parameter
#
# Revision 1.59  2006/10/18 18:05:38  trey
# USE_TIME_WITHOUT_HEURISTIC is now a run-time config parameter
#
# Revision 1.58  2006/10/17 19:29:16  trey
# updated version number
#
# Revision 1.57  2006/10/03 03:26:33  trey
# updated version number
#
# Revision 1.56  2006/09/21 17:31:05  trey
# bumped version number
#
# Revision 1.55  2006/09/19 01:48:40  trey
# updated version number
#
# Revision 1.54  2006/08/08 21:18:01  trey
# added USE_REF_COUNT_PRUNE
#
# Revision 1.53  2006/07/26 20:59:31  trey
# updated for 0.5.2
#
# Revision 1.52  2006/07/26 20:53:09  trey
# added USE_CONVEX_CACHE
#
# Revision 1.51  2006/07/24 17:06:00  trey
# added USE_CONVEX_SUPPORT_LIST
#
# Revision 1.50  2006/07/04 23:23:42  trey
# updated for version 0.5.1 (this should have been committed before)
#
# Revision 1.49  2006/06/16 02:40:42  trey
# updated version number
#
# Revision 1.48  2006/06/05 20:17:22  trey
# updated version number and release notes
#
# Revision 1.47  2006/06/01 17:01:31  trey
# updated for v0.4.0
#
# Revision 1.46  2006/04/28 21:11:26  trey
# changed numbering scheme, 0.3 is now 0.3.0
#
# Revision 1.45  2006/04/28 18:53:01  trey
# removed many obsolete CFLAGS
#
# Revision 1.44  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.43  2006/04/27 23:06:47  trey
# turned on USE_MASKED_ALPHA
#
# Revision 1.42  2006/04/27 20:18:43  trey
# renamed WRTDP flags to HSVI
#
# Revision 1.41  2006/04/12 19:26:56  trey
# added USE_WRTDP_ADAPTIVE_DEPTH flag
#
# Revision 1.40  2006/04/10 20:25:35  trey
# removed obsolete flags USE_HDP_LOWER_BOUND and USE_RTDPCORE_UB_ACTION
#
# Revision 1.39  2006/04/04 17:27:10  trey
# no longer using obsolete USE_FRTDP_ALT_PRIO flag
#
# Revision 1.38  2006/03/17 20:09:05  trey
# bumped up ZMDP_VERSION to correct level
#
# Revision 1.37  2006/02/27 20:16:29  trey
# turned off USE_HDP_LOWER_BOUND flag
#
# Revision 1.36  2006/02/20 02:03:42  trey
# added USE_HDP_LOWER_BOUND and RTDPCORE_UB_ACTION
#
# Revision 1.35  2006/02/19 18:31:02  trey
# added USE_FRTDP_ALT_PRIO flag
#
# Revision 1.34  2006/02/15 16:21:51  trey
# added USE_TIME_WITHOUT_HEURISTIC flag
#
# Revision 1.33  2006/02/08 19:22:14  trey
# added USE_DEBUG_PRINT flag
#
# Revision 1.32  2006/02/07 18:49:02  trey
# returned to more typical settings
#
# Revision 1.31  2006/02/06 19:29:52  trey
# added USE_OLD_ANYTIME flag
#
# Revision 1.30  2006/02/01 01:13:35  trey
# renamed ZPOMDP_VERSION flag -> ZMDP_VERSION
#
# Revision 1.29  2006/01/28 03:08:49  trey
# added USE_DF_IGNORE_UNC
#
# Revision 1.28  2005/12/18 21:20:39  trey
# added USE_DF_DEPTH_TERMINATION
#
# Revision 1.27  2005/12/17 19:08:49  trey
# added USE_NO_INTERNAL_PRIORITY and USE_DF_NON_MYOPIC
#
# Revision 1.26  2005/12/14 01:34:06  trey
# added several new flags for PAOStar
#
# Revision 1.25  2005/12/12 15:47:25  trey
# added USE_ACCUM_SUM
#
# Revision 1.24  2005/12/09 17:51:50  trey
# added USE_DEPTH_FIRST
#
# Revision 1.23  2005/12/06 20:28:56  trey
# added three new flags
#
# Revision 1.22  2005/11/28 20:46:16  trey
# added ALT_ACTION_SELECTION and USE_EXPLORE_LOWER options
#
# Revision 1.21  2005/11/11 04:36:04  trey
# added USE_KD_NORMAL
#
# Revision 1.20  2005/11/10 22:07:08  trey
# added USE_KD_BOUNDS
#
# Revision 1.19  2005/11/08 18:14:48  trey
# added USE_RS_NATIVE, USE_BOUNDS_TERMINATION, USE_NO_GENERALIZATION
#
# Revision 1.18  2005/11/02 21:03:09  trey
# added new flags
#
# Revision 1.17  2005/10/28 22:18:29  trey
# added ZPOMDP_VERSION to CFLAGS
#
# Revision 1.16  2005/10/28 03:50:32  trey
# simplified license
#
# Revision 1.15  2005/10/28 02:51:40  trey
# added copyright headers
#
# Revision 1.14  2005/10/21 20:04:59  trey
# added some flags related to exec heuristic experiments
#
# Revision 1.13  2005/03/28 18:13:38  trey
# turned off USE_MASKED_ALPHA for now; EnduranceCrater problem seems to have exposed a bug
#
# Revision 1.12  2005/03/25 21:42:30  trey
# added USE_SAFE_HSVI
#
# Revision 1.11  2005/03/25 16:10:04  trey
# added NO_FIB_UPPER_BOUND and USE_UNIFORM_RANDOM_OBS
#
# Revision 1.10  2005/03/11 19:25:35  trey
# added USE_UB_HASH
#
# Revision 1.9  2005/03/10 21:12:34  trey
# added USE_MASKED_ALPHA
#
# Revision 1.8  2005/02/09 20:44:15  trey
# turned off USE_RANDOM_OBS and USE_MORE_UB_BACKUPS by default
#
# Revision 1.7  2005/02/08 23:54:59  trey
# added USE_COMPRESSED_ALPHA, USE_RANDOM_OBS, and USE_MORE_UB_BACKUPS options
#
# Revision 1.6  2005/02/06 16:40:10  trey
# added USE_EMPOMDP option, changed USE_CPLEX to be off by default
#
# Revision 1.5  2005/02/04 19:56:56  trey
# turned off ublas by default
#
# Revision 1.4  2005/01/28 03:23:37  trey
# moved USER_CFLAGS into options.mak
#
# Revision 1.3  2005/01/21 18:07:02  trey
# preparing for transition to sla matrix types
#
# Revision 1.2  2005/01/21 15:22:40  trey
# added DO_AGGRESSIVE_LB_PRUNE
#
# Revision 1.1  2005/01/17 19:37:32  trey
# initial check-in
#
#
