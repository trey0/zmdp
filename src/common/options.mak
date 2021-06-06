#!gmake 	# Tell emacs about this file: -*- Makefile -*-
#
# Copyright (c) 2002-2006, Trey Smith. All rights reserved.
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

CFLAGS += -DZMDP_VERSION=1.1.7

# algorithm configuration options (most of the options that used to be
# in this file are now run-time configuration parameters; see
# src/main/zmdp.config)

#CFLAGS += -DUSE_HSVI_ADAPTIVE_DEPTH=1

# debug/optimization options

USER_CFLAGS := -O3
#USER_CFLAGS := -g

XCFLAGS := $(CFLAGS)
CFLAGS += -DCFLAGS="\"$(XCFLAGS)\""
