ARDUINO_PATH    =  /usr/share/arduino
ENERGIA_PATH    =  $(EMBEDDED_PATH)/energia-0101E0012
ENERGIA_PATH    =  $(EMBEDDED_PATH)/Energia
USER_LIB_PATH   =  $(EMBEDDED_APPLICATION_PATH)
CURRENT_DIR     := $(shell pwd)
CURRENT_DIR     := $(shell echo '$(CURRENT_DIR)' | sed 's/ /\\\ /g')
APP_LIBS_LIST   =  0
AVRDUDE_PATH    =  /usr
SERIAL_BAUDRATE =  115200
MAKEFILE_PATH   =  $(USER_LIB_PATH)/Makefiles
UTILITIES_PATH  =  $(EMBEDDED_PATH)/xcode/Utilities
#USER_LIBS       =  Library/ErrorCodes              \
#                   Library/SerialCommands          \
#                   Library/SampleStatistics        \
#                   Library/Timer                   \
#                   Library/MainLoop
