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
# DESCRIPTION:  To be included at end of Makefile.  Sets up most
#               of the Makefile rules (install, clean, etc.)

include $(BUILD_DIR)/fixvars.mak # backwards compatibility

######################################################################
# PATTERN MATCHING META-RULES

# 'gmake foo-subdirs' performs 'gmake foo' in all subdirectories
%-subdirs: ;
ifneq ($(SUBDIRS),)
	$(PERL) $(BUILD_DIR)/iterate.perl $(SUBDIRS) - \
           echo\; $(MAKE) $(MFLAGS) -fMakefile $*
endif

# 'gmake foo-subdirs-depend' performs 'gmake foo' in all subdirectories
#   with dependency checking on (this is the default)
%-subdirs-depend: ;
ifneq ($(SUBDIRS),)
	$(PERL) $(BUILD_DIR)/iterate.perl $(SUBDIRS) - \
	   echo\; $(MAKE) $(MFLAGS) -fMakefile DEPEND=1 $*
endif

print-%: ;
	@echo -e "\n  ----------- beginning $* ------------ \n"

######################################################################
# DEBUG RULES

# 'gmake echo-FOO' prints out 'FOO == [foo_value]' for debugging
echo-%:
	@echo '$* == [$($*)]'

# spits out all the variables
vars:
	@perl -e '$$label = ""; warn "NO VARSn" unless open (IN,"$(MAKE) -p 2>& 1 |"); while (<IN>) { $$l = $$_; if ( $$l =~ /# Directories/) { last }; if ($$l =~ /^#/) { @l = split(" ",$$l); if ($$#l > 0) { $$label = $$l[1]}; next } ; if ( $$l =~ /=/ ) { print "$$label $$l" } else { next } }; exit (0);'

######################################################################
# CANCEL BUILT-IN RULES

# built-in rules unexpectedly kicking in is a source of all
# kinds of bugs... this command should be equivalent to running
# gmake with the --no-builtin-rules option
.SUFFIXES:

######################################################################
# HEADER RULES

$(TARGET_INC_DIR)/%: %
	@[ -d $(TARGET_INC_DIR) ] || mkdir -p $(TARGET_INC_DIR)
	$(INSTALLHEADERS_COPY) $(THIS_DIR)/$< $@

$(TARGET_INC_DIR)/%: $(LOCAL_INC_DIR)/%
	@[ -d $(TARGET_INC_DIR) ] || mkdir -p $(TARGET_INC_DIR)
	$(INSTALLHEADERS_COPY) $(THIS_DIR)/$< $@

######################################################################
# FLEX/BISON RULES

# don't automatically delete intermediate files, speeds up future builds
.SECONDARY:

# the last line of this command is a fall-back for an older version
# of bison that puts output in *.tab.cc.h instead of *.tab.hh
$(LOCAL_INC_DIR)/%.tab.cc $(LOCAL_INC_DIR)/%.tab.hh: %.y
	@[ -d $(LOCAL_INC_DIR) ] || mkdir -p $(LOCAL_INC_DIR)
	$(YACC) $(YFLAGS) --name-prefix=$(LEX_PREFIX) $< -o $(LOCAL_INC_DIR)/$*.tab.cc
	[ -f $(LOCAL_INC_DIR)/$*.tab.hh ] || ln -s $*.tab.cc.h $(LOCAL_INC_DIR)/$*.tab.hh

$(LOCAL_INC_DIR)/%.$(LEX_PREFIX).cc: %.l $(LOCAL_INC_DIR)/%.tab.hh
	@[ -d $(LOCAL_INC_DIR) ] || mkdir -p $(LOCAL_INC_DIR)
	$(LEX) -P$(LEX_PREFIX) $(LEXFLAGS) -o$@ $<

######################################################################
# XDRGEN RULES

$(LOCAL_INC_DIR)/%.xdr.h: %.xdr
	@[ -d $(LOCAL_INC_DIR) ] || mkdir -p $(LOCAL_INC_DIR)
	$(XDRGEN) $(CFLAGS) $< $@

######################################################################
# TDLC RULES

$(LOCAL_INC_DIR)/%.tdl.h : %.tdl
	@[ -d $(LOCAL_INC_DIR) ] || mkdir $(LOCAL_INC_DIR)
	rm -f $@
	perl -ple 's/#include\s+(.*)$$/__x_include($$1)/' $< > include/tmp1_$<
	cpp $(CFLAGS) include/tmp1_$< include/tmp2_$<
	perl -nle 'if (/^# \d+\b/) { print "\n"; next; } s/__x_include\(([^\)]*)\)\s*$$/#include $$1/; print' include/tmp2_$< > include/tmp3_$<
	$(JAVA) $(JAVA_FLAGS) TDLC $(TDLC_HEADER_FLAGS) -o $@ include/tmp3_$<
	rm -f include/tmp*_$<

$(LOCAL_INC_DIR)/%.tdl.cc : %.tdl $(LOCAL_INC_DIR)/%.tdl.h
	@[ -d $(LOCAL_INC_DIR) ] || mkdir $(LOCAL_INC_DIR)
	rm -f $@
	perl -ple 's/#include\s+(.*)$$/__x_include($$1)/' $< > include/tmp1_$<
	cpp $(CFLAGS) include/tmp1_$< include/tmp2_$<
	perl -nle 'if (/^# \d+\b/) { print "\n"; next; } s/__x_include\(([^\)]*)\)\s*$$/#include $$1/; print' include/tmp2_$< > include/tmp3_$<
	$(JAVA) $(JAVA_FLAGS) TDLC $(TDLC_SOURCE_FLAGS) -A '#include "$(@:%.cc=%.h)"'  -o $@ include/tmp3_$<
	rm -f include/tmp*_$<

######################################################################
# JAVA RULES

$(TARGET_JAVA_DIR)/%.class : %.java
	@[ -d $(TARGET_JAVA_DIR) ] || mkdir $(TARGET_JAVA_DIR)
	@-rm -f $@
	$(JAVAC) $(JAVAC_FLAGS) -d $(TARGET_JAVA_DIR) $<

######################################################################
# SKILL MANAGER RULES

$(LOCAL_INC_DIR)/%.skill.cc $(LOCAL_INC_DIR)/%.skill.h : %.cc %_io.h
	@[ -d $(LOCAL_INC_DIR) ] || mkdir -p $(LOCAL_INC_DIR)
	$(SM_BINARY) $(<D)/$* -o $(LOCAL_INC_DIR)/$*.skill 2> /dev/null

$(LOCAL_INC_DIR)/%.smxx.cc : %.smxx $(GEN_SKILL_H)
	@[ -d $(LOCAL_INC_DIR) ] || mkdir -p $(LOCAL_INC_DIR)
	$(SM_BINARY) $< -o $@ 2> /dev/null

######################################################################
# PRECIOUS RULES

# keep gmake from automatically deleting generated source files after
#  the build is complete
.PRECIOUS: $(LOCAL_INC_DIR)/%.cc $(LOCAL_INC_DIR)/%.h

######################################################################
# BASIC C COMPILE RULES

# COMFLAGS are used at compile- and link-time
# CFLAGS are used at compile-time only

$(OBJ_DIR)/%.o: %.c
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# just run the pre-processor to support debugging
%.E: %.c
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -E $< -o $@

# compile to assembly to support debugging
%.s: %.c
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -S $< -o $@

# all the same rules, but for *.cc instead of *.c
# if %.o gets found as include/foo.o, $(@F) will be 'foo.o', where
#   $@ would have been 'include/foo.o'
$(OBJ_DIR)/%.o: %.cc
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(@F)

%.E: %.cc
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -E $< -o $@

%.s: %.cc
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -S $< -o $@

# the same rule, but for include/*.cc instead of *.c
# it does make a difference that LOCAL_INC_DIR is explicitly named,
#   despite the fact that it's in the VPATH
$(OBJ_DIR)/%.o: $(LOCAL_INC_DIR)/%.cc
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(@F)

# all the same rules, but for *.cpp instead of *.c
# if %.o gets found as include/foo.o, $(@F) will be 'foo.o', where
#   $@ would have been 'include/foo.o'
$(OBJ_DIR)/%.o: %.cpp
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(@F)

%.E: %.cpp
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -E $< -o $@

%.s: %.cpp
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -S $< -o $@

# the same rule, but for include/*.cpp instead of *.c
# it does make a difference that LOCAL_INC_DIR is explicitly named,
#   despite the fact that it's in the VPATH
$(OBJ_DIR)/%.o: $(LOCAL_INC_DIR)/%.cpp
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(C++) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(@F)

######################################################################
# DATA FILE RULES

$(TARGET_COMMON_BIN_DIR)/%: %
	@[ -d $(TARGET_COMMON_BIN_DIR) ] || mkdir -p $(TARGET_COMMON_BIN_DIR)
	chmod 755 $<
	$(INSTALLSCRIPT_COPY) $(THIS_DIR)/$< $(TARGET_COMMON_BIN_DIR)/$<

######################################################################
# INSTALL RULES

# .PHONY causes make to be more efficient, by telling it that
#   "gmake foo" doesn't mean "try to make the file `foo'",
#   for any foo in this list of commands.
.PHONY: depend depend-local
depend: depend-local nothing-subdirs-depend

# when DEPEND=1 is enabled, we automatically do dependencies
depend-local:
	@echo
	$(MAKE) $(MFLAGS) -fMakefile DEPEND=1 nothing

# use the nothing target when you want to do nothing
# rather than building the default target (doing nothing may have side
# effects... for instance, building dependencies if DEPEND=1)
.PHONY: nothing
nothing:

# override the toplevel-pre-install target in the top-level makefile to
# install tools needed for the rest of the compile
.PHONY: toplevel-pre-install
toplevel-pre-install:

######################################################################
# INSTALL
######################################################################

.PHONY: install

# some steps must be done before dependency check: for instance,
# the dependency check will break if header files generated by
# xdrgen are missing.
.PHONY: install-before-deps
install-before-deps: toplevel-pre-install print-headerinstall headerinstall print-libinstall

.PHONY: install-after-deps
install-after-deps: libinstall print-bininstall bininstall

ifdef NO_DEPEND
# user explicitly doesn't want dependencies
install: install-before-deps install-after-deps
else
ifdef DEPEND
# DEPEND is already defined: just go about your business
install: install-before-deps install-after-deps
else
# DEPEND isn't defined; re-run gmake with DEPEND=1
install: install-before-deps
	@echo
	$(MAKE) $(MFLAGS) -fMakefile DEPEND=1 install-after-deps
endif
endif

######################################################################
# ALL
######################################################################

.PHONY: all

# some steps must be done before dependency check: for instance,
# the dependency check will break if header files generated by
# xdrgen are missing.
.PHONY: all-before-deps
all-before-deps: print-headers headers print-libs

.PHONY: all-after-deps
all-after-deps: libs print-bins bins

ifdef NO_DEPEND
# user explicitly doesn't want dependencies
all: all-before-deps all-after-deps
else
ifdef DEPEND
# DEPEND is already defined: just go about your business
all: all-before-deps all-after-deps
else
# DEPEND isn't defined; re-run gmake with DEPEND=1
all: all-before-deps
	@echo
	$(MAKE) $(MFLAGS) -fMakefile DEPEND=1 all-after-deps
endif
endif

.PHONY: xdrgen xdrgen-local
xdrgen: xdrgen-local xdrgen-subdirs
xdrgen-local: $(XDRGEN_TARGETS)

.PHONY: headerinstall install headerinstall-local
headerinstall: headerinstall-local headerinstall-subdirs
include: headerinstall

.PHONY: headers headers-local
headers: headers-local headers-subdirs

.PHONY: libinstall libinstall-local
libinstall: libinstall-local libinstall-subdirs

.PHONY: libs libs-local
libs: libs-local libs-subdirs

.PHONY: bininstall bininstall-local
bininstall: bininstall-local bininstall-subdirs

.PHONY: bins bins-local
bins: bins-local bins-subdirs

FOOTER_ALL_TARGETS :=

HEADERINSTALL_TARGETS := \
  $(XDRGEN_TARGETS) \
  $(INSTALLHEADERS_TARGETS)
FOOTER_ALL_TARGETS += $(HEADERINSTALL_TARGETS)

# on headerinstall (pass 1): auto-generate explicitly generated files
# (e.g., *.xdr.h), build java classes, install headers.
headerinstall-local: $(HEADERINSTALL_TARGETS)
headers-local: $(XDRGEN_LOCALS)

FOOTER_ALL_TARGETS += $(BUILDLIB_TARGETS)

# on libinstall (pass 2): check dependencies, compile and build libs.
# NOTE: checking dependencies may cause headers to get generated (e.g.,
# *.tdl.h)
libinstall-local: $(BUILDLIB_TARGETS)
libs-local: $(BUILDLIB_LOCALS)

BININSTALL_TARGETS := \
  $(BUILDBIN_TARGETS) \
  $(INSTALL_DATA_TARGETS) \
  $(BUILDJAVA_TARGETS)

FOOTER_ALL_TARGETS += $(BININSTALL_TARGETS)

# on bininstall (pass 3): link binaries, install data files.
bininstall-local: $(BININSTALL_TARGETS)
bins-local: $(BUILDBIN_LOCALS)

######################################################################
# DOC RULES

.PHONY: localdoc
localdoc:
	@[ -d $(LOCAL_INC_DIR) ] || mkdir -p $(LOCAL_INC_DIR)
	$(DOXYGEN) $(BUILD_DIR)/doxygen.cfg

######################################################################
# CLEANUP RULES

.PHONY: clean clean-local
clean: clean-local clean-subdirs

# the weirdness with the TEST variable is to make sure that anything
# built with TEST=1 is cleaned up even without TEST=1

clean-local:
ifeq (,$(TEST))
	$(MAKE) $(MFLAGS) -fMakefile TEST=1 clean-local
else
	rm -rf $(OBJ_DIR) $(LOCAL_INC_DIR) $(LOCAL_LIB_DIR) $(LOCAL_BIN_DIR) $(BUILDBIN_SYMLINKS_TO_CLEAN) $(LOCAL_CLEAN_TARGETS) $(LOCAL_CLEAN_FILES) *~ core core.* *.pyc
endif

.PHONY: uninstall uninstall-local
uninstall: toplevel-uninstall uninstall-local uninstall-subdirs
uninstall-local:
ifeq (,$(TEST))
	$(MAKE) $(MFLAGS) -fMakefile TEST=1 uninstall-local
else
	-rm -f $(FOOTER_ALL_TARGETS)
endif

# override the toplevel-uninstall target in the toplevel Makefile only
.PHONY: toplevel-uninstall
toplevel-uninstall:

.PHONY: realclean
ifeq (,$(TEST))
realclean:
	$(MAKE) $(MFLAGS) -fMakefile TEST=1 realclean
else
realclean: clean uninstall
endif
