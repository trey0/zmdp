#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: toplevel.mak,v 1.2 2005-10-28 02:24:42 trey Exp $
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
