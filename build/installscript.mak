#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: installscript.mak,v 1.1.1.1 2004-11-09 16:18:57 trey Exp $
#
# PROJECT:      FIRE Architecture Project
# DESCRIPTION:  Include to install a script in the common binary dir
#

# Input variables:
#
# INSTALLSCRIPT_FILES
#   Script files to install (e.g. foo.perl)

INSTALL_DATA_TARGETS += $(INSTALLSCRIPT_FILES:%=$(TARGET_COMMON_BIN_DIR)/%)

# clear input variables
INSTALLSCRIPT_FILES :=

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.3  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.2  2004/03/19 18:26:33  trey
# cleaned up some variables
#
# Revision 1.1  2003/10/05 16:51:41  trey
# initial atacama check-in
#
# Revision 1.2  2003/10/05 15:40:11  trey
# renamed variables
#
# Revision 1.1  2003/10/05 02:59:35  trey
# initial microraptor check-in
#
# Revision 1.1.1.1  2001/08/27 15:46:51  trey
# initial check-in
#
# Revision 1.1  2000/02/25 22:11:47  trey
# initial check-in
#
#
