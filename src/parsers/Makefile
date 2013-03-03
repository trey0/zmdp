#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.11 2006-11-09 20:49:08 trey Exp $
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
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='parsers main' install

INSTALLHEADERS_HEADERS := \
	sparse-matrix.h \
	CassandraModel.h \
	CassandraParser.h \
	FastParser.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpPomdpParser.a
BUILDLIB_SRCS := pomdp_spec.tab.cc pomdp_spec.yy.cc \
  imm-reward.c decision-tree.c parse_err.c parse_hash.c \
  sparse-matrix.c mdp.c \
  CassandraModel.cc \
  CassandraParser.cc \
  FastParser.cc
include $(BUILD_DIR)/buildlib.mak

# use 'gmake TEST=1 install' to build the following stuff
ifneq (,$(TEST))

BUILDBIN_TARGET := testDecisionTree
BUILDBIN_SRCS := testDecisionTree.c decision-tree.c
BUILDBIN_INDEP_LIBS := 
BUILDBIN_DEP_LIBS :=
include $(BUILD_DIR)/buildbin.mak

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.10  2006/11/08 16:42:06  trey
# added CassandraModel, CassandraParser, and FastParser
#
# Revision 1.9  2006/05/29 05:03:51  trey
# added decision-tree.c and testDecisionTree.c; removed testSpec.cc
#
# Revision 1.8  2006/05/27 19:04:32  trey
# now export sparse-matrix because conversion from Cassandra matrix format to sla is performed by code in src/pomdpCore
#
# Revision 1.7  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.6  2006/01/31 20:26:43  trey
# renamed libpomdp_spec -> libzmdpPomdpParser
#
# Revision 1.5  2005/10/28 03:54:39  trey
# simplified license
#
# Revision 1.4  2005/10/27 21:36:58  trey
# renamed pomdp.{h,cc} to pomdpCassandraWrapper.{h,cc}
#
# Revision 1.3  2005/01/17 19:37:14  trey
# added include of options.mak
#
# Revision 1.2  2004/11/09 21:31:59  trey
# got pomdp source tree into a building state again
#
# Revision 1.1.1.1  2004/11/09 16:18:56  trey
# imported hsvi into new repository
#
# Revision 1.2  2003/04/02 17:13:54  trey
# minor changes to enable compilation under gcc 3.0.1
#
# Revision 1.1.1.1  2003/01/07 19:19:41  trey
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
