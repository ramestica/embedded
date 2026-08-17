# The following page contains a clear overview for targeting an arm
# http://www.cryptopp.com/wiki/ARM_Embedded_%28Bare_Metal%29

REPOROOT := $(shell git rev-parse --show-toplevel)

#
# INTROOT externally defined by environment variable, and module path
# assumed to be one up from path to present Makefile.
#
INTROOT := $(abspath $(EMBEDDED_ROOT))
MODROOT := $(abspath ../)
$(info INTROOT=$(INTROOT))

#
# Some defines introduced below:
#
# F_CPU: clock frequency in Hz. Always an integer value.
# F_CPU_CYCLES_PER_US: clock cycles per microsecond. Always an integer value.

#
# When not set then force a default target
# 
ifndef TARGET
$(error no TARGET defined)
$(info forcing TARGET=$(TARGET))
else
$(info using TARGET=$(TARGET))
endif

#
# make sure the target is known
#
ifeq ($(TARGET),nano328p)
PROCESSOR=atmega328p
else ifneq (,$(findstring $(TARGET),tivac-minim4))
PROCESSOR=tm4c123gh6pm
F_CPU=80000000UL
F_CPU_CYCLES_PER_US=80UL
TARGET_INCLUDE=$(REPOROOT)/cmsis/core/CMSIS/Core/Include:$(REPOROOT)/cmsis/dsp/Include:$(REPOROOT)/tivaware
# why stmcube distributes its own cmsis?
#TARGET_INCLUDE=/home/ramestic/embedded/STM32Cube_FW_F4_V1.24.0/Drivers/CMSIS/Core/Include:/home/ramestic/embedded/STM32Cube_FW_F4_V1.24.0/Drivers/CMSIS/DSP/Include:$(TIVA_WARE_C_ROOT)
BOARD_LIBS_PATH=$(REPOROOT)/tivaware/driverlib/gcc:$(CMSIS_ROOT)/CMSIS/DSP/Lib/GCC
#
# board specific libraries contributed from somewhere else. Originally
# I tried gcc's standard LIBRARY_PATH variable instead of my own
# BOARD_LIBS_PATH. But it seems that arm-none-eabi-gcc replaces
# LIBRARY_PATH by its own.
#
#BOARD_LIBS = driver arm_cortexM4lf_math
else ifneq (,$(findstring $(TARGET),discovery))
PROCESSOR=stm32f407vgt6
F_CPU=168000000UL
F_CPU_CYCLES_PER_US=168UL
TARGET_INCLUDE=$(CMSIS_ROOT)/CMSIS/Core/Include:$(CMSIS_ROOT)/CMSIS/DSP/Include:$(STM32_ROOT)/$(TARGET)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
TARGET_C_DEFS=-DUSE_HAL_DRIVER -DSTM32F407xx
BOARD_LIBS_PATH=$(STM32_ROOT)/$(TARGET)/build
BOARD_LIBS=$(TARGET)
else ifneq (,$(findstring $(TARGET),minim4stm))
#STM32F415xx
PROCESSOR=STM32F415RG
F_CPU=168000000UL
F_CPU_CYCLES_PER_US=168UL
TARGET_INCLUDE=/home/ramestic/embedded/STM32Cube_FW_F4_V1.24.0/Drivers/CMSIS/Core/Include:/home/ramestic/embedded/STM32Cube_FW_F4_V1.24.0/Drivers/CMSIS/DSP/Include
else
$(error unsuported board $(TARGET))
endif

$(info building for TARGET/PROCESSOR=$(TARGET)/$(PROCESSOR))

#
# Include makefile infrastructure always from the same place. Useful to let Kit
# actually bootstrap when no makefiles are still present at root.
# 
MAKEFILEDIR:=$(dir $(lastword $(MAKEFILE_LIST)))

#
# Debug flags are target independent, so far. Note that optimization is
# automatically force to 'size' if no debugging is required.
#
ifndef DEBUG
DEBUG=s
DEBUG=g
DEBUG_STR=forcing
else
DEBUG_STR=using
endif
ifeq ($(DEBUG),g)
DEBUG_FLAGS = -Og -ggdb3
else
DEBUG_FLAGS = -O$(DEBUG) -g
endif

$(info $(DEBUG_STR) DEBUG_FLAGS=$(DEBUG_FLAGS))

#
# different targets implies different settings
#
ifneq (,$(findstring $(TARGET),tivac-minim4-discovery))

#
# if not assumed then assume RAM located executable
#
ifndef TARGET_MEMORY
TARGET_MEMORY=RAM
endif

$(info building binary for $(TARGET_MEMORY))

CC        = arm-none-eabi-gcc
CXX       = arm-none-eabi-g++
OBJDUMP   = arm-none-eabi-objdump
LD        = arm-none-eabi-gcc
AR        = arm-none-eabi-ar
DEMANGLER = arm-none-eabi-c++filt
SIZE      = arm-none-eabi-size
READELF   = arm-none-eabi-readelf
LOADER    = openocd -s $(call path_macro,$(INTROOT),host,config)

#
# FIXME: does single-precision-constantgcc really belong here?
# See link below for some hints on parameters used here.
#
# http://www.embedded.com/design/mcus-processors-and-socs/4007134/Building-Bare-Metal-ARM-Systems-with-GNU-Part-4
#
# See in this link to a suggestion of why __FPU_PRESENT=1 and
# __VFP_FP__ need to be explicitly set, even after setting gcc fpu and
# float-abi options:
#
# https://community.arm.com/processors/b/blog/posts/10-useful-tips-to-using-the-floating-point-unit-on-the-arm-cortex--m4-processor
PROCESSOR-FLAGS=                                \
    -mcpu=cortex-m4                             \
    -mthumb                                     \
    -mfpu=fpv4-sp-d16                           \
    -mfloat-abi=hard                            \
    -fsingle-precision-constant

# FIXME: is -w doing anything?
COMPILATION-FLAGS=					\
    $(PROCESSOR-FLAGS)					\
    $(DEBUG_FLAGS)					\
    -ffunction-sections					\
    -fdata-sections					\
    -MMD						\
    -Wall						\
    -DARM_MATH_CM4					\
    -DTARGET_$(TARGET)					\
    -DF_CPU=$(F_CPU)					\
    -DF_CPU_CYCLES_PER_US=$(F_CPU_CYCLES_PER_US)

CFLAGS:=                                        \
    $(COMPILATION-FLAGS)                        \
    -std=gnu99

# FIXME: do I need gnu11++?
# FIXME: my singletons are not thread safe, that's why no-threadsafe-statics.
# https://rowley.zendesk.com/entries/46190--Undefined-reference-to-cxa-guard-acquire-error-message
CXXFLAGS=                                       \
    $(COMPILATION-FLAGS)                        \
    -fno-exceptions                             \
    -fno-rtti                                   \
    -fno-threadsafe-statics                     \
    -std=gnu++11

LDFLAGS=                                        \
    -Os                                         \
    -nostartfiles                               \
    -nostdlib                                   \
    -specs=nano.specs                           \
    -Wl,--undefined=ResetISR                    \
    -Wl,--gc-sections                           \
    -Wl,--fatal-warnings                        \
    -Wl,--no-undefined                          \
    $(PROCESSOR-FLAGS)

OBJDUMPFLAGS=-Cdl --insn-width=16

ifeq ($(TARGET_MEMORY),RAM)
LDSCRIPT=$(INTROOT)/config/$(PROCESSOR).ram.ld
else
LDSCRIPT=$(INTROOT)/config/$(PROCESSOR).rom.ld
endif

#icdi
LOADER_CONFIG_FILE = board/ek-$(PROCESSOR).cfg
#jlink
LOADER_CONFIG_FILE = $(PROCESSOR)-jlink.cfg

# c and cpp include files all together
IPATH:=						\
    $(TARGET_INCLUDE)				\
    $(MODROOT)/include				\
    $(INTROOT)/include

STANDARD_LIBS=nosys gcc c m stdc++

else ifeq ($(TARGET),nano328p)

CC      = avr-gcc
CXX     = avr-g++
LD      = avr-gcc
AR      = avr-ar
OBJCOPY = avr-objcopy
SIZE    = avr-size
LOADER  = avrdude

PROCESSOR-FLAGS=                                \
    -mmcu=atmega328p

# FIXME: is -w doing anything?
COMPILATION-FLAGS=                              \
    $(PROCESSOR-FLAGS)                          \
    $(DEBUG_FLAGS)                              \
    -ffunction-sections                         \
    -fdata-sections                             \
    -MMD                                        \
    -Wall                                       \
    -DARDUINO=105                               \
    -DBOARD_TAG=nano328                         \
    -DTARGET_$(TARGET)				\
    -DF_CPU=16000000UL                          \
    -DF_CPU_CYCLES_PER_US=16UL

CFLAGS:=                                        \
    $(COMPILATION-FLAGS)                        \
    -std=gnu99

# FIXME: do I need gnu11++?
CXXFLAGS=                                       \
    $(COMPILATION-FLAGS)                        \
    -fno-exceptions                             \
    -fno-rtti                                   \
    -std=gnu++11

LDFLAGS=                                        \
    $(DEBUG_FLAGS)                              \
    -Wl,--gc-sections                           \
    -Wl,--no-undefined                          \
    $(PROCESSOR-FLAGS)

LDSCRIPT=

LOADER_CONFIG_FILE = /usr/share/arduino/hardware/tools/avrdude.conf

IPATH:=									\
    /usr/share/arduino/hardware/arduino/cores/arduino			\
    /usr/share/arduino/hardware/arduino/variants/eightanaloginputs	\
    $(MODROOT)/include							\
    $(INTROOT)/include

#
# give a chance to do not link against any board library by defining the
# variable as an empty string.
#
BOARD_LIBS ?= /home/ramestic/downloads/arduino/lib/$(TARGET)/libarduino.a

#
# FIXME: note that m and c ordering is swapped compared to tivac. Why is
# that? The nano328p order copied from Arduino-Makefile.
#
STANDARD_LIBS = m c

endif

#
# Include macros that implement actual targets
#
include $(MAKEFILEDIR)/Macros.mk

#
# whether to printout or not the commands doing the actual work
#
ifdef MAKE_VERBOSE
LOG=
else
LOG=@
endif

# Automatically generate makefile dependencies
# http://www.microhowto.info/howto/automatically_generate_makefile_dependencies.html
# http://scottmcpeak.com/autodepend/autodepend.html

#
# default make target is 'all'
#
.DEFAULT_GOAL := all

#
# Delete targets that failed to build midway
#
# http://www.gnu.org/software/make/manual/html_node/Errors.html
#
.DELETE_ON_ERROR:

#
# ???
#
test:
	if [ $$(pgrep -fc "^SCREEN.*") = 1 ]; then  \
       echo one;\
    else\
       echo $$(pgrep -fa SCREEN);\
       echo not one;\
    fi

.PHONY: do_all
# no-op to avoid nothing-to-do warnings
# Note that scripts names under bin directory have no extesion.
do_all:														\
    $(foreach lib,$(LIBRARIES) $(LIBRARIES_L),$(call path_macro,$(MODROOT),target,lib)/lib$(lib).a)		\
    $(foreach exec,$(EXECUTABLES) $(EXECUTABLES_L),$(call path_macro,$(MODROOT),target,bin)/$(exec).elf)	\
    $(foreach script,$(SCRIPTS) $(SCRIPTS_L),$(call path_macro,$(MODROOT),host,bin)/$(basename $(script)))	\
    $(foreach package,$(PYTHON_PACKAGES),$(call path_python_package_macro,$(MODROOT))/$(package))
	@:

# FIXME: what's the reason for INCLUDES_TARGET?
.PHONY: do_install
$(eval $(call do_install_macro,host,bin,$(foreach script,$(SCRIPTS),$(basename $(script)))))
$(eval $(call do_install_macro,host,include,$(foreach i,$(INCLUDES),$(i))))
$(eval $(call do_install_macro,host,config,$(foreach i,$(CONFIGS),$(i))))
$(eval $(call do_install_macro,target,bin,$(foreach exec,$(EXECUTABLES),$(exec).elf)))
$(foreach lib,$(LIBRARIES),$(eval $(call do_install_macro_lib,$(lib),$(addsuffix .o,$(basename $($(lib)_SOURCES))),$($(lib)_LIBNAME))))
$(eval $(call do_install_macro,target,include,$(foreach i,$(INCLUDES_TARGET),$(i))))
$(foreach pyp,$(strip $(PYTHON_PACKAGES)),$(eval $(call do_install_python_package_macro,$(pyp),$($(pyp)_MODULES))))
do_install: $(if $(SCRIPTS),do_install_host_bin) $(if $(EXECUTABLES),do_install_target_bin) $(foreach lib,$(LIBRARIES),do_install_lib_$(lib)) $(foreach pyp,$(PYTHON_PACKAGES),do_install_python_package_$(pyp)) $(if $(INCLUDES),do_install_host_include) $(if $(INCLUDES_TARGET),do_install_target_include) $(if $(CONFIGS),do_install_host_config)

.PHONY: do_upload
# load first executable. want to load a different one? alter their order.
do_upload: $(call path_macro,$(MODROOT),target,bin)/$(word 1, $(EXECUTABLES_L)).elf
	$(upload_receipe)

.PHONY: do_restart
# like pressing reset button on target board
do_reset:
	$(reset_receipe)

.PHONY: do_clean
do_clean:
	$(LOG)rm -f $(call path_macro,$(MODROOT),target,object)/*.o $(call path_macro,$(MODROOT),target,object)/*.d $(call path_macro,$(MODROOT),target,lib)/*.a $(foreach exec,$(EXECUTABLES) $(EXECUTABLES_L),$(call path_macro,$(MODROOT),target,bin)/$(exec).elf) $(foreach script,$(SCRIPTS) $(SCRIPTS_L),$(call path_macro,$(MODROOT),host,bin)/$(basename $(script)))

#
# for each library produce a rule to archive object files into an archive and
# also archives that the library depends on; all source file extensions are
# replace by 'o'. It means that eachi library has associated an archive that
# would contain, apart from plain object files, other archives (libraries),
# therefore, there will be quite a duplication but the benefit is that
# specifying on which library or libraries another one depends on is simplified
# by simply referring to the top level one of them. Example: if B depends on A
# and C depends on B, then to build C is enough to write C_LIBS=B.
#
$(foreach lib,$(LIBRARIES) $(LIBRARIES_L),              \
$(eval                                                  \
    $(call archive_rule,                                \
        $(lib),                                         \
        $(addsuffix .o,$(basename $($(lib)_SOURCES))),  \
        $($(lib)_LIBS))))

#
# produce a compilation rule for each source file in each library
#
$(foreach lib,$(LIBRARIES) $(LIBRARIES_L),                          \
$(eval                                                              \
    $(call compile_rules,                                           \
        $($(lib)_SOURCES),                                          \
        $(filter $(LIBRARIES) $(LIBRARIES_L),$($(lib)_LIBS)),       \
        $(filter-out $(LIBRARIES) $(LIBRARIES_L),$($(lib)_LIBS)))))

#
# for each executable produce a rule to link object files into an elf; all
# source file extensions are replace by 'o'.
#
$(foreach exec, $(EXECUTABLES) $(EXECUTABLES_L),                               \
$(eval                                                                         \
    $(call link_rule,                                                          \
        $(exec),                                                               \
        $(addsuffix .o,$(basename $($(exec)_SOURCES))),                        \
        all $(if $(USE_SYSTEM_VIEW),SystemView,SystemViewOff) $($(exec)_LIBS), \
        $(BOARD_LIBS),                                                         \
        $(STANDARD_LIBS),                                                      \
        $(LDSCRIPT))))

#
# produce a compilation rule for each source file in each executable
#
$(foreach exec,$(EXECUTABLES) $(EXECUTABLES_L),                     \
$(eval                                                              \
    $(call compile_rules,                                           \
        $($(exec)_SOURCES),                                         \
        $(filter $(LIBRARIES) $(LIBRARIES_L),$($(lib)_LIBS)),       \
        $(filter-out $(LIBRARIES) $(LIBRARIES_L),$($(lib)_LIBS)))))

#
# produce python packaging rule for each package
#
$(foreach package,$(strip $(PYTHON_PACKAGES) $(PYTHON_PACKAGES_L)), \
$(eval                                          \
    $(call python_package_compile_rule,$(package),$(addsuffix .py,$($(package)_MODULES)))))

#
# for each shell script produce a rule to build a shell script, that
# is, to add a corresponding banshee and create an executable script
# file without extension in modules bin directory.
#
$(foreach script,$(SCRIPTS) $(SCRIPTS_L),       \
$(eval                                          \
    $(call compile_rules,$(script),)))

#
# force remake when Makefile has changed, avoid cleaning twice by defining the
# rule only if the goal is not clean itself.
#
ifneq ($(MAKECMDGOALS),clean)
-include $(call path_macro,$(MODROOT),target,object)/Makefile.remake
$(call path_macro,$(MODROOT),target,object)/Makefile.remake: Makefile
	@mkdir -p $(call path_macro,$(MODROOT),target,object)
	@touch $@
	@make -s clean
-include $(call path_macro,$(MODROOT),target,object)/Makefile.mk.remake
$(call path_macro,$(MODROOT),target,object)/Makefile.mk.remake: Makefile.mk
	@mkdir -p $(call path_macro,$(MODROOT),target,object)
	@touch $@
	@make -s clean
endif

#___oOo___
