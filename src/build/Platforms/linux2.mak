#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: linux2.mak,v 1.4 2006-04-28 17:57:41 trey Exp $
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

#######################################################################
# DESCRIPTION:  Variable values specific to Linux

CFLAGS += -DLINUX

PERL := /usr/bin/perl

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
# Revision 1.1  2003/10/05 02:59:36  trey
# initial microraptor check-in
#
# Revision 1.3  2003/02/17 22:57:20  trey
# make USE_RLOG on by default, and made it easier to override at the command line
#
# Revision 1.2  2002/05/15 20:36:51  trey
# changed default CFLAGS to add -g and remove -O2
#
# Revision 1.1.1.1  2001/08/27 15:46:52  trey
# initial check-in
#
# Revision 1.3  2000/12/14 15:34:18  trey
# changed makedep call so that we pick up all of the header dependencies
#
# Revision 1.2  2000/09/21 14:18:32  trey
# changed LINUX_2_2_5_15 define to LINUX_2_2 define to match this patchlevel change
#
# Revision 1.1  2000/09/17 19:27:13  hersh
# copy of Linux-2.2.5-15.mak
#
# Revision 1.4  2000/08/28 20:01:38  trey
# added use of Mark Maimone's iterate.perl in place of a /bin/sh for loop; this fixes a problem with not stopping when we get compilation errors
#
# Revision 1.3  2000/08/28 16:51:44  trey
# added -Wall flag
#
# Revision 1.2  2000/02/22 03:47:49  trey
# added some defines to the Linux CFLAGS
#
# Revision 1.1.1.1  1999/10/27 02:48:59  trey
# Imported sources
#
#
