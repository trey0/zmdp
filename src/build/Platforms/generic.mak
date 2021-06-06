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
LEXFLAGS  := -f -Ca --nounput
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
