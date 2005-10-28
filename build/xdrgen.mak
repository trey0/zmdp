#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: xdrgen.mak,v 1.2 2005-10-28 02:24:42 trey Exp $
#
# Copyright (c) 1996-2005, Carnegie Mellon University
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
# * Neither the name of Carnegie Mellon University nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
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
