#!gmake 	# Tell emacs about this file: -*- Makefile -*-  
# $Id: Makefile,v 1.1.1.1 2004-11-09 16:18:56 trey Exp $
#
# PROJECT:      FIRE Architecture Project
# DESCRIPTION:  

######################################################################
# DO NOT MODIFY THIS HEADER SECTION

CHECKOUT_DIR := $(shell pwd | perl -e '$$/ = undef; $$all = <>; $$pos = $$[; $$out = ""; $$all =~ s:\\:/:g; chop($$all); while (($$pos = index($$all,"/src")) >= $$[) { $$all = substr($$all,$$pos+1); if (length($$all) > 3 && substr($$all,$$[+3,1) ne "/") { next } $$out = $$all; $$out =~ s/[^\/]+/../g; if (-f $$out . "/src/Build/header.mak") { break; } else { $$out = "" } } print $$out;')
include $(CHECKOUT_DIR)/src/Build/header.mak

######################################################################
# PLACE MODIFICATIONS BELOW THIS LINE

# ADD ANY SUBDIRS THAT NEED TO BE VISITED DURING A BUILD TO THE SUBDIRS LIST
SUBDIRS := 

CPPFLAGS += \
  -I/usr/local/include/python2.2 \
  -I/usr/local/lib/python2.2/config

$(TARGET_LIB_DIR)/_%.so: $(OBJ_DIR)/%_wrap.o
	$(LD) $(COMFLAGS) $(LDFLAGS) -shared $< $(LIB_DIRS) -lpomdp_spec -o $@

PY_MODULE_HEADERS := pomdp.h

$(GEN_DIR)/%_wrap.cc: %.i $(PY_MODULE_HEADERS)
	@[ -d $(GEN_DIR) ] || mkdir -p $(GEN_DIR)
	swig -c++ -python -shadow -dnone -o $@ $<
	@[ -d $(TARGET_PYTHON_DIR) ] || mkdir -p $(TARGET_PYTHON_DIR)
	mv $(GEN_DIR)/$*.py $(TARGET_PYTHON_DIR)/

# using these vars directly is a bit of an abuse of the makefile system

XDRGEN_TARGETS += $(GEN_DIR)/pomdp_wrap.cc
BUILDBIN_TARGETS += $(TARGET_LIB_DIR)/_pomdp.so

BUILDLIB_TARGET := pomdp_spec
BUILDLIB_SRCS := pomdp_spec.tab pomdp_spec.yy \
  imm-reward parse_err parse_hash \
  sparse-matrix mdp pomdp
include $(BUILD_DIR)/buildlib.mak

# use 'gmake TEST=1 install' to build the following stuff
ifneq (,$(TEST))

BUILDBIN_TARGET := testSpec
BUILDBIN_SRCS := testSpec
BUILDBIN_INDEP_LIBS := 
BUILDBIN_DEP_LIBS := pomdp_spec
include $(BUILD_DIR)/buildbin.mak

endif

######################################################################
# DO NOT MODIFY BELOW THIS POINT

include $(BUILD_DIR)/footer.mak

######################################################################
# $Log: not supported by cvs2svn $
# Revision 1.2  2003/04/02 17:13:54  trey
# minor changes to enable compilation under gcc 3.0.1
#
# Revision 1.1.1.1  2003/01/07 19:19:41  trey
# Imported sources
#
# Revision 1.12  2002/09/25 16:20:42  trey
# put in updated rules for DW version of genWorld and viewWorld
#
# Revision 1.11  2002/09/25 16:15:04  trey
# defined sr library and rules for SR version of genWorld and viewWorld
#
# Revision 1.10  2002/09/19 15:18:00  trey
# added SR stuff
#
# Revision 1.9  2002/08/26 20:47:04  trey
# added text display to SimpleImage (involved recoding almost from scratch)
#
# Revision 1.8  2002/08/16 17:44:24  trey
# started adding config file support
#
# Revision 1.7  2002/07/04 14:33:09  trey
# greedy2 overhauled to allow interleaved em and execution, new apps added
#
# Revision 1.6  2002/06/26 20:41:34  trey
# massive improvements to EM, first experiments run
#
# Revision 1.5  2002/05/30 18:07:56  trey
# added basic EM algorithm for inferring cluster parameters
#
# Revision 1.4  2002/05/22 20:23:40  trey
# discrete world now works
#
# Revision 1.3  2002/05/21 03:25:48  trey
# almost ready for discrete sim to work
#
# Revision 1.2  2002/05/16 23:27:15  trey
# major overhaul
#
# Revision 1.1  2002/04/30 14:26:21  trey
# initial check-in
#
# Revision 1.1  2001/10/31 21:43:24  trey
#
