#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: options.mak,v 1.6 2005-02-06 16:40:10 trey Exp $

# algorithm configuration options

#CFLAGS += -DDO_LB_SELF_UPDATE=1
CFLAGS += -DDO_UB_CACHED_Q=1
#CFLAGS += -DDO_UB_SELF_UPDATE=1
#CFLAGS += -DNO_COMPRESSED_MATRICES=1
#CFLAGS += -DDO_AGGRESSIVE_LB_PRUNE=1
#CFLAGS += -DUSE_UBLAS=1
#CFLAGS += -DUSE_EMPOMDP=1

# turns on optimization in boost matrix library (see commonTypes.h)
CFLAGS += -DVEC_OPTIM=1

USER_CFLAGS := -O3
#USER_CFLAGS := -g
#USE_CPLEX := 1

ifneq (,$(USE_CPLEX))
  CFLAGS += -DUSE_CPLEX=1
endif

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.5  2005/02/04 19:56:56  trey
# turned off ublas by default
#
# Revision 1.4  2005/01/28 03:23:37  trey
# moved USER_CFLAGS into options.mak
#
# Revision 1.3  2005/01/21 18:07:02  trey
# preparing for transition to sla matrix types
#
# Revision 1.2  2005/01/21 15:22:40  trey
# added DO_AGGRESSIVE_LB_PRUNE
#
# Revision 1.1  2005/01/17 19:37:32  trey
# initial check-in
#
#
