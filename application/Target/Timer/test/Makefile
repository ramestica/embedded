TARGET_MEMORY=ROM
#USE_SYSTEM_VIEW=on

EXECUTABLES_L = systick timer 
systick_SOURCES = systick.cpp
#systick_LIBS = driver
timer_SOURCES = timer_$(PROCESSOR).cpp

include $(EMBEDDED_ROOT)/include/Makefile.mk

all: do_all
	@echo " . . . 'all' done" 

install: do_install
	@echo " . . . 'install' done" 

upload: do_upload
	@echo " . . . 'upload' done" 

clean: do_clean
	@echo " . . . 'clean' done" 

#___oOo___
