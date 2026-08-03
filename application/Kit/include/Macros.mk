#
# space character used here and there for better stdout formatting
#
space :=
space +=

#
# A macro to produce a path from given parameters. Every possible
# path, needed for compilation or installation, is encoded by this
# macro. The macro makes unnecessary to define variable for specific
# directories.
#
# Parameters:
#
#    1/ module or introot: location path, that is, either module where
#       Makefile is located or application integration root.
#
#    2/ host or target: use intent, that is, target if files in a
#       directory are specific for a cpu or module otherwise.
#
#    3/ bin, include, lib, object, config: file types in directory.
#
# | destination  | meta-type | type    | directory name              |
# |--------------+-----------+---------+-----------------------------|
# | module path  | host      | bin     | $(modroot)/bin              |
# |              |           | lib     | $(modroot)/lib              |
# |              |           | object  | $(modroot)/object           |
# |              |           | include | $(modroot)/include          |
# |              |           | config  | $(modroot)/config           |
# |              | target    | bin     | $(modroot)/bin/$(target)    |
# |              |           | lib     | $(modroot)/lib/$(target)    |
# |              |           | object  | $(modroot)/object/$(target) |
# |              |           | include | $(modroot)/include/target   |
# |--------------+-----------+---------+-----------------------------|
# | introot path | host      | bin     | $(introot)/bin              |
# |              |           | lib     | $(introot)/lib              |
# |              |           | include | $(introot)/include          |
# |              |           | config  | $(introot)/config           |
# |              | target    | bin     | $(introot)/bin/$(target)    |
# |              |           | lib     | $(introot)/lib/$(target)    |
# |              |           | include | $(introot)/include/target   |
# 
path_macro = \
    $(1)/$(3)$(if $(findstring host,$(2)),,$(if $(findstring include,$(3)),$(if $(findstring $(MODROOT),$(1)),,/target),/$(TARGET)))

#
# find file in module or introot, module takes priority
#
# Parameters:
#    1/ meta-type
#    2/ type
#    3/ file name
find_file = \
    $(if $(wildcard $(call path_macro,$(MODROOT),$(1),$(2))/$(3)),$(wildcard $(call path_macro,$(MODROOT),$(1),$(2))/$(3)),$(wildcard $(call path_macro,$(INTROOT),$(1),$(2))/$(3)))

#
# Python packages are always under 'host'
#
# Parameters:
#
#    1/ module or introot: location path, that is, either module where
#       Makefile is located or application integration root.
#
path_python_package_macro = \
    $(call path_macro,$(1),host,lib)/python/embedded

define do_install_python_package_macro
.PHONY: do_install_python_package_$(1)
do_install_python_package_$(1): $(addsuffix .py,$(2))
	$(LOG)mkdir -p $(call path_python_package_macro,$(INTROOT))/$(1)
	$(LOG)cp $(foreach i,$(2),$(call path_python_package_macro,$(MODROOT))/$(1)/$(i).py) $(call path_python_package_macro,$(INTROOT))/$(1)/.
endef

#
# Macro to instantiate an install target. Macro parameters for not lib:
#
#    1/ meta-type: host or target
#    2/ type of files to install (bin, lib, include, config)
#    3/ files to install (names with extension)
#
# Macro parameters for lib:
#
#    1/ meta-type: host or target
#    2/ type is lib
#    3/ libraries to install (names with extension .a)
#    4/ objects files to install (names with extension .o)
#
# $(1) is interpreted as that part of the directory structure from where to copy
# files into the equivalent place under root. If $(2) is empty (no files to
# install) the macro creates a target without dependencies nor commands.
# 
# To resolve a path to the module's installation directory there is a macro;
# which hardcodes the actual mapping from installation type (bin, lib, include)
# to a path.
#
define do_install_macro
ifneq ($(3),)
.PHONY: do_install_$(1)_$(2)
do_install_$(1)_$(2): $(foreach i,$(3),$(call path_macro,$(MODROOT),$(1),$(2))/$(i))
	$(if $(strip $(3)),@echo "[INSTALL]" $$@)
	$(if $(strip $(3)),@echo "    $(subst $(space),\n    ,$(strip $(3)))")
	$(if $(strip $(3)),$(LOG)mkdir -p $(call path_macro,$(INTROOT),$(1),$(2)))
	$(if $(strip $(3)),$(LOG)cp $$^ $(call path_macro,$(INTROOT),$(1),$(2))/.,)
ifeq ($(2),lib)
	$(if $(strip $(3)),$(LOG)mkdir -p $(call path_macro,$(INTROOT),$(1),$(2))/$(3))
	$(if $(strip $(3)),$(LOG)cp $$^ $(call path_macro,$(INTROOT),$(1),$(2))/$(3)/.,)
	$(LOG)ramUnpackArchive $(addprefix $(call path_macro,$(INTROOT),target,lib)/,$(3))
endif
endif
endef

#
# Macro parameters for lib:
#
#    1/ library name (just the name, this is not a file)
#    2/ objects files to install (names with extension .o)
#
# Libraries have their own target macro for installation. Instead of
# using more general define do_install_macro a specific target is
# produced for each library in the Makefile. This is so because each
# library is not a individual file. Instead, all objects files in
# MODROOT associated to a library are the actual files to copy,
# therefore, that's the list of dependencies for each library
# (do_install_macro assumes that the dependencies are the actual
# library archive). In this case all objexts are copied to
# INTROOT/lib/target/LIBNAME and a global thin archive is updated with
# references to those objects. That is, there is no archive to
# install, only object files.

# $(1) is interpreted as that part of the directory structure from where to copy
# files into the equivalent place under root. If $(2) is empty (no files to
# install) the macro creates a target without dependencies nor commands.
# 
# To resolve a path to the module's installation directory there is a macro;
# which hardcodes the actual mapping from installation type (bin, lib, include)
# to a path.
#
define do_install_macro_lib
ifneq ($(2),)
.PHONY: do_install_lib_$(1)
do_install_lib_$(1): $(foreach i,$(2),$(call path_macro,$(MODROOT),target,object)/$(i))
	@echo "[INSTALL]" $$@
	@echo "	   $(subst $(space),\n	  ,$(2))"
	$(LOG)mkdir -p $(call path_macro,$(INTROOT),target,lib)/$(1)
	$(LOG)cp $$^ $(call path_macro,$(INTROOT),target,lib)/$(1)/.
	$(LOG)$(AR) -TPrs $(call path_macro,$(INTROOT),target,lib)/lib$(if $(3),$(3),all).a $(foreach obj,$(2),$(call path_macro,$(INTROOT),target,lib)/$(1)/$(obj))
endif
endef

#
# Macro for 'compiling shell and python scripts. A script start as a
# file in src from where it is 'compiled' by adding a heading shebang
# statement into a file under module's bin. Macro parameter:
#
#   1/ script file name with extension
#
define script_compile_rule
$(call path_macro,$(MODROOT),host,bin)/$(basename $(1)): $(1)
	$(LOG)mkdir -p $(call path_macro,$(MODROOT),host,bin)
	@echo " [$$(subst .,,$$(suffix $(1)))]" `basename $$@`
ifeq ($(suffix $(1)),.sh)
	$(LOG)echo "#!/bin/bash" > $$@
else ifeq ($(suffix $(1)),.py)
	$(LOG)echo "#!/usr/bin/env python3" > $$@
else
	echo invalid script type for $(1); false
endif
	$(LOG)cat $$< >> $$@
	$(LOG)chmod u+x $$@
endef

#
# A python package is a set of python module files grouped together into
# one directory.
#
#   1/ package name
#   2/ list of python files with extension
#
define python_package_compile_rule
.PHONY: $(call path_python_package_macro,$(MODROOT))/$(1)
$(call path_python_package_macro,$(MODROOT))/$(1): $(2)
	$(LOG)mkdir -p $(call path_python_package_macro,$(MODROOT))/$(1)
	@echo " [PYP]" `basename $$@`
	$(if $(strip $(2)),@echo "    $(subst $(space),\n    ,$(strip $(2)))")
	@cp $(2) $$@/.
endef

#
# Compile but do not link. Macro variables:
#
#     1/ s or S file to compile (with extension)
#     2/ static libraries the object depends on (path and extension names)
#
define assembler_compile_rule
$(call path_macro,$(MODROOT),target,object)/$(basename $(1)).o: $(1) $(2)
	@mkdir -p $(call path_macro,$(MODROOT),target,object)
	@echo "  [$$(suffix $(1))]" `basename $$@`
	$(LOG)$(CC) -c                          \
    $(CFLAGS)                                   \
    $(foreach i,$(subst :, ,$(IPATH)), -I$(i))  \
    $$<                                         \
    -o $$@
	$(LOG)$(SIZE) $$@ | cut -f1-5

#
# check for modified dependencies collected in *.d files
#
-include $(call path_macro,$(MODROOT),target,object)/$(basename $(1)).d
endef

#
# Compile but do not link. Create also a target for an assembler file,
# a target that needs to be called explicitly to generate an *.s
# file. Originally create for rmsbolt's sake.
# Macro variables:
#
#     1/ c or cpp file to compile (with extension)
#
define c/cpp_compile_rule
$(eval COMPCMD:=$(if $(findstring cpp,$(suffix $(1))),$(CXX),$(CC)))
$(eval COMPFLAGS:=$(if $(findstring cpp,$(suffix $(1))),$(CXXFLAGS),$(CFLAGS)))
$(call path_macro,$(MODROOT),target,object)/$(basename $(1)).o: $(1)
	@mkdir -p $(call path_macro,$(MODROOT),target,object)
	@echo [$(suffix $(1))] `basename $$@`
	$(LOG)$(COMPCMD) -c                                          \
    $(COMPFLAGS)                                                     \
    $(foreach i,$(subst :, ,$(IPATH)), -I$(i))                       \
    $$<                                                              \
    -o $$@
	$(LOG)$(SIZE) $$@ | cut -f1-5
.PHONY: $(call path_macro,$(MODROOT),target,object)/$(basename $(1)).s
$(call path_macro,$(MODROOT),target,object)/$(basename $(1)).s:
	@mkdir -p $(call path_macro,$(MODROOT),target,object)
	@echo [$(suffix $(1))] `basename $$@`
	$(LOG)$(COMPCMD) -S -g                                       \
    $(COMPFLAGS)                                                     \
    $(foreach i,$(subst :, ,$(IPATH)), -I$(i))                       \
    $(1)                                                             \
    -o $$(@:.o=.s)
	$(eval TMP := $(shell tempfile))
	$(LOG)$(DEMANGLER) < $$(@:.o=.s) > $(TMP) && mv $(TMP) $$(@:.o=.s)
#
# check for modified dependencies collected in *.d files
#
-include $(call path_macro,$(MODROOT),target,object)/$(basename $(1)).d
endef

#
# Compile but do not link. Macro variables:
#
#     1/ files to compile together (with extension)
#
#     2/ module's static libraries the object depends on (names only,
#        e.g module/lib<name>.a)
#
#     3/ introot's static libraries the object depends on (names only, e.g
#        introot/lib<name>.a)
#
# $(2) is used only as a dependency to force re-building whenever a static
# library has changed.
#
define compile_rules
$(foreach i,$(filter %.s %.S,$(strip $(1))),                            \
    $(eval                                                              \
    $(call assembler_compile_rule,$(i))))
$(foreach i,$(filter %.c %.cpp,$(strip $(1))),                          \
    $(eval                                                              \
    $(call c/cpp_compile_rule,$(i))))
$(foreach i,$(filter %.sh %.py,$(strip $(1))),                          \
    $(eval                                                              \
    $(call script_compile_rule,$(i))))
endef

#
# Linker rule. Macro variables:
#
#     1/ executable name without extension 
#     2/ object files with extension to link together
#     3/ static library names to link together
#     4/ board libs
#     5/ standard libraries
#     6/ linker script (it could be an empty string)
#
# Note: ranlib is not used because the following note
#
# https://www.gnu.org/software/make/manual/html_node/Archive-Symbols.html
#
# This is not necessary when using the GNU ar program, which updates the
# __.SYMDEF member automatically.
#
reverse = $(shell printf "%s\n" $(strip $1) | tac)
define link_rule
$(eval objects:=$(foreach obj,$(strip $(2)),$(call path_macro,$(MODROOT),target,object)/$(obj)))
$(eval archives:=$(foreach arch,$(call reverse,$(strip $(3))),-l$(arch)))
$(eval boardlibs:=$(foreach lib,$(call reverse,$(strip $(4))),-l$(lib)))
$(eval stdlibs:=$(foreach lib,$(call reverse,$(strip $(5))),-l$(lib)))
$(eval Tldscript:=$(if $(strip $(6)),-T$(strip $(6)),))
$(call path_macro,$(MODROOT),target,bin)/$(strip $(1)).elf: $(strip $(6)) $(objects)
	@mkdir -p $(call path_macro,$(MODROOT),target,bin)
	@echo " "[LD] `basename $$@`
	$(LOG)$(LD) $(LDFLAGS)				\
    $(Tldscript)					\
    -L$(call path_macro,$(MODROOT),target,lib)		\
    -L$(call path_macro,$(INTROOT),target,lib)		\
    $(patsubst %,-L%,$(subst :, ,$(BOARD_LIBS_PATH)))	\
    -o $$@						\
    $(objects)						\
    -Wl,--start-group $(archives) -Wl,--end-group	\
    $(boardlibs)					\
    $(stdlibs)
ifeq ($(TARGET),nano328p)
	$(LOG)$(OBJCOPY) -O ihex -j .eeprom	\
    --set-section-flags=.eeprom=alloc,load	\
    --no-change-warnings			\
    --change-section-lma .eeprom=0		\
    $$@						\
    $$(@:.elf=.epp)
	$(LOG)$(OBJCOPY) -O ihex -R .eeprom	\
    $$@						\
    $$(@:.elf=.hex)
endif
	$(LOG)$(SIZE) $$@ | cut -f1-5
endef

#
# Archive rule. Macro variables:
#
#     1/ archive name without leading lib and no extension
#     2/ object files with extension to archive together
#     3/ archive files without extension to archive together
#
define archive_rule
$(eval objects:=$(foreach obj,$(strip $(2)),$(call path_macro,$(MODROOT),target,object)/$(obj)))
$(eval archives:=$(foreach arch,$(strip $(3)),$(call find_file,target,lib,lib$(arch).a)))
$(call path_macro,$(MODROOT),target,lib)/lib$(strip $(1)).a: $(objects)
	@mkdir -p $(call path_macro,$(MODROOT),target,lib)
	@echo " "[AR] `basename $$@`
	$(LOG)rm -f $(call path_macro,$(MODROOT),target,lib)/lib$(strip $(1)).a
	$(LOG)$(AR) -rcs $(call path_macro,$(MODROOT),target,lib)/lib$(strip $(1)).a $(objects) $(archives)
	$(eval tmp := $(shell $(AR) -t $(call path_macro,$(MODROOT),target,lib)/lib$(strip $(1)).a))
	$(if $(strip $(tmp)),@echo "    $(subst $(space),\n    ,$(strip $(tmp)))")
endef

#
# Bit stream upload recipe. Defining the recipe standalone makes
# possible to use it at will. For example, in any Makefile one could
# define a rule with a random prerequisite, which this recipe will
# upload, without trying to build anything.
#
ifneq (,$(findstring $(TARGET),tivac-minim4))
define upload_receipe
	$(eval tmp := $(lastword $(shell $(READELF) -h $< | grep "Entry point address")))
	$(if $(filter RAM,$(TARGET_MEMORY)),$(LOG)$(LOADER) -f $(LOADER_CONFIG_FILE) -c "ramboot $< $(tmp)",$(LOG)$(LOADER) -f $(LOADER_CONFIG_FILE) -c "romboot $<")
endef
else
define upload_receipe
#
# if there is ONE screen running on the target then kill it. If there is more
# than one then I do not really know what's going on and better to let the
# loader try and see what happens.
# Note: it took me ~two days work to craft these few script lines. What confused
# me all the time was that when not using a regular expression then pgrep was
# also counting its own process and diverting all my logic to hell.
#
	$(LOG)if [ $$(pgrep -fc "^SCREEN /dev/$(TARGET).*") = 1 ];  \
    then                                                        \
       pid=$$(pgrep -f "^SCREEN /dev/$(TARGET).*");             \
       session=$$(screen -ls | grep $$pid | awk '{print $$1}'); \
       screen -X -S $$session kill;                             \
    fi
	$(LOG)$(LOADER) -C $(LOADER_CONFIG_FILE)                        \
    -q -V -D -p atmega328p -c arduino -P/dev/$(TARGET) -b57600 -D   \
    -Uflash:w:$(<:.elf=.hex):i 
endef
endif

#
# Reset target to simply start the program again
#
ifneq (,$(findstring $(TARGET),tivac-minim4-discovery))
define reset_receipe
	$(LOG)$(LOADER) -f $(LOADER_CONFIG_FILE) -c reset
endef
else
$(error reset not implemented for TARGET=$(TARGET))
endif

#___oOo___
