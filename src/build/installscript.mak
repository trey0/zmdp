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
# DESCRIPTION:  Include to install a script in the common binary dir
#
# Input variables:
#
# INSTALLSCRIPT_FILES
#   Script files to install (e.g. foo.perl)

INSTALL_DATA_TARGETS += $(INSTALLSCRIPT_FILES:%=$(TARGET_COMMON_BIN_DIR)/%)

# clear input variables
INSTALLSCRIPT_FILES :=
