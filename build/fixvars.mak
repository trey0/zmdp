#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: fixvars.mak,v 1.3 2005-10-28 03:34:41 trey Exp $
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
# DESCRIPTION:  Enables backwards compatibility with some old variable names

# fold old distinct variables (if defined) into the new more generic variables

ifneq (,$(CPPFLAGS))
  CFLAGS += $(CPPFLAGS)
  CPPFLAGS :=
endif

ifneq (,$(INC_DIRS))
  CFLAGS += $(INC_DIRS)
  INC_DIRS :=
endif

ifneq (,$(LIB_DIRS))
  LDFLAGS += $(LIB_DIRS)
  LIB_DIRS :=
endif

ifneq (,$(COMFLAGS))
  CFLAGS += $(COMFLAGS)
  LDFLAGS += $(COMFLAGS)
  COM_FLAGS :=
endif

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.2  2005/10/28 02:24:42  trey
# added copyright headers
#
# Revision 1.1.1.1  2004/11/09 16:18:57  trey
# imported hsvi into new repository
#
# Revision 1.1  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.1  2004/03/19 19:43:25  trey
# initial check-in, are you happy, dom?
#
#
