#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: example-pomdp-buildrc.mak,v 1.1 2004-11-09 18:33:06 trey Exp $
#
# PROJECT:      POMDP Planning
# DESCRIPTION:  install this file as ~/.pomdp.buildrc to set up
#               your personal configuration
#
######################################################################

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
# Revision 1.1  2004/05/21 19:16:58  trey
# branched example-buildrc.mak into two files, one for atacama and one for microraptor; added COMPILER_DIRECTORY_WITH_SLASH configuration variable
#
#
#
