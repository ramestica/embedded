LIBRARIES  = Utils
INCLUDES   = Utils.h

Utils_SOURCES = delayus.cpp malloc.cpp getBitIndex.cpp
Utils_LIBS    =

SCRIPTS = accelerometer3d.py

PYTHON_PACKAGES = utils mathematics serial data

utils_MODULES = formatting time

mathematics_MODULES = adc

serial_MODULES = serial_monitor application_commands

data_MODULES = defs usbtmc telemetry_helper accelerometer_ellipsoid gui

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
