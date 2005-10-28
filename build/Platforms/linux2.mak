#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: linux2.mak,v 1.3 2005-10-28 03:34:41 trey Exp $
#
# Copyright (c) 1996-2005, Carnegie Mellon University
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * The software may not be sold or incorporated into a commercial
#   product without specific prior written permission.
# * The above copyright notice and this permission notice shall be
#   included in all copies or substantial portions of the software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#######################################################################
# DESCRIPTION:  Variable values specific to Linux

CFLAGS += -DLINUX

PERL := /usr/bin/perl

######################################################################
# $Log: not supported by cvs2svn $
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
