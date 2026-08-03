#
# bootstrap by manually copying makefile infrastructure to root
#

INCLUDES = Makefile.mk Macros.mk etl_profile.h

SCRIPTS =					\
    getTemplateDirectory.sh			\
    getTemplateSourceCode.sh			\
    getTemplateMakefile.sh			\
    ramArchiveUpdate.sh				\
    ramUnpackArchive.py				\
    ramRelocateArchive.py			\
    getEntryAddress.py

CONFIGS =					\
    template.sh					\
    templateCSrc.c				\
    templateCppSrc.cpp				\
    templateCHeader.h				\
    templateCppHeader.h				\
    templateCppTest.cpp				\
    templateMakefile				\
    templatePyScript.py

#
# Kit must bootstrap reading its own makefiles, not from root.
#
include ../include/Makefile.mk

all: do_all
	@echo " . . . 'all' done" 

install: do_install
	@echo " . . . 'install' done" 

clean: do_clean
	@echo " . . . 'clean' done" 

clean_liball: do_clean
	$(LOG)rm -f $(call path_macro,$(INTROOT),target,lib)/liball.a
	@echo " . . . 'clean_build' done" 

#___oOo___
