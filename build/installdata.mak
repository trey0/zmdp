#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: installdata.mak,v 1.2 2005-10-28 02:24:42 trey Exp $
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
#
######################################################################
# DESCRIPTION:  Include to install a file in the data directory.
#
# Input variables:
#
# INSTALLDATA_FILES
#   Files to install (e.g. foo.config)
# INSTALLDATA_DIR
#   Directory to put files in (e.g. $(HOME))

INSTALLDATA_SRCS := $(INSTALLDATA_FILES:%=$(THIS_DIR)/%)
INSTALLDATA_TARGETS := $(INSTALLDATA_FILES:%=$(INSTALLDATA_DIR)/%)

# the installdata target depends on $(INSTALL_DATA_TARGETS)
INSTALL_DATA_TARGETS += $(INSTALLDATA_TARGETS)

_LT := $(word 1,$(INSTALLDATA_SRCS))
$(_LT)_DIR := $(INSTALLDATA_DIR)

$(INSTALLDATA_TARGETS): $(INSTALLDATA_SRCS)
	@[ -d $($<_DIR) ] || mkdir -p $($<_DIR)
	$(INSTALLDATA_COPY) $? $($<_DIR)

# clear input variables
INSTALLDATA_FILES :=
INSTALLDATA_DIR :=

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.1.1.1  2004/11/09 16:18:57  trey
# imported hsvi into new repository
#
# Revision 1.3  2004/06/25 20:22:32  trey
# propagated changes from atacama
#
# Revision 1.4  2004/06/25 20:16:20  trey
# fixed problem that cropped up when installing multiple data files
#
# Revision 1.3  2004/04/21 20:54:10  trey
# fixed behavior for multiple data files
#
# Revision 1.2  2004/03/19 18:26:33  trey
# cleaned up some variables
#
# Revision 1.1  2003/10/05 16:51:41  trey
# initial atacama check-in
#
# Revision 1.1  2003/10/05 02:59:35  trey
# initial microraptor check-in
#
# Revision 1.1.1.1  2001/08/27 15:46:51  trey
# initial check-in
#
# Revision 1.4  2000/02/14 20:34:36  trey
# fixed afs-related problems
#
# Revision 1.3  1999/11/11 15:08:23  trey
# now uses diraenv environment variables; using installdata is also streamlined
#
# Revision 1.2  1999/11/08 15:43:01  trey
# added ability to specify subdirectory of software/data to install to
#
# Revision 1.1  1999/10/27 18:41:08  trey
# Makefile system overhaul after talking to Reid
#
# Revision 1.1.1.1  1999/10/27 02:48:58  trey
# Imported sources
#
#
