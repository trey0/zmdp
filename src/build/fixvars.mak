#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: fixvars.mak,v 1.4 2006-04-28 17:57:41 trey Exp $
#
# Copyright (c) 1996-2005, Carnegie Mellon University. All rights reserved.
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
# Revision 1.3  2005/10/28 03:34:41  trey
# switched to simpler license
#
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
