#!gmake 	# Tell emacs about this file: -*- Makefile -*-
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
