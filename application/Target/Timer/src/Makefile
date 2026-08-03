LIBRARIES       = TargetTimer
INCLUDES_TARGET = Timer.h SystemTick.h

TargetTimer_SOURCES = Timer.cpp Timer_$(PROCESSOR).cpp SystemTick_$(PROCESSOR).cpp
TargetTimer_LIBS    =

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
