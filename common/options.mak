#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: options.mak,v 1.10 2005-03-11 19:25:35 trey Exp $

# algorithm configuration options

#CFLAGS += -DDO_LB_SELF_UPDATE=1
CFLAGS += -DDO_UB_CACHED_Q=1
#CFLAGS += -DDO_UB_SELF_UPDATE=1
#CFLAGS += -DNO_COMPRESSED_MATRICES=1
#CFLAGS += -DDO_AGGRESSIVE_LB_PRUNE=1
#CFLAGS += -DUSE_UBLAS=1
#CFLAGS += -DUSE_EMPOMDP=1
#CFLAGS += -DUSE_RANDOM_OBS=1
CFLAGS += -DUSE_COMPRESSED_ALPHA=1
#CFLAGS += -DUSE_MORE_UB_BACKUPS=1
CFLAGS += -DUSE_MASKED_ALPHA=1
CFLAGS += -DUSE_UB_HASH=0

# turns on optimization in boost matrix library (see commonTypes.h)
CFLAGS += -DVEC_OPTIM=1

USER_CFLAGS := -O3
#USER_CFLAGS := -g
#USE_CPLEX := 1

ifneq (,$(USE_CPLEX))
  CFLAGS += -DUSE_CPLEX=1
endif

XCFLAGS := $(CFLAGS)
CFLAGS += -DCFLAGS="\"$(XCFLAGS)\""

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.9  2005/03/10 21:12:34  trey
# added USE_MASKED_ALPHA
#
# Revision 1.8  2005/02/09 20:44:15  trey
# turned off USE_RANDOM_OBS and USE_MORE_UB_BACKUPS by default
#
# Revision 1.7  2005/02/08 23:54:59  trey
# added USE_COMPRESSED_ALPHA, USE_RANDOM_OBS, and USE_MORE_UB_BACKUPS options
#
# Revision 1.6  2005/02/06 16:40:10  trey
# added USE_EMPOMDP option, changed USE_CPLEX to be off by default
#
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
