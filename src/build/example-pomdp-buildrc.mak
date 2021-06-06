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
# DESCRIPTION:  install this file as ~/.pomdp.buildrc to set up
#               your personal configuration
#
# you might want to specify the location of gcc/g++ (the default
# is to use your path)

# COMPILER_DIRECTORY_WITH_SLASH := /usr/local/bin/

# uncomment to recover old behavior that files are directly built
# to e.g. software/bin/linux2 instead of ./bin/linux2 (faster, saves space)

# SKIP_LOCALS := 1

# uncomment to get the behavior Dom likes, that the binaries are built
# in the current directory (actually, this builds them in ./bin/linux2
# and symlinks them to .)  note: not compatible with SKIP_LOCALS.

# PUT_MY_BINARIES_RIGHT_HERE_THANK_YOU_VERY_MUCH := 1
