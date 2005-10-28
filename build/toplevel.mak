#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: toplevel.mak,v 1.3 2005-10-28 03:34:41 trey Exp $
#
# Copyright (c) 1996-2005, Carnegie Mellon University
# All rights reserved.
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
# SPECIAL RULES FOR THE TOP-LEVEL MAKEFILE

# toplevel-pre-install can be used to do something before compilation starts
#   if 'gmake install' is run at the top level (e.g. build xdrgen)

# toplevel-pre-install:
#	$(MAKE) $(MFLAGS) -fMakefile -C XDRGen DEPEND=1 install

# toplevel-uninstall can be used to do some special cleaning if
#   'gmake uninstall' or 'gmake realclean' is done at the top level
#   (e.g. uninstall xdrgen, remove lib and bin target directories)

toplevel-uninstall:
	rm -rf $(TARGET_CLEAN_DIRS)
#	$(MAKE) $(MFLAGS) -fMakefile -C XDRGen DEPEND=1 realclean

.PHONY: doc
doc:
	$(BUILD_DIR)/doxygenRecurse.perl

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.2  2005/10/28 02:24:42  trey
# added copyright headers
#
# Revision 1.1.1.1  2004/11/09 16:18:57  trey
# imported hsvi into new repository
#
# Revision 1.2  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.1  2003/10/05 16:51:41  trey
# initial atacama check-in
#
# Revision 1.1  2003/10/05 02:59:35  trey
# initial microraptor check-in
#
# Revision 1.2  2001/11/19 16:01:03  trey
# cosmetic changes
#
# Revision 1.1.1.1  2001/08/27 15:46:52  trey
# initial check-in
#
# Revision 1.2  2001/03/06 17:48:22  trey
# set up auto-generation of hierarchical directory of docs
#
# Revision 1.1  2001/02/13 16:45:07  trey
# initial check-in
#
#
