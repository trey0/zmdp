#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.3 2005-01-17 19:37:14 trey Exp $
#
# PROJECT:      FIRE Architecture Project
# DESCRIPTION:  

######################################################################
# DO NOT MODIFY THIS HEADER SECTION

CHECKOUT_DIR := $(shell perl -e '$$_ = `pwd`; chop; $$_ = scalar reverse("$$_/"); if (s:.*/crs/::) { print scalar reverse($$_); } else { print "<error>"; die "*** could not calculate CHECKOUT_DIR ***\n"; }')
include $(CHECKOUT_DIR)/src/build/header.mak

######################################################################
# PLACE MODIFICATIONS BELOW THIS LINE

include $(SRC_DIR)/common/options.mak

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS := 

INSTALLHEADERS_HEADERS := pomdp.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libpomdp_spec.a
BUILDLIB_SRCS := pomdp_spec.tab.cc pomdp_spec.yy.cc \
  imm-reward.c parse_err.c parse_hash.c \
  sparse-matrix.c mdp.c pomdp.cc
include $(BUILD_DIR)/buildlib.mak

# use 'gmake TEST=1 install' to build the following stuff
ifneq (,$(TEST))

BUILDBIN_TARGET := testSpec
BUILDBIN_SRCS := testSpec.cc
BUILDBIN_INDEP_LIBS := 
BUILDBIN_DEP_LIBS := -lpomdp_spec
include $(BUILD_DIR)/buildbin.mak

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
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
