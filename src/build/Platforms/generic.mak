#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: generic.mak,v 1.4 2006-04-28 17:57:41 trey Exp $
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
# DESCRIPTION: Standard names and options for tools; these can be
#              overridden in the specific sub-makefile for a given
#              platform, e.g. linux2.mak

# by default, use whatever is in the user's path
COMPILER_DIRECTORY_WITH_SLASH :=

# note: we are using '=' instead of ':=' here so the user can
#   override COMPILER_DIRECTORY_WITH_SLASH later and pick
#   up the changes
CC = $(COMPILER_DIRECTORY_WITH_SLASH)gcc
C++ = $(COMPILER_DIRECTORY_WITH_SLASH)g++
LD = $(C++)

ifndef USER_CFLAGS
  USER_CFLAGS := -g
endif

CFLAGS = -Wall -pipe $(USER_CFLAGS)
LDFLAGS = $(USER_CFLAGS)
LDFLAGS_START_WHOLE := -Wl,--whole-archive
LDFLAGS_END_WHOLE := -Wl,--no-whole-archive
MAKEDEP := $(C++) -MM -MG

AR := ar
ARFLAGS := cr
RANLIB := ranlib

INSTALL := install
SED := sed

# Use the GNU versions of lex and yacc.
# Assume they can be found in the users path.
BISON   := bison
FLEX    := flex

# Flex / Bison specifics
# -b -7 -Cr
LEXFLAGS  := -f -Ca
LEX       := $(FLEX)
LEX_PREFIX := yy

# xdrgen-related
XDRGEN    := $(FIRE_MEMBERS_DIR)/software/bin/$(HOST_OS)/xdrgen --directives=portable

# --debug --verbose
YFLAGS    := --defines
YACC      := $(BISON)

PERL      := perl

# java related
JAVA		  := gij
JAVA_FLAGS	  := 
JAVAC		  := javac
JAVAC_FLAGS       :=

# TDLC related
TDLC_HEADER_FLAGS := -2Hy
TDLC_SOURCE_FLAGS := -2Cy

# swig/python related
SWIG := swig
SWIG_FLAGS := -c++ -python -shadow -dnone
SWIG_LIB_DIR := /usr/lib/swig_lib

# doc building
DOXYGEN := doxygen

# do we install files by copying or linking? ('cp -p' or 'ln -sf')
LN := ln -sf
CP := install

INSTALLBINS_COPY := $(CP)
INSTALLLIBS_COPY := $(CP)

INSTALLHEADERS_COPY := $(LN)
INSTALLDATA_COPY := $(LN)
INSTALLSCRIPT_COPY := $(LN)

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
# Revision 1.9  2004/08/11 22:48:12  trey
# propagated updates from atacama
#
# Revision 1.7  2004/06/17 19:31:25  trey
# made USER_CFLAGS updates take effect if set after header.mak is included
#
# Revision 1.6  2004/06/11 20:01:44  trey
# integrated fixes from microraptor version
#
# Revision 1.6  2004/05/21 19:13:36  trey
# switched gcc/g++ location to be based on user path, made it easier to change the location
#
# Revision 1.5  2004/05/21 18:03:32  trey
# ok, this time it is really up to date
#
# Revision 1.4  2004/03/19 16:23:07  trey
# fixed some bugs i found with dom
#
# Revision 1.3  2003/12/01 04:01:29  trey
# added SWIG configuration
#
# Revision 1.2  2003/11/18 21:12:21  trey
# changed *.c compilation to use g++ instead of gcc, since this is the only thing that seems to work for parts of the mission exec; maybe a better approach will be found later
#
# Revision 1.1  2003/10/05 16:51:41  trey
# initial atacama check-in
#
# Revision 1.2  2003/10/05 15:33:48  trey
# added INSTALLSCRIPT_COPY
#
# Revision 1.1  2003/10/05 02:59:36  trey
# initial microraptor check-in
#
# Revision 1.10  2003/04/25 18:42:14  danig
# Modified to allow compilation on a system with linux kernel version
# 2.4 and gcc 2.95.3.
#
# Revision 1.9  2003/02/28 15:48:10  danig
# Removed optimization flags for compilation under anything but gcc 2.95.3.
#
# Revision 1.8  2003/02/17 22:57:20  trey
# make USE_RLOG on by default, and made it easier to override at the command line
#
# Revision 1.7  2002/05/21 23:01:18  danig
# Added the LDFLAGS_START_WHOLE and LDFLAGS_END_WHOLE variables for
# options that only apply to libraries whose whole contents are linked
# into the executable.  It may be that only the -Wl,--whole-archive and
# -Wl,--no-whole-archive options ever make sense here.
#
# Revision 1.6  2002/05/18 04:11:56  trey
# made USER_CFLAGS able to get rid of -O2 flag
#
# Revision 1.5  2002/05/15 20:36:51  trey
# changed default CFLAGS to add -g and remove -O2
#
# Revision 1.4  2002/04/25 22:13:41  trey
# added java rules; removed xdrgen target now that the system can auto-detect when xdrgen needs to be run
#
# Revision 1.3  2002/04/25 01:24:56  trey
# fixed bug that *.tdl.h files were not being automatically generated if another *.tdl file tried to include them
#
# Revision 1.2  2002/02/07 23:51:04  mroth
# Trey's fix
#
# Revision 1.1.1.1  2001/08/27 15:46:52  trey
# initial check-in
#
# Revision 1.10  2001/04/17 21:48:56  trey
# added DEFAULT_BODIES_CONFIG variable
#
# Revision 1.9  2001/03/06 04:01:39  trey
# added rules for documentation generation, gmake doc
#
# Revision 1.8  2001/02/13 16:33:35  trey
# switched include of headers under src to use "dir/foo.h" instead of <dir/foo.h>; dependency checking is now easier. made some corresponding makefile changes
#
# Revision 1.7  2001/02/06 02:03:13  trey
# added rules for generating header files from XDR specs
#
# Revision 1.6  2000/09/30 23:25:37  trey
# modified makefile system to put generated source and header files in the gen/ directory
#
# Revision 1.5  2000/09/20 22:39:56  trey
# added TDL definitions
#
# Revision 1.4  2000/09/08 17:03:23  trey
# added define for perl binary location
#
# Revision 1.3  1999/11/03 20:51:45  trey
# added support for flex/bison, tweaked other stuff
#
# Revision 1.2  1999/10/27 18:41:14  trey
# Makefile system overhaul after talking to Reid
#
# Revision 1.1.1.1  1999/10/27 02:48:58  trey
# Imported sources
#
#
