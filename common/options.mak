#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: options.mak,v 1.1 2005-01-17 19:37:32 trey Exp $

# algorithm configuration options

CFLAGS += -DDO_LB_SELF_UPDATE=1
CFLAGS += -DDO_UB_CACHED_Q=1
#CFLAGS += -DDO_UB_SELF_UPDATE=1
#CFLAGS += -DNO_COMPRESSED_MATRICES=1

# turns on optimization in boost matrix library (see bvector.h)
#CFLAGS += -DVEC_OPTIM=1

######################################################################
# $Log: not supported by cvs2svn $
#
