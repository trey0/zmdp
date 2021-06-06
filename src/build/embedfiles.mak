#!gmake 	# Tell emacs about this file: -*- Makefile -*-
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

EMBEDFILES_OUT := $(TARGET_INC_DIR)/$(EMBEDFILES_TARGET)
EMBEDFILES_DEP := $(EMBEDFILES_TARGET:%.cc=$(OBJ_DIR)/%.d)

$(EMBEDFILES_OUT): $(EMBEDFILES_SRC)
	@[ -d $(TARGET_INC_DIR) ] || mkdir -p $(TARGET_INC_DIR)
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
