LIBRARIES = Main
INCLUDES  = Main.h
CONFIGS   = tm4c123gh6pm.ld tm4c123gh6pm-jlink.cfg

Main_SOURCES = main.cpp

include $(EMBEDDED_ROOT)/include/Makefile.mk

all: do_all
	@echo " . . . 'all' done" 

install: do_install
#
# arm cortex m4 linker script renamed in place (introot) with ram and
# rom specific versions.
#
ifneq ($(TARGET),nano328p)
	$(LOG)$(CC) -E -P -x c -DLOAD_TO_ROM $(EMBEDDED_ROOT)/config/$(PROCESSOR).ld > $(EMBEDDED_ROOT)/config/$(PROCESSOR).rom.ld
	$(LOG)$(CC) -E -P -x c -DLOAD_TO_RAM $(EMBEDDED_ROOT)/config/$(PROCESSOR).ld > $(EMBEDDED_ROOT)/config/$(PROCESSOR).ram.ld
endif
	@echo " . . . 'install' done" 

upload: do_upload
	@echo " . . . 'upload' done" 

clean: do_clean
	@echo " . . . 'clean' done" 

#___oOo___
