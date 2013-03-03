#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.22 2007-03-23 00:14:54 trey Exp $
#
# Copyright (c) 2002-2005, Trey Smith.  All rights reserved.
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
#
######################################################################
# DO NOT MODIFY THIS HEADER SECTION

CHECKOUT_DIR := $(shell perl -e '$$_ = `pwd`; chop; $$_ = scalar reverse("$$_/"); if (s:.*/crs/::) { print scalar reverse($$_); } else { print "<error>"; die "*** could not calculate CHECKOUT_DIR ***\n"; }')
include $(CHECKOUT_DIR)/src/build/header.mak

######################################################################
# PLACE MODIFICATIONS BELOW THIS LINE

include $(SRC_DIR)/common/options.mak

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS = common parsers mdps bounds search exec pomdpCore pomdpBounds pomdpModels main

include $(BUILD_DIR)/toplevel.mak

test:
	cd tests && ./testAll

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.21  2006/11/08 18:04:07  trey
# renamed pomdpParser/ directory to parsers/
#
# Revision 1.20  2006/10/25 02:12:22  trey
# added "test" target
#
# Revision 1.19  2006/07/26 21:04:17  trey
# added build in pomdpModels
#
# Revision 1.18  2006/06/15 16:07:49  trey
# added pomdpExec
#
# Revision 1.17  2006/04/28 17:57:41  trey
# changed to use apache license
#
# Revision 1.16  2006/04/27 23:29:35  trey
# renamed rtdp -> search
#
# Revision 1.15  2006/04/05 21:37:11  trey
# removed hsvi2 directory, added bounds and pomdpBounds
#
# Revision 1.14  2006/02/27 20:14:15  trey
# removed ghsvi directory
#
# Revision 1.13  2006/02/10 19:32:34  trey
# added rtdp directory
#
# Revision 1.12  2006/02/01 18:03:34  trey
# added mdps directory
#
# Revision 1.11  2006/01/31 20:27:22  trey
# added/renamed subdirectories
#
# Revision 1.10  2005/11/28 20:56:48  trey
# added ghsvi directory
#
# Revision 1.9  2005/10/28 22:04:21  trey
# renamed hsvi to hsvi2
#
# Revision 1.8  2005/10/28 03:56:13  trey
# simplified license
#
# Revision 1.7  2005/10/28 02:57:00  trey
# added copyright header
#
# Revision 1.6  2005/01/28 03:16:08  trey
# removed empomdp from SUBDIRS
#
# Revision 1.5  2005/01/17 19:36:40  trey
# added include of options.mak
#
# Revision 1.4  2004/12/16 03:59:24  trey
# added hsvi back into the build sequence
#
# Revision 1.3  2004/11/15 23:44:17  trey
# included empomdp in default build
#
# Revision 1.2  2004/11/13 23:48:54  trey
# rearranged testPomdp into the main directory
#
# Revision 1.1  2004/11/09 21:31:36  trey
# initial check-in
#
# Revision 1.12  2004/08/17 03:05:30  trey
# added limited build targets
#
# Revision 1.11  2004/04/28 17:15:20  dom
# Added header with copyright information to source and Makefiles.
#
# Revision 1.10  2003/11/18 23:16:27  trey
# checkout dir inference is now simpler and has better error checking
#
# Revision 1.9  2003/10/15 20:16:57  brennan
# Committing changes made during install on seamonkey.
#
# Revision 1.8  2003/10/05 02:56:47  trey
# switched to variant of FIRE makefile system
#
#
