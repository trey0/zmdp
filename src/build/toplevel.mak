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
