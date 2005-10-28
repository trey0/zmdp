#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: embedfiles.mak,v 1.2 2005-10-28 02:24:42 trey Exp $
#
# Copyright (c) 1996-2005, Carnegie Mellon University
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# * This code and derivatives may not be incorporated into commercial
#   products without specific prior written permission.
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the name of Carnegie Mellon University nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
