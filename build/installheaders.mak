#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: installheaders.mak,v 1.4 2006-04-28 17:57:41 trey Exp $
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
# DESCRIPTION:  Used to install headers in the include directory.
#
# Input variables:
#
# INSTALLHEADERS_HEADERS
#   Header files to install (e.g. foo.h)

# Include this sub-makefile only once.

INSTALLHEADERS_SRCS := $(INSTALLHEADERS_HEADERS:%=$(THIS_DIR)/%)
INSTALLHEADERS_TARGETS += $(INSTALLHEADERS_HEADERS:%=$(TARGET_INC_DIR)/%)

# made obsolete by a new rule in footer.mak
#$(INSTALLHEADERS_TARGETS): $(INSTALLHEADERS_SRCS)
#	@[ -d $(TARGET_INC_DIR) ] || mkdir -p $(TARGET_INC_DIR)
#	$(INSTALLHEADERS_COPY) $(INSTALLHEADERS_SRCS) $(TARGET_INC_DIR)

# clear input variables
INSTALLHEADERS_HEADERS :=

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
# Revision 1.4  2004/07/06 15:11:27  trey
# installheaders.mak no longer clobbers INSTALLHEADERS_TARGETS; instead it appends to it
#
# Revision 1.3  2004/05/13 20:25:50  trey
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
# Revision 1.1  2003/10/05 02:59:35  trey
# initial microraptor check-in
#
#
