MODULES_SUPPORT =				\
    Support/ErrorCodes				\
    Support/Utils

MODULES_TARGET =				\
    Target/SystemView				\
    Target/Base					\
    Target/Interrupts				\
    Target/GPIO					\
    Target/UART					\
    Target/Timer				\
    Target/PWM					\
    Target/I2C					\
    Target/Main

MODULES_LOGIC =					\
    Logic/Queue					\
    Logic/SampleStatistics			\
    Logic/Host					\
    Logic/UART					\
    Logic/I2C					\
    Logic/PulseModulation			\
    Logic/Endianness				\
    Logic/MainLoop

MODULES_SENSORS =				\
    Sensors/MPU6050

MODULES_CONTROL =				\
    Control/Filter				\
    Control/IMU					\
    Control/PID    

MODULES_DRONES =				\
    Drones/TurnigyHAL

MODULES =					\
    Kit						\
    $(MODULES_SUPPORT)				\
    $(MODULES_TARGET)				\
    $(MODULES_LOGIC)				\
    $(MODULES_SENSORS)				\
    $(MODULES_CONTROL)				\
    $(MODULES_DRONES)

blog = build-$(TARGET).log

#
# 1/ modules to make clean all install
#
define canned
@for module in $(1); do \
    echo "############ $${module}" ;\
    if [ ! -d $${module} ]; then \
    echo "### ==> $${modules} MODULE NOT FOUND! FAILED! " | tee -a $(blog);\
    fi;\
    if [ -f $${module}/src/Makefile ]; then \
    make -Bwk -C $${module}/src/ $(if $(clean), clean, clean all install) >> $(blog) 2>& 1 || echo "### ==> FAILED $@ ! " | tee -a $(blog); \
    fi;\
    done
endef

support:
	$(call canned,$(MODULES_SUPPORT))
target:
	$(call canned,$(MODULES_TARGET))
logic:
	$(call canned,$(MODULES_LOGIC))
sensors:
	$(call canned,$(MODULES_SENSORS))
control:
	$(call canned,$(MODULES_CONTROL))
drones:
	$(call canned,$(MODULES_DRONES))

.PHONY: support target logic sensors control build 

build:
	@echo "############ RAM Embeeded Software ($(TARGET)) ############" | tee -a $(blog)
	@make -ks -C Kit/src/ clean_liball all install >> $(blog) 2>& 1
	@make -ks support
	@make -ks target
	@make -ks logic
	@make -ks sensors
	@make -ks control
	@make -ks drones

clean:
	@echo "############ RAM Embeeded Software ($(TARGET)) ############" | tee -a $(blog)
	@make -ks support clean=on
	@make -ks target clean=on
	@make -ks logic clean=on
	@make -ks sensors clean=on
	@make -ks control clean=on
	@make -ks drones clean=on

#
# ___oOo___
