# COPYRIGHT 2004, Carnegie Mellon University
#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.6 2005-01-28 03:16:08 trey Exp $
#
# PROJECT:      Life in the Atacama
# DESCRIPTION:  

######################################################################
# DO NOT MODIFY THIS HEADER SECTION

CHECKOUT_DIR := $(shell perl -e '$$_ = `pwd`; chop; $$_ = scalar reverse("$$_/"); if (s:.*/crs/::) { print scalar reverse($$_); } else { print "<error>"; die "*** could not calculate CHECKOUT_DIR ***\n"; }')
include $(CHECKOUT_DIR)/src/build/header.mak

######################################################################
# PLACE MODIFICATIONS BELOW THIS LINE

include $(SRC_DIR)/common/options.mak

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS = common spec main hsvi # empomdp

include $(BUILD_DIR)/toplevel.mak

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
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
