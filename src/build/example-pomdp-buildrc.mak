#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: example-pomdp-buildrc.mak,v 1.4 2006-04-28 17:57:41 trey Exp $
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

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.3  2005/10/28 03:34:41  trey
# switched to simpler license
#
# Revision 1.2  2005/10/28 02:24:42  trey
# added copyright headers
#
# Revision 1.1  2004/11/09 18:33:06  trey
# adjusted to new project name
#
# Revision 1.1  2004/05/21 19:16:58  trey
# branched example-buildrc.mak into two files, one for atacama and one for microraptor; added COMPILER_DIRECTORY_WITH_SLASH configuration variable
#
#
#
