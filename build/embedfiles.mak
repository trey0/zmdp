#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: embedfiles.mak,v 1.3 2005-10-28 03:34:41 trey Exp $
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

######################################################################
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
# Revision 1.2  2005/10/28 02:24:42  trey
# added copyright headers
#
# Revision 1.1.1.1  2004/11/09 16:18:57  trey
# imported hsvi into new repository
#
# Revision 1.1  2004/05/13 20:25:50  trey
# synced up with atacama makefile system again
#
# Revision 1.1  2004/04/19 17:53:38  trey
# initial check-in
#
#
