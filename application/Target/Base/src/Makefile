LIBRARIES       = Base
INCLUDES_TARGET = Target.h

Base_SOURCES = startup_$(PROCESSOR).c system_$(PROCESSOR).c globals.cpp
Base_LIBS    =

INCLUDES    =					\
    $(PROCESSOR).h				\
    system_$(PROCESSOR).h    

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
