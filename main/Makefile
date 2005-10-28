#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.8 2005-10-28 02:59:30 trey Exp $
#
# Copyright (c) 2002-2005, Trey Smith
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * This code and derivatives may not be incorporated into commercial
#   products without specific prior written permission.
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * The name of Trey Smith may not be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

######################################################################
# DO NOT MODIFY THIS HEADER SECTION

CHECKOUT_DIR := $(shell perl -e '$$_ = `pwd`; chop; $$_ = scalar reverse("$$_/"); if (s:.*/crs/::) { print scalar reverse($$_); } else { print "<error>"; die "*** could not calculate CHECKOUT_DIR ***\n"; }')
include $(CHECKOUT_DIR)/src/build/header.mak

######################################################################
# PLACE MODIFICATIONS BELOW THIS LINE

include $(SRC_DIR)/common/options.mak

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS :=

# because facilities changed the version (sigh)
ifeq (1,0)
  GCC_DIR := /usr/local/libexec/gcc-3.0.1/bin
  CC  := $(GCC_DIR)/gcc
  C++ := $(GCC_DIR)/g++
  LD  := $(GCC_DIR)/g++
  MAKEDEP := $(C++) -MM -MG
endif

#C++ := insure
#LD := insure

CFLAGS += -I$(EXTERNAL_DIR)/boost

# gprof
ifeq (1,0)
  CFLAGS += -pg
  LDFLAGS += -pg
endif

BINARY_NAME := testPomdp

ifneq (,$(USE_CPLEX))
  # use cplex
  BINARY_NAME := testPomdpWithCplex
  CPLEX_DIR := $(EXTERNAL_DIR)/ilog/cplex81
  CFLAGS += -DUSE_CPLEX=1 -I$(CPLEX_DIR)/include
  LDFLAGS += \
	-Wl,-rpath -Wl,$(CPLEX_DIR)/bin/i86_linux2_glibc2.2_gcc3.0 \
	             -L$(CPLEX_DIR)/bin/i86_linux2_glibc2.2_gcc3.0 \
	-Wl,-rpath -Wl,$(EXTERNAL_DIR)/stdc++ \
	             -L$(EXTERNAL_DIR)/stdc++
  LP_LIBS := -lcplex81 -lpthread
endif

BUILDBIN_TARGET := $(BINARY_NAME)
BUILDBIN_SRCS := testPomdp.cc
BUILDBIN_INDEP_LIBS := -lpomdpCommonTime $(LP_LIBS)
BUILDBIN_DEP_LIBS := -lhsvi -lpomdpUtils -lpomdp_spec
include $(BUILD_DIR)/buildbin.mak

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
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
