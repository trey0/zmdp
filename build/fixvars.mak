#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: fixvars.mak,v 1.1.1.1 2004-11-09 16:18:57 trey Exp $
#
# PROJECT:      FIRE Architecture Project
# DESCRIPTION:  Enables backwards compatibility with some old variable names

# fold old distinct variables (if defined) into the new more generic variables

ifneq (,$(CPPFLAGS))
  CFLAGS += $(CPPFLAGS)
  CPPFLAGS :=
endif

ifneq (,$(INC_DIRS))
  CFLAGS += $(INC_DIRS)
  INC_DIRS :=
endif

ifneq (,$(LIB_DIRS))
  LDFLAGS += $(LIB_DIRS)
  LIB_DIRS :=
endif

ifneq (,$(COMFLAGS))
  CFLAGS += $(COMFLAGS)
  LDFLAGS += $(COMFLAGS)
  COM_FLAGS :=
endif

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.1  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.1  2004/03/19 19:43:25  trey
# initial check-in, are you happy, dom?
#
#
