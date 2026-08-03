USE_SYSTEM_VIEW=on
LIBRARIES = SystemView SystemViewOff

SystemView_SOURCES =				\
    SEGGER_SYSVIEW.c				\
    SEGGER_SYSVIEW_Config_NoOS.c		\
    SEGGER_RTT.c				\
    SEGGER_RTT_ASM_ARMv7M.S			\
    SystemView.cpp
SystemView_LIBNAME = SystemView

SystemViewOff_SOURCES =				\
    SystemViewOff.cpp
SystemViewOff_LIBNAME = SystemViewOff

INCLUDES_TARGET  =				\
    Global.h					\
    SEGGER.h					\
    SEGGER_RTT_Conf.h				\
    SEGGER_RTT.h				\
    SEGGER_SYSVIEW_ConfDefaults.h		\
    SEGGER_SYSVIEW_Conf.h			\
    SEGGER_SYSVIEW.h				\
    SEGGER_SYSVIEW_Int.h			\
    SystemView.h    

CONFIGS =					\
    SystemViewNoOS.txt

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
