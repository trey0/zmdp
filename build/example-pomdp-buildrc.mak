#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: example-pomdp-buildrc.mak,v 1.2 2005-10-28 02:24:42 trey Exp $
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
# Revision 1.1  2004/11/09 18:33:06  trey
# adjusted to new project name
#
# Revision 1.1  2004/05/21 19:16:58  trey
# branched example-buildrc.mak into two files, one for atacama and one for microraptor; added COMPILER_DIRECTORY_WITH_SLASH configuration variable
#
#
#
