#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: xdrgen.mak,v 1.3 2005-10-28 03:34:41 trey Exp $
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
# DESCRIPTION:  Include to generate a header from an XDR spec using xdrgen
#

# Input variables:
#
# XDRGEN_XDRGS
#   Files to run xdrgen over (e.g. foo.xdr)

XDRGEN_LOCALS += $(XDRGEN_XDRS:%=$(TARGET_INC_DIR)/%.xdr.h)
XDRGEN_TARGETS += $(XDRGEN_XDRS:%=$(LOCAL_INC_DIR)/%.xdr.h)

# clear input variables
XDRGEN_XDRS :=

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
# Revision 1.3  2004/03/19 18:26:33  trey
# cleaned up some variables
#
# Revision 1.2  2004/03/19 14:50:40  trey
# major changes to support "gmake all" and other features atacama folks want
#
# Revision 1.1  2003/10/05 16:51:41  trey
# initial atacama check-in
#
# Revision 1.1  2003/10/05 02:59:36  trey
# initial microraptor check-in
#
# Revision 1.3  2002/04/26 17:25:48  trey
# re-added explicit capability to build *.xdr.h files for uncommon cases when auto-detect fails
#
# Revision 1.1  2001/08/27 15:46:51  trey
# Initial revision
#
# Revision 1.1  2001/02/06 02:03:13  trey
# added rules for generating header files from XDR specs
#
#
#
