#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: buildjava.mak,v 1.1.1.1 2004-11-09 16:18:57 trey Exp $
#
# PROJECT:      FIRE Architecture Project
# DESCRIPTION:  Include to build a java class file.
#

# Input variables:
#
# BUILDJAVA_SRCS
#   Java source files to compile (e.g. foo.java)

BUILDJAVA_TARGETS += $(BUILDJAVA_SRCS:%.java=$(TARGET_JAVA_DIR)/%.class)

# clear input variables
BUILDJAVA_SRCS :=

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.4  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.3  2004/03/19 18:26:33  trey
# cleaned up some variables
#
# Revision 1.2  2004/03/19 14:50:40  trey
# major changes to support "gmake all" and other features atacama folks want
#
# Revision 1.1  2003/10/05 16:51:41  trey
# initial atacama check-in
#
# Revision 1.3  2003/10/05 15:40:11  trey
# renamed variables
#
# Revision 1.2  2003/10/05 15:32:58  trey
# now specify foo.java instead of foo when using buildjava.mak
#
# Revision 1.1  2003/10/05 02:59:35  trey
# initial microraptor check-in
#
# Revision 1.1  2002/04/25 22:13:40  trey
# added java rules; removed xdrgen target now that the system can auto-detect when xdrgen needs to be run
#
#
