#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.4 2005-01-17 19:36:48 trey Exp $
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

CFLAGS += -I$(CHECKOUT_DIR)/external/boost

INSTALLHEADERS_HEADERS := \
	commonDefs.h \
	commonTime.h \
	commonTypes.h \
	SmartRef.h \
	Interleave.h \
	MatrixUtils.h \
	PomdpM.h \
	PomdpSim.h \
	Solver.h \
	ValueFunction.h \
	MDPValueFunction.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libpomdpUtils.a
BUILDLIB_SRCS := \
	commonTypes.cc \
	Interleave.cc \
	MatrixUtils.cc \
	PomdpM.cc \
	PomdpSim.cc \
	ValueFunction.cc \
	MDPValueFunction.cc
include $(BUILD_DIR)/buildlib.mak

BUILDLIB_TARGET := libcommonTime.a
BUILDLIB_SRCS := commonTime.cc
include $(BUILD_DIR)/buildlib.mak

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
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
