#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.2 2006-02-11 22:38:19 trey Exp $
#
# Copyright (c) 2006, Trey Smith. All rights reserved.
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

maininstall:
	$(MAKE) $(MFLAGS) -C .. -fMakefile SUBDIRS='rtdp main' install

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS :=

INSTALLHEADERS_HEADERS := \
	RTDPCore.h \
	RTDP.h
include $(BUILD_DIR)/installheaders.mak

BUILDLIB_TARGET := libzmdpRtdp.a
BUILDLIB_SRCS := RTDP.cc RTDPCore.cc
include $(BUILD_DIR)/buildlib.mak

#BUILDLIB_TARGET := libzmdpLrtdp.a
#BUILDLIB_SRCS := LRTDP.cc
#include $(BUILD_DIR)/buildlib.mak

ifneq (,$(TEST))

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.1  2006/02/09 21:59:04  trey
# initial check-in
#
#
