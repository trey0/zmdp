#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: embedfiles.mak,v 1.1.1.1 2004-11-09 16:18:57 trey Exp $
#
# PROJECT:      FIRE Architecture Project
# DESCRIPTION:  Include to embed a data file in C code.
#

# Input variables:
#
# EMBEDFILES_TARGET
#    The output C file that includes the embedded data file.
# EMBEDFILES_SRC
#    The input C file that includes calls to the EMBED_FILE() macro.
#    (EMBED_FILE() calls provide information about which files to embed.)

include $(BUILD_DIR)/fixvars.mak # backwards compatibility

EMBEDFILES_OUT := $(LOCAL_INC_DIR)/$(EMBEDFILES_TARGET)
EMBEDFILES_DEP := $(EMBEDFILES_TARGET:%.cc=$(OBJ_DIR)/%.d)

$(EMBEDFILES_OUT): $(EMBEDFILES_SRC)
	@[ -d $(LOCAL_INC_DIR) ] || mkdir -p $(LOCAL_INC_DIR)
	$(BUILD_DIR)/embedFiles.perl $(CFLAGS) $(EMBEDFILES_SRC) -o $(EMBEDFILES_OUT)

$(EMBEDFILES_DEP): $(EMBEDFILES_SRC)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(BUILD_DIR)/embedFiles.perl $(CFLAGS) $(EMBEDFILES_SRC) -o $(EMBEDFILES_OUT) -M > $@

XDRGEN_LOCALS += $(EMBEDFILES_OUT)
XDRGEN_TARGETS += $(EMBEDFILES_OUT)

ifdef DEPEND
-include $(EMBEDFILES_DEP)
endif

# clear input variables (no need, we don't include this more than once)
#EMBEDFILES_TARGET :=
#EMBEDFILES_SRC :=

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.1  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.1  2004/04/19 17:53:38  trey
# initial check-in
#
#
